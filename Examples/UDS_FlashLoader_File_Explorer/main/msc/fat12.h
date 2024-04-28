#ifndef _FAT12_H_
#define _FAT12_H_
#pragma pack(1)
typedef struct {
	unsigned short	BytesPerSector;
	unsigned char	SectorsPerCluster;
	unsigned short	ReservedSectors;
	unsigned char	NumFATS;
	unsigned short	RootDirEntries;
	unsigned short	SmallSectors;
	unsigned char	MediaDescriptor;
	unsigned short	SectorsPerFAT;
	unsigned short	SectorsPerTrack;
	unsigned short	Heads;	
	unsigned int	HiddenSectors;	
	unsigned int	LargeSectors;
} BIOS_PARAMETER_BLOCK;


#pragma pack(1)
typedef struct {
	unsigned char	DriveNum;
	unsigned char	CurrentHead;
	unsigned char	Signature;
	unsigned int	VolumeID;
	unsigned char	VolumeLabel[11];
	unsigned char	FileSystem[8];
} EXTENDED_BIOS_PARAMETER_BLOCK;


#pragma pack(1)
typedef struct {
	unsigned char					JmpInstruction[3];
	unsigned char					OEMID[8];
	BIOS_PARAMETER_BLOCK 			BPB;
	EXTENDED_BIOS_PARAMETER_BLOCK	ExtendedBPB;
	unsigned char					BootStrapLoader[448];
	unsigned short					BootSignature;
} BOOT_SECTOR;


#pragma pack(1)
typedef struct {
	unsigned char Seconds:5;
	unsigned char Minutes:6;
	unsigned char Hours:5;
} FILE_TIME;


#pragma pack(1)
typedef struct {
	unsigned char Day:5;
	unsigned char Month:4;
	unsigned char Year:7;
} FILE_DATE;


#pragma pack(1)
typedef struct {
	unsigned char		FileName[11];
	unsigned char		Attributes;
	unsigned char		Reserved[2];
	FILE_TIME			TimeStamp;
	FILE_DATE			DateStamp;
	FILE_DATE			AccessDate;
	unsigned short		ClusterHi;
	FILE_TIME			WriteTime;
	FILE_DATE			WriteDate;
	unsigned short		Cluster;
	unsigned int		FileSize;
} DIRECTORY_ENTRY;


#define ATTR_READ_ONLY   	0x01
#define ATTR_HIDDEN 		0x02
#define ATTR_SYSTEM 		0x04
#define ATTR_VOLUME_ID 		0x08
#define ATTR_DIRECTORY		0x10
#define ATTR_ARCHIVE  		0x20


#define END_OF_FILE			0xFF0


#define FAT12_FREE			0x000
#define FAT12_BAD			0xFF7
#define	FAT12_END			0xFF8

#define	DIRENTRY_DELETED	0xE5
#define DIRENTRY_END		0x00



enum
{
  APPL_DISK_BLOCK_NUM = 3072, // 8KB is the smallest size that windows allow to mount
  APPL_DISK_BLOCK_SIZE = 512,
  APPL_DISC_SECTORS_PER_TABLE = 32  // 1 mean each table sector can fit 170KB (340 sectors), 0 not allowed
};

extern void fat12_findFiles(int* fileFound,char (*extensions)[4],char (*buffer)[11]);
extern char * fat12_ReadFile(const char * FileNameToBeUse,uint32_t *messagefile_size);
extern char * fat12_ReadConfigFile(uint32_t *messagefile_size);
extern uint8_t __attribute__((aligned(4))) *appl_msc_disk;
typedef uint8_t (*msc_disk_t)[APPL_DISK_BLOCK_SIZE];
extern bool msc_mounted;
#endif