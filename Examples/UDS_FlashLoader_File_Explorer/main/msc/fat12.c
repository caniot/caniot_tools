#ifndef _FAT12_C_
#define _FAT12_C_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "fat12.h"
#include <ctype.h>
#define UPPER_12_BITS(x) (x >> 4)
#define LOWER_12_BITS(x) (x & 0x0FFF)
#define FAT_OFFSET(x) (x + (x >> 1))
#define FAT_ENTRY(x, y) (x[y] + (x[y + 1] * 0x100))
#define EVEN(x) ((x & 1) != 1)

static FILE *device;
bool msc_mounted = false;
static BOOT_SECTOR *read_boot_sect()
{

	int nbytes = 0;

	BOOT_SECTOR *buffer;

	/* check that the device is opened */
	if (device == NULL)
	{
		// fprintf(stderr, "boot_sect_read: Device not opened\n");
		return NULL;
	}

	/* allocate some memory for the boot sector */
	if ((buffer = (BOOT_SECTOR *)malloc(sizeof(BOOT_SECTOR))) == NULL)
	{
		// fprintf(stderr, "boot_sect_read: Could not allocate memory for buffer\n");
		return NULL;
	}

	/* if the reading position is not at the correct position then seek */
	if (ftell(device) != 0x00)
	{
		if (fseek(device, 0x00, SEEK_SET) != 0x00)
		{
			// fprintf(stderr, "boot_sect_read: Could not seek to beginning\n");
			return NULL;
		}
	}

	/* read the boot sector into the buffer */
	if ((nbytes = fread(buffer, 1, sizeof(BOOT_SECTOR), device)) == 0x00)
	{
		// fprintf(stderr, "boot_sect_read: Could not read from device\n");
		return NULL;
	}

	/* return a pointer to the buffer */
	return buffer;
}

static DIRECTORY_ENTRY *read_root_dir(BIOS_PARAMETER_BLOCK *bpb)
{

	int nbytes = 0;

	DIRECTORY_ENTRY *buffer;

	int ROOT_DIR_START = bpb->ReservedSectors * bpb->BytesPerSector + bpb->NumFATS * bpb->SectorsPerFAT * bpb->BytesPerSector;
	int ROOT_DIR_SIZE = sizeof(DIRECTORY_ENTRY) * bpb->RootDirEntries;

	/* check for null pointer */
	if (bpb == NULL)
	{
		return NULL;
	}

	/* check that the device is opened */
	if (device == NULL)
	{
		// fprintf(stderr, "read_root_dir: Device not opened\n");
		return NULL;
	}

	/* allocate some memory for the root directory sector */
	if ((buffer = (DIRECTORY_ENTRY *)malloc(ROOT_DIR_SIZE)) == NULL)
	{
		// fprintf(stderr, "read_root_dir: Could not allocate memory for buffer\n");
		return NULL;
	}

	/* if the reading position is not at the correct position then seek */
	if (ftell(device) != ROOT_DIR_START)
	{
		if (fseek(device, ROOT_DIR_START, SEEK_SET) != 0x00)
		{
			// fprintf(stderr, "read_root_dir: Could not seek to ROOT_DIR_START (0x%04x)\n", ROOT_DIR_START);
			return NULL;
		}
	}

	/* read the boot sector into the buffer */
	if ((nbytes = fread(buffer, 1, ROOT_DIR_SIZE, device)) == 0x00)
	{
		// fprintf(stderr, "read_root_dir: Could not read from device\n");
		return NULL;
	}

	/* return a pointer to the buffer */
	return buffer;
}

static unsigned char *read_fat(BIOS_PARAMETER_BLOCK *bpb)
{

	int nbytes = 0;

	unsigned char *buffer;

	int FAT_START = bpb->ReservedSectors * bpb->BytesPerSector;
	int FAT_SIZE = bpb->SectorsPerFAT * bpb->BytesPerSector;

	/* check for null pointer */
	if (bpb == NULL)
	{
		return NULL;
	}

	/* check that the device is opened */
	if (device == NULL)
	{
		// fprintf(stderr, "read_fat: Device not opened\n");
		return NULL;
	}

	/* allocate some memory for the root directory sector */
	if ((buffer = (unsigned char *)malloc(FAT_SIZE)) == NULL)
	{
		// fprintf(stderr, "read_fat: Could not allocate memory for buffer\n");
		return NULL;
	}

	/* if the reading position is not at the correct position then seek */
	if (ftell(device) != FAT_START)
	{
		if (fseek(device, FAT_START, SEEK_SET) != 0x00)
		{
			// fprintf(stderr, "read_fat: Could not seek to FAT_START (0x%04x)\n", FAT_START);
			return NULL;
		}
	}

	/* read the boot sector into the buffer */
	if ((nbytes = fread(buffer, 1, FAT_SIZE, device)) == 0x00)
	{
		// fprintf(stderr, "read_fat: Could not read from device\n");
		return NULL;
	}

	/* return a pointer to the buffer */
	return buffer;
}

static int next_cluster(unsigned char *fat, int cluster)
{

	int fat_offset;
	int fat_entry;

	/* check for null pointer */
	if (fat == NULL)
	{
		return 0x00;
	}

	/* multiply the cluster by 1.5 to get the fat position */
	fat_offset = FAT_OFFSET(cluster);

	/* read 16 bits from the fat at fat_offset */
	fat_entry = FAT_ENTRY(fat, fat_offset);

	/* if the cluster is even then take the upper 12 bits */
	if (EVEN(cluster))
	{
		return LOWER_12_BITS(fat_entry);

		/* otherwise take the lower 12 bits */
	}
	else
	{
		return UPPER_12_BITS(fat_entry);
	}
}

static uint8_t *load_file(BIOS_PARAMETER_BLOCK *bpb, unsigned char *fat, DIRECTORY_ENTRY *entry, uint32_t *FileSize)
{

	int nbytes = 0;
	int position = 0;
	int open_space = 0;
	int cluster = 0;
	int bytes_read = 0;

	uint8_t *buffer = NULL;

	/* check for null pointers */
	if (bpb == NULL || fat == NULL || entry == NULL || FileSize == NULL)
	{
		return NULL;
	}

	/* check for a 0 byte filee */
	if (entry->FileSize == 0)
	{
		return NULL;
	}

	/* check that the device is opened */
	if (device == NULL)
	{
		// fprintf(stderr, "load_file: Device not opened\n");
		return NULL;
	}

	/* allocate some memory for the file */
	if ((buffer = (uint8_t *)malloc(entry->FileSize + 1)) == NULL)
	{
		// fprintf(stderr, "load_file: Could not allocate memory for buffer\n");
		return NULL;
	}

	/* calculate the position of the open space */
	open_space = bpb->ReservedSectors * bpb->BytesPerSector + bpb->NumFATS * bpb->SectorsPerFAT * bpb->BytesPerSector + bpb->RootDirEntries * sizeof(DIRECTORY_ENTRY);

	cluster = entry->Cluster;
	*FileSize = 0;
	while (cluster < FAT12_END)
	{

		position = open_space + (cluster - 2) * bpb->SectorsPerCluster * bpb->BytesPerSector;

		/* if the reading position is not at the correct position then seek */
		if (ftell(device) != position)
		{
			if (fseek(device, position, SEEK_SET) != 0x00)
			{
				// fprintf(stderr, "load_file: Could not seek to position (0x%04x)\n", position);
				return NULL;
			}
		}

		/* read the sector into the buffer */
		if (entry->FileSize - bytes_read < bpb->BytesPerSector)
		{
			if ((nbytes = fread(&buffer[bytes_read], 1, entry->FileSize - bytes_read, device)) == 0x00)
			{
				// fprintf(stderr, "load_file: Could not read from device\n");
				return NULL;
			}
		}
		else
		{
			if ((nbytes = fread(&buffer[bytes_read], 1, bpb->BytesPerSector, device)) == 0x00)
			{
				// fprintf(stderr, "load_file: Could not read from device\n");
				return NULL;
			}

			bytes_read += bpb->BytesPerSector;
		}

		*FileSize += (uint32_t)nbytes;

		/* calculate where the next cluster is */
		cluster = next_cluster(fat, cluster);
	}

	/* return a pointer to the buffer */
	// buffer[entry->FileSize] = '\0';
	return buffer;
}

static DIRECTORY_ENTRY *load_directory(BIOS_PARAMETER_BLOCK *bpb, unsigned char *fat, DIRECTORY_ENTRY *entry)
{

	int nbytes = 0;
	int position = 0;
	int open_space = 0;
	int cluster = 0;
	int bytes_read = 0;
	int size = 0;

	unsigned char *buffer;

	/* check for null pointers */
	if (bpb == NULL || fat == NULL || entry == NULL)
	{
		return NULL;
	}

	/* check that the device is opened */
	if (device == NULL)
	{
		// fprintf(stderr, "load_file: Device not opened\n");
		return NULL;
	}

	/* calculate the position of the open space */
	open_space = bpb->ReservedSectors * bpb->BytesPerSector + bpb->NumFATS * bpb->SectorsPerFAT * bpb->BytesPerSector + bpb->RootDirEntries * sizeof(DIRECTORY_ENTRY);

	/* go through the cluster chain so we know how much space to allocate */
	cluster = entry->Cluster;

	while (cluster < FAT12_END)
	{
		size += bpb->SectorsPerCluster * bpb->BytesPerSector;
		cluster = next_cluster(fat, cluster);
	}

	/* allocate some memory for the directory */
	if ((buffer = (unsigned char *)malloc(size)) == NULL)
	{
		// fprintf(stderr, "load_directory: Could not allocate memory for buffer\n");
		return NULL;
	}

	/* now load the directory */
	cluster = entry->Cluster;

	while (cluster < FAT12_END)
	{

		position = open_space + (cluster - 2) * bpb->SectorsPerCluster * bpb->BytesPerSector;

		/* if the reading position is not at the correct position then seek */
		if (ftell(device) != position)
		{
			if (fseek(device, position, SEEK_SET) != 0x00)
			{
				// fprintf(stderr, "load_directory: Could not seek to position (0x%04x)\n", position);
				return NULL;
			}
		}

		/* read the sector into the buffer */
		if ((nbytes = fread(&buffer[bytes_read], 1, bpb->BytesPerSector, device)) == 0x00)
		{
			// fprintf(stderr, "load_directory: Could not read from device\n");
			return NULL;
		}

		bytes_read += bpb->BytesPerSector;

		/* calculate where the next cluster is */
		cluster = next_cluster(fat, cluster);
	}

	/* return a pointer to the buffer */
	return (DIRECTORY_ENTRY *)buffer;
}

static void print_bpb(BIOS_PARAMETER_BLOCK *bpb)
{

	/* check for null pointer */
	if (bpb == NULL)
	{
		return;
	}

	// printf("\nBIOS PARAMETER BLOCK\n");
	// printf("  BytesPerSector    = %#06x\n", bpb->BytesPerSector);
	// printf("  SectorsPerCluster = %#04x\n", bpb->SectorsPerCluster);
	// printf("  ReservedSectors   = %#06x\n", bpb->ReservedSectors);
	// printf("  NumFATS           = %#04x\n", bpb->NumFATS);
	// printf("  RootDirEntries    = %#06x\n", bpb->RootDirEntries);
	// printf("  SmallSectors      = %#06x\n", bpb->SmallSectors);
	// printf("  MediaDescriptor   = %#04x\n", bpb->MediaDescriptor);
	// printf("  SectorsPerFAT     = %#06x\n", bpb->SectorsPerFAT);
	// printf("  SectorsPerTrack   = %#06x\n", bpb->SectorsPerTrack);
	// printf("  Heads             = %#06x\n", bpb->Heads);
	// printf("  HiddenSectors     = 0x%08x\n", bpb->HiddenSectors);
	// printf("  LargeSectors      = 0x%08x\n", bpb->LargeSectors);
}

// static void print_dir_entry(DIRECTORY_ENTRY *entry)
// {

// 	// int i;

// 	// /* check for null pointer */
// 	// if (entry == NULL)
// 	// {
// 	// 	return;
// 	// }
// 	// // printf("\n");
// 	// /* check if this is the volume label */
// 	// if ((entry->Attributes & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)
// 	// {
// 	// 	// printf("VOLUME LABEL: ");
// 	// }

// 	// /* write out the file name */
// 	// for (i = 0; i < sizeof(entry->FileName) - 3; i++)
// 	// {
// 	// 	if (entry->FileName[i] != ' ')
// 	// 	{
// 	// 		// printf("%c", entry->FileName[i]);
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		break;
// 	// 	}
// 	// }

// 	// /* write out the extension if it's not a directory or volume label*/
// 	// if ((((entry->Attributes) & ATTR_DIRECTORY) == 0x00) && ((entry->Attributes & ATTR_VOLUME_ID) == 0x00))
// 	// {
// 	// 	// printf(".");
// 	// 	// printf("%c", entry->FileName[sizeof(entry->FileName) - 3]);
// 	// 	// printf("%c", entry->FileName[sizeof(entry->FileName) - 2]);
// 	// 	// printf("%c", entry->FileName[sizeof(entry->FileName) - 1]);
// 	// }
// 	// else
// 	// {
// 	// 	// printf("    ");
// 	// }

// 	// // /* pad out the filename to 8 bytes*/
// 	// // while (i < 8)
// 	// // {
// 	// // 	printf(" ");
// 	// // 	i++;
// 	// // }

// 	// /* if this was a volume label then we need no more information */
// 	// if ((entry->Attributes & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)
// 	// {
// 	// 	return;
// 	// }

// 	// // printf("   ");

// 	// /* write out the attributes */
// 	// if (((entry->Attributes) & ATTR_DIRECTORY) == ATTR_DIRECTORY)
// 	// {
// 	// 	// printf("<DIR>");
// 	// }
// 	// else
// 	// {
// 	// 	if (((entry->Attributes) & ATTR_ARCHIVE) == ATTR_ARCHIVE)
// 	// 		// printf("a");
// 	// 		else
// 	// 			// printf("-");
// 	// 			if (((entry->Attributes) & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)
// 	// 			// printf("v");
// 	// 			else
// 	// 			// printf("-");
// 	// 			if (((entry->Attributes) & ATTR_SYSTEM) == ATTR_SYSTEM)
// 	// 			// printf("s");
// 	// 			else
// 	// 			// printf("-");
// 	// 			if (((entry->Attributes) & ATTR_HIDDEN) == ATTR_HIDDEN)
// 	// 			// printf("h");
// 	// 			else
// 	// 			// printf("-");
// 	// 			if (((entry->Attributes) & ATTR_READ_ONLY) == ATTR_READ_ONLY)
// 	// 			// printf("r");
// 	// 			else
// 	// 	// printf("-");
// 	// }

// 	// // printf("   ");

// 	// /* write out the file size if it's not a directory */
// 	// if (((entry->Attributes) & ATTR_DIRECTORY) == 0x00)
// 	// {
// 	// 	// printf("%-7i bytes (%7.2f KB)", entry->FileSize, entry->FileSize / 1024.0);
// 	// }

// 	// // printf("\n");
// }
static int lookForFiles(BIOS_PARAMETER_BLOCK *bpb, unsigned char *fat, DIRECTORY_ENTRY *directory, int indent, char (*extensions)[4], int extentionsLength, char (*filenames)[11], int maxFile)
{
	int fileFound = 0;
	int i;

	DIRECTORY_ENTRY *dir = directory;
	DIRECTORY_ENTRY *sub_dir = NULL;

	for (int i = 0; i < extentionsLength; i++)
	{
		if (strlen(extensions[i]) < 3)
		{
			return 0;
		}
	}

	/* check for null pointers */
	if (bpb == NULL || fat == NULL || directory == NULL || extensions == NULL || filenames == NULL)
	{
		return fileFound;
	}

	while (dir->FileName[0] != DIRENTRY_END)
	{

		/* if this is not a deleted directory or the beginning */
		if ((dir->FileName[0] != DIRENTRY_DELETED) && (dir->FileName[0] != '.'))
		{

			// print_dir_entry(dir);

			/* if the entry in the directory is a subdirectory then load the sub directory */
			if (((dir->Attributes) & ATTR_DIRECTORY) == ATTR_DIRECTORY)
			{
				sub_dir = load_directory(bpb, fat, dir);

				if (sub_dir != NULL)
				{
					/* and write out this directory */
					lookForFiles(bpb, fat, sub_dir, indent + 4, extensions, extentionsLength, filenames + fileFound, maxFile - fileFound);

					/* free the memory for the sub directory now */
					free(sub_dir);
				}
			}
			else if ((((dir->Attributes) & ATTR_DIRECTORY) == 0x00) && ((dir->Attributes & ATTR_VOLUME_ID) == 0x00))
			{
				int size = sizeof(dir->FileName);
				for (size_t j = 0; j < extentionsLength; j++)
				{
					if (tolower(extensions[j][0]) == tolower(dir->FileName[size - 3]) &&
						tolower(extensions[j][1]) == tolower(dir->FileName[size - 2]) &&
						tolower(extensions[j][2]) == tolower(dir->FileName[size - 1]))
					{
						/* write out the file name */
						for (i = 0; i < size - 3; i++)
						{

							if (dir->FileName[i] == ' ')
							{
								break;
							}
							else
							{
								filenames[fileFound][i] = (char)tolower(dir->FileName[i]);
							}
						}
						if (fileFound < maxFile)
							fileFound++;
					}
				}
			}
		}

		/* move on to the next entry */
		dir = (DIRECTORY_ENTRY *)((int)dir + sizeof(DIRECTORY_ENTRY));
	}
	return fileFound;
}
static const char configName[] = "config";
static char *load_config_file(BIOS_PARAMETER_BLOCK *bpb, unsigned char *fat, DIRECTORY_ENTRY *directory, int indent, uint32_t *messagefile_size)
{
	DIRECTORY_ENTRY *dir = directory;
	DIRECTORY_ENTRY *sub_dir = NULL;
	char *messagefile_ptr = NULL;
	/* check for null pointers */
	if (bpb == NULL || fat == NULL || directory == NULL)
	{
		return NULL;
	}

	while (dir->FileName[0] != DIRENTRY_END)
	{

		/* if this is not a deleted directory or the beginning */
		if ((dir->FileName[0] != DIRENTRY_DELETED) && (dir->FileName[0] != '.'))
		{

			// print_dir_entry(dir);

			/* if the entry in the directory is a subdirectory then load the sub directory */
			if (((dir->Attributes) & ATTR_DIRECTORY) == ATTR_DIRECTORY)
			{
				sub_dir = load_directory(bpb, fat, dir);

				if (sub_dir != NULL)
				{
					/* and write out this directory */
					messagefile_ptr = load_config_file(bpb, fat, sub_dir, indent + 4, messagefile_size);

					/* free the memory for the sub directory now */
					free(sub_dir);
				}
			}
			else if ((((dir->Attributes) & ATTR_DIRECTORY) == 0x00) && ((dir->Attributes & ATTR_VOLUME_ID) == 0x00))
			{
				int size = sizeof(dir->FileName);
				/*look only for cfg file*/
				if (('c' == tolower(dir->FileName[size - 3]) &&
					 'f' == tolower(dir->FileName[size - 2]) &&
					 'g' == tolower(dir->FileName[size - 1])))
				{
					bool configFileExist = true;
					/* write out the file name */
					for (int i = 0; i < size - 3 && i < strlen(configName); i++)
					{

						if (dir->FileName[i] == ' ')
						{
							break;
						}
						else
						{
							if ((char)tolower(dir->FileName[i]) != configName[i])
							{
								configFileExist = false;
								break;
							}
						}
					}
					if (configFileExist)
					{
						messagefile_ptr = (char *)load_file(bpb, fat, dir, messagefile_size);
						if (messagefile_ptr != NULL)
						{
							break; /*only one file*/
						}
					}
				}
			}
		}

		/* move on to the next entry */
		dir = (DIRECTORY_ENTRY *)((int)dir + sizeof(DIRECTORY_ENTRY));
	}
	return messagefile_ptr;
}

static char *print_directory(const char *FileNameToBeUse, BIOS_PARAMETER_BLOCK *bpb, unsigned char *fat, DIRECTORY_ENTRY *directory, int indent, uint32_t *messagefile_size)
{

	DIRECTORY_ENTRY *dir = directory;
	DIRECTORY_ENTRY *sub_dir = NULL;
	char *messagefile_ptr = NULL;
	/* check for null pointers */
	if (bpb == NULL || fat == NULL || directory == NULL)
	{
		return NULL;
	}

	while (dir->FileName[0] != DIRENTRY_END)
	{

		/* if this is not a deleted directory or the beginning */
		if ((dir->FileName[0] != DIRENTRY_DELETED) && (dir->FileName[0] != '.'))
		{

			// print_dir_entry(dir);

			/* if the entry in the directory is a subdirectory then load the sub directory */
			if (((dir->Attributes) & ATTR_DIRECTORY) == ATTR_DIRECTORY)
			{
				sub_dir = load_directory(bpb, fat, dir);

				if (sub_dir != NULL)
				{
					/* and write out this directory */
					messagefile_ptr = print_directory(FileNameToBeUse, bpb, fat, sub_dir, indent + 4, messagefile_size);

					/* free the memory for the sub directory now */
					free(sub_dir);
				}
			}
			else if ((((dir->Attributes) & ATTR_DIRECTORY) == 0x00) && ((dir->Attributes & ATTR_VOLUME_ID) == 0x00))
			{
				int size = sizeof(dir->FileName);
				/*look only for hex or bn file*/
				if (('h' == tolower(dir->FileName[size - 3]) &&
					 'e' == tolower(dir->FileName[size - 2]) &&
					 'x' == tolower(dir->FileName[size - 1])) ||
					('b' == tolower(dir->FileName[size - 3]) &&
					 'i' == tolower(dir->FileName[size - 2]) &&
					 'n' == tolower(dir->FileName[size - 1])))
				{

					bool configFileExist = true;

					if (FileNameToBeUse != NULL)
					{
						/* write out the file name */
						for (int i = 0; i < size - 3 && i < strlen(FileNameToBeUse); i++)
						{

							if (dir->FileName[i] == ' ')
							{
								break;
							}
							else
							{
								if ((char)tolower(dir->FileName[i]) != (char)tolower(FileNameToBeUse[i]))
								{
									configFileExist = false;
									break;
								}
							}
						}
					}

					if (configFileExist)
					{
						messagefile_ptr = (char *)load_file(bpb, fat, dir, messagefile_size);
						if (messagefile_ptr != NULL)
						{
							break; /*only one file*/
						}
					}
				}
			}
		}

		/* move on to the next entry */
		dir = (DIRECTORY_ENTRY *)((int)dir + sizeof(DIRECTORY_ENTRY));
	}
	return messagefile_ptr;
}

void fat12_findFiles(int *fileFound, char (*extensions)[4], char (*buffer)[11])
{

	BOOT_SECTOR *boot_sect = NULL;
	unsigned char *fat = NULL;
	DIRECTORY_ENTRY *root_dir = NULL;
	unsigned char *f = NULL;

	/* check for null pointers */
	if (fileFound == NULL || extensions == NULL || buffer == NULL)
	{
		return;
	}
	/* check that the device is mounted */
	if (false == msc_mounted)
	{
		return;
	}

	/* open the file for reading in binary mode */
	device = fmemopen(appl_msc_disk, APPL_DISK_BLOCK_NUM * APPL_DISK_BLOCK_SIZE, "r");

	if (device == NULL)
	{
		// printf("&\n");
		// fprintf(stderr, "Could not open device %s\n", filename);
		goto cleanup;
	}

	/* read the boot sector */
	if ((boot_sect = read_boot_sect()) == NULL)
	{
		goto cleanup;
	}

	/* read the fat */
	if ((fat = read_fat(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the bios parameter block */
	print_bpb(&boot_sect->BPB);

	/* read the root directory */
	if ((root_dir = read_root_dir(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the root directory */
	// printf("\n\n\n");
	// print_directory(&boot_sect->BPB, fat, root_dir, 0);
	*fileFound = lookForFiles(&boot_sect->BPB, fat, root_dir, 0, extensions, 2, buffer, 10);
	if (*fileFound > 0)
	{
		for (size_t i = 0; i < *fileFound; i++)
		{
			printf("FieName%d: %.*s\n", i, 8, buffer[i]);
		}
	}

cleanup:

	/* free the used memory */
	if (f != NULL)
	{
		free(f);
	}

	if (root_dir != NULL)
	{
		free(root_dir);
	}

	if (fat != NULL)
	{
		free(fat);
	}

	if (boot_sect != NULL)
	{
		free(boot_sect);
	}

	/* close the file */
	if (device != NULL)
	{
		fclose(device);
	}
}

char *fat12_ReadFile(const char *FileNameToBeUse, uint32_t *messagefile_size)
{
	char *messagefile_ptr = NULL;
	BOOT_SECTOR *boot_sect = NULL;
	unsigned char *fat = NULL;
	DIRECTORY_ENTRY *root_dir = NULL;
	unsigned char *f = NULL;

	/* check for null pointers */
	if (messagefile_size == NULL)
	{
		return NULL;
	}
	/* check that the device is mounted */
	if (false == msc_mounted)
	{
		*messagefile_size = 0;
		return NULL;
	}
	/* open the file for reading in binary mode */
	device = fmemopen(appl_msc_disk, APPL_DISK_BLOCK_NUM * APPL_DISK_BLOCK_SIZE, "r");

	if (device == NULL)
	{
		// printf("&\n");
		// fprintf(stderr, "Could not open device %s\n", filename);
		goto cleanup;
	}

	/* read the boot sector */
	if ((boot_sect = read_boot_sect()) == NULL)
	{
		goto cleanup;
	}

	/* read the fat */
	if ((fat = read_fat(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the bios parameter block */
	print_bpb(&boot_sect->BPB);

	/* read the root directory */
	if ((root_dir = read_root_dir(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the root directory */
	// printf("\n\n\n");
	messagefile_ptr = print_directory(FileNameToBeUse, &boot_sect->BPB, fat, root_dir, 0, messagefile_size);

cleanup:

	/* free the used memory */
	if (f != NULL)
	{
		free(f);
	}

	if (root_dir != NULL)
	{
		free(root_dir);
	}

	if (fat != NULL)
	{
		free(fat);
	}

	if (boot_sect != NULL)
	{
		free(boot_sect);
	}

	/* close the file */
	if (device != NULL)
	{
		fclose(device);
	}

	return messagefile_ptr;
}
char *fat12_ReadConfigFile(uint32_t *messagefile_size)
{
	char *messagefile_ptr = NULL;

	BOOT_SECTOR *boot_sect = NULL;
	unsigned char *fat = NULL;
	DIRECTORY_ENTRY *root_dir = NULL;
	unsigned char *f = NULL;

	/* check for null pointers */
	if (messagefile_size == NULL)
	{
		return NULL;
	}
	/* check that the device is mounted */
	if (false == msc_mounted)
	{
		*messagefile_size = 0;
		return NULL;
	}
	/* open the file for reading in binary mode */
	device = fmemopen(appl_msc_disk, APPL_DISK_BLOCK_NUM * APPL_DISK_BLOCK_SIZE, "r");

	if (device == NULL)
	{
		// printf("&\n");
		// fprintf(stderr, "Could not open device %s\n", filename);
		goto cleanup;
	}

	/* read the boot sector */
	if ((boot_sect = read_boot_sect()) == NULL)
	{
		goto cleanup;
	}

	/* read the fat */
	if ((fat = read_fat(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the bios parameter block */
	print_bpb(&boot_sect->BPB);

	/* read the root directory */
	if ((root_dir = read_root_dir(&boot_sect->BPB)) == NULL)
	{
		goto cleanup;
	}

	/* print out the root directory */
	// printf("\n\n\n");
	messagefile_ptr = load_config_file(&boot_sect->BPB, fat, root_dir, 0, messagefile_size);

cleanup:

	/* free the used memory */
	if (f != NULL)
	{
		free(f);
	}

	if (root_dir != NULL)
	{
		free(root_dir);
	}

	if (fat != NULL)
	{
		free(fat);
	}

	if (boot_sect != NULL)
	{
		free(boot_sect);
	}

	/* close the file */
	if (device != NULL)
	{
		fclose(device);
	}

	return messagefile_ptr;
}
#endif