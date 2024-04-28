
#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "sdkconfig.h"
#include "freertos/stream_buffer.h"
#include "tusb.h"
#include <esp_partition.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "errno.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "esp_check.h"
#include <stdio.h>
#include <string.h>
#include "fat12.h"
#include "main_if.h"
#include "lvgl_if.h"

#if CFG_TUD_MSC
static QueueHandle_t fat12_queue;
static void init_disk();
// Some MCU doesn't have enough 8KB SRAM to store the whole disk
// We will use Flash as read-only disk with board that has
// CFG_EXAMPLE_MSC_READONLY defined

#define FILE_CONFIG_CONTENTS "{\"FileNameToBeUse\": \"FILE_NAME.BIN\",\"NOTE\": \"change FileNameToBeUse data attribute with your file name(max name length is 11 inclusive extension)\"}"

#define FAT_U8(v) ((v) & 0xFF)
#define FAT_U16(v) FAT_U8(v), FAT_U8((v) >> 8)
#define FAT_U32(v) FAT_U8(v), FAT_U8((v) >> 8), FAT_U8((v) >> 16), FAT_U8((v) >> 24)
#define FAT_MS2B(s, ms) FAT_U8(((((s) & 0x1) * 1000) + (ms)) / 10)
#define FAT_HMS2B(h, m, s) FAT_U8(((s) >> 1) | (((m) & 0x7) << 5)), FAT_U8((((m) >> 3) & 0x7) | ((h) << 3))
#define FAT_YMD2B(y, m, d) FAT_U8(((d) & 0x1F) | (((m) & 0x7) << 5)), FAT_U8((((m) >> 3) & 0x1) | ((((y)-1980) & 0x7F) << 1))
#define FAT_TBL2B(l, h) FAT_U8(l), FAT_U8(((l >> 8) & 0xF) | ((h << 4) & 0xF0)), FAT_U8(h >> 4)

const uint8_t _msc_disk[5][APPL_DISK_BLOCK_SIZE] =
    {
        //------------- Block0: Boot Sector -------------//
        // byte_per_sector    = APPL_DISK_BLOCK_SIZE; fat12_sector_num_16  = APPL_DISK_BLOCK_NUM;
        // sector_per_cluster = 1; reserved_sectors = 1;
        // fat_num            = 1; fat12_root_entry_num = 16;
        // sector_per_fat     = 1; sector_per_track = 1; head_num = 1; hidden_sectors = 0;
        // drive_number       = 0x80; media_type = 0xf8; extended_boot_signature = 0x29;
        // filesystem_type    = "FAT12   "; volume_serial_number = 0x1234; volume_label = "TinyUSB MSC";
        // FAT magic code at offset 510-511
        {
            FAT_U8(0xEB), 0x3C, 0x90, 'M', 'S', 'D', 'O', 'S', '5', '.', '0',
            FAT_U16(APPL_DISK_BLOCK_SIZE),        // bytes_per_sector
            FAT_U8(1),                            // sectors_per_cluster
            FAT_U16(1),                           // reserved_sectors_count
            FAT_U8(1),                            // file_alloc_tables_num
            FAT_U16(16),                          // max_root_dir_entries
            FAT_U16(APPL_DISK_BLOCK_NUM),         // fat12_sector_num
            0xF8,                                 // media_descriptor
            FAT_U16(APPL_DISC_SECTORS_PER_TABLE), // sectors_per_alloc_table;//FAT12 and FAT16
            FAT_U16(1),                           // sectors_per_track;//A value of 0 may indicate LBA-only access
            FAT_U16(1),                           // num_heads
            FAT_U32(0),                           // hidden_sectors_count
            FAT_U32(0),                           // total_sectors_32
            0x00,                                 // physical_drive_number;0x00 for (first) removable media, 0x80 for (first) fixed disk
            0x00,                                 // reserved
            0x29,                                 // extended_boot_signature;//should be 0x29
            FAT_U32(0x1234),                      // serial_number: 0x1234 => 1234
            'C', 'A', 'N', 'I', 'O', 'T', ' ', 'T', 'E', 'A', 'M',
            'F', 'A', 'T', '1', '2', ' ', ' ', ' ',

            // Zero up to 2 last bytes of FAT magic code (448 bytes)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

            // boot signature (2 bytes)
            0x55, 0xAA},

        //------------- Block1: FAT12 Table -------------//
        {
            FAT_TBL2B(0xFF8, 0xFFF), FAT_TBL2B(0xFFF, 0x000) // first 2 entries must be 0xFF8 0xFFF, third entry is cluster end of readme file
        },

        //------------- Block2: Root Directory -------------//
        //------------- Block2: Root Directory -------------//
        {
            // first entry is volume label
            'C', 'A', 'N', 'I', 'O', 'T', 'B', 'O', 'X', ' ', ' ',
            0x08, // FILE_ATTR_VOLUME_LABEL
            0x00,
            FAT_MS2B(0, 0),
            FAT_HMS2B(0, 0, 0),
            FAT_YMD2B(0, 0, 0),
            FAT_YMD2B(0, 0, 0),
            FAT_U16(0),
            FAT_HMS2B(13, 42, 30),  // last_modified_hms
            FAT_YMD2B(2018, 11, 5), // last_modified_ymd
            FAT_U16(0),
            FAT_U32(0),

            // second entry is readme file
            'C', 'O', 'N', 'F', 'I', 'G', ' ', ' ',   // file_name[8]; padded with spaces (0x20)
            'C', 'F', 'G',                            // file_extension[3]; padded with spaces (0x20)
            0x20,                                     // file attributes: FILE_ATTR_ARCHIVE
            0x00,                                     // ignore
            FAT_MS2B(1, 980),                         // creation_time_10_ms (max 199x10 = 1s 990ms)
            FAT_HMS2B(13, 42, 36),                    // create_time_hms [5:6:5] => h:m:(s/2)
            FAT_YMD2B(2018, 11, 5),                   // create_time_ymd [7:4:5] => (y+1980):m:d
            FAT_YMD2B(2020, 11, 5),                   // last_access_ymd
            FAT_U16(0),                               // extended_attributes
            FAT_HMS2B(13, 44, 16),                    // last_modified_hms
            FAT_YMD2B(2019, 11, 5),                   // last_modified_ymd
            FAT_U16(2),                               // start of file in cluster
            FAT_U32(sizeof(FILE_CONFIG_CONTENTS) - 1) // file size
        },
        FILE_CONFIG_CONTENTS
        //------------- Block3: Readme Content -------------//
};
uint8_t WORD_ALIGNED_ATTR *appl_msc_disk = NULL; // &_msc_disk[0];
typedef uint8_t (*msc_disk_t)[APPL_DISK_BLOCK_SIZE];
extern void UDS_Flash_Task(void *filename);
static void lvgl_msgBox_event_handler(lv_obj_t *obj, lv_event_t event)
{
  uint16_t bnt = lv_msgbox_get_active_btn(obj);
  if (event == LV_EVENT_CLICKED)
  {
    if (bnt == 1)
    {
      xTaskCreatePinnedToCore(UDS_Flash_Task, "Flash_Task1", 4096, (void *)"/sdcard/uds_flashLoader/S32K1_uds_flash_CAN_BlueLed.json", 15, NULL, 1);
    }
    lv_msgbox_start_auto_close(obj, 0);
  }
}
static void lvgl_msgBox_show()
{
  lvgl_if_take();

  static const char *btns2[] = {"Cancel", "Ok", ""};

  lv_obj_t *m = lv_msgbox_create(lv_scr_act(), NULL);
  lv_msgbox_set_text(m, "new file is availabe\n do you want to update?");
  lv_msgbox_add_btns(m, btns2);
  lv_obj_set_event_cb(m, lvgl_msgBox_event_handler);
  lv_obj_t *btnm = lv_msgbox_get_btnmatrix(m);
  lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECK_STATE);
  lvgl_if_give();
}
void fat12Task(void *TaskArg)
{
  int16_t message_actions = -1;

  fat12_queue = xQueueCreate(100, sizeof(uint16_t));

  init_disk();
  while (1)
  {
    if (xQueueReceive(fat12_queue, &message_actions, portMAX_DELAY))
    {
      do
      {
        // vTaskDelay(1000);
      } while (xQueueReceive(fat12_queue, &message_actions, 1000));
      char buffer[10][11];
      memset(buffer, '\0', 10 * 11);
      char extenBuffer[2][4] = {"bin", "hex"};
      int fileFound = 0;
      fat12_findFiles(&fileFound, extenBuffer, buffer);
      if (fileFound > 0)
      {
        lvgl_msgBox_show();
      }
    }
  }
  vTaskDelete(NULL);
}
static void init_disk()
{
  appl_msc_disk = (uint8_t *)heap_caps_calloc(APPL_DISK_BLOCK_NUM, APPL_DISK_BLOCK_SIZE, MALLOC_CAP_SPIRAM);
  assert(appl_msc_disk != NULL);
  memcpy(appl_msc_disk, &_msc_disk[0][0], sizeof(_msc_disk));
  memcpy(&appl_msc_disk[(APPL_DISC_SECTORS_PER_TABLE + 1) * APPL_DISK_BLOCK_SIZE], &_msc_disk[2][0], APPL_DISK_BLOCK_SIZE);
  memcpy(&appl_msc_disk[(APPL_DISC_SECTORS_PER_TABLE + 2) * APPL_DISK_BLOCK_SIZE], FILE_CONFIG_CONTENTS, strlen(FILE_CONFIG_CONTENTS));
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
__attribute__((used)) int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  (void)lun;

  uint8_t *addr = &appl_msc_disk[lba * APPL_DISK_BLOCK_SIZE] + offset;
  memcpy(addr, buffer, bufsize);

  uint16_t message_actions = 1;
  xQueueSend(fat12_queue, &message_actions, portMAX_DELAY);
  return bufsize;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
__attribute__((used)) int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  (void)lun;

  uint8_t *addr = &appl_msc_disk[lba * APPL_DISK_BLOCK_SIZE] + offset;
  memcpy(buffer, addr, bufsize);

  return bufsize;
}
// Callback invoked when WRITE10 command is completed (status received and accepted by host).
__attribute__((used)) void tud_msc_write10_complete_cb(uint8_t lun)
{
}
__attribute__((used)) void tud_mount_cb(void)
{
  msc_mounted = true;
}
// Invoked when device is unplugged
__attribute__((used)) void tud_umount_cb(void)
{
  msc_mounted = false;
}

__attribute__((used)) void tud_msc_scsi_complete_cb(uint8_t lun, uint8_t const scsi_cmd[16])
{
}
// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
__attribute__((used)) void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void)lun;

  const char vid[] = "caniot team";
  const char pid[] = "CaniotBof";
  const char rev[] = "1.0";

  memcpy(vendor_id, vid, strlen(vid));
  memcpy(product_id, pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
__attribute__((used)) bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void)lun;

  return true; // RAM disk is always ready
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
__attribute__((used)) void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
  (void)lun;
  *block_count = APPL_DISK_BLOCK_NUM;
  *block_size = APPL_DISK_BLOCK_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
__attribute__((used)) bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void)lun;
  (void)power_condition;

  if (load_eject)
  {
    if (start)
    {
      // load disk storage
    }
    else
    {
      // unload disk storage
    }
  }

  return true;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
__attribute__((used)) int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const *response = NULL;
  uint16_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
  case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
    // Host is about to read/write etc ... better not to disconnect disk
    resplen = 0;
    break;

  default:
    // Set Sense = Invalid Command Operation
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

    // negative means error -> tinyusb could stall and/or response with failed status
    resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if (resplen > bufsize)
    resplen = bufsize;

  if (response && (resplen > 0))
  {
    if (in_xfer)
    {
      memcpy(buffer, response, resplen);
    }
    else
    {
      // SCSI output
    }
  }

  return resplen;
}

#endif
