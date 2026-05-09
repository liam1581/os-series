#include "x86_64/atapi.h"
#include "x86_64/port.h"

// Primary IDE channel
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DRIVE_SELECT 0x1F6
#define ATA_PRIMARY_STATUS       0x1F7
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_ALT_STATUS   0x3F6

// Secondary IDE channel
#define ATA_SECONDARY_DATA         0x170
#define ATA_SECONDARY_ERROR        0x171
#define ATA_SECONDARY_SECTOR_COUNT 0x172
#define ATA_SECONDARY_LBA_LOW      0x173
#define ATA_SECONDARY_LBA_MID      0x174
#define ATA_SECONDARY_LBA_HIGH     0x175
#define ATA_SECONDARY_DRIVE_SELECT 0x176
#define ATA_SECONDARY_STATUS       0x177
#define ATA_SECONDARY_COMMAND      0x177
#define ATA_SECONDARY_ALT_STATUS   0x376

// ATA status bits
#define ATA_STATUS_ERR  (1 << 0)
#define ATA_STATUS_DRQ  (1 << 3)
#define ATA_STATUS_BSY  (1 << 7)

// ATA/ATAPI commands
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_PACKET          0xA0

// ATAPI command
#define ATAPI_CMD_READ12        0xA8

// Signature bytes that identify an ATAPI device
#define ATAPI_SIG_MID 0x14
#define ATAPI_SIG_HI  0xEB

// Currently selected channel ports
static uint16_t port_data;
static uint16_t port_error;
static uint16_t port_sector_count;
static uint16_t port_lba_mid;
static uint16_t port_lba_high;
static uint16_t port_drive_select;
static uint16_t port_status;
static uint16_t port_command;
static uint16_t port_alt_status;

static void use_primary() {
    port_data         = ATA_PRIMARY_DATA;
    port_error        = ATA_PRIMARY_ERROR;
    port_sector_count = ATA_PRIMARY_SECTOR_COUNT;
    port_lba_mid      = ATA_PRIMARY_LBA_MID;
    port_lba_high     = ATA_PRIMARY_LBA_HIGH;
    port_drive_select = ATA_PRIMARY_DRIVE_SELECT;
    port_status       = ATA_PRIMARY_STATUS;
    port_command      = ATA_PRIMARY_COMMAND;
    port_alt_status   = ATA_PRIMARY_ALT_STATUS;
}

static void use_secondary() {
    port_data         = ATA_SECONDARY_DATA;
    port_error        = ATA_SECONDARY_ERROR;
    port_sector_count = ATA_SECONDARY_SECTOR_COUNT;
    port_lba_mid      = ATA_SECONDARY_LBA_MID;
    port_lba_high     = ATA_SECONDARY_LBA_HIGH;
    port_drive_select = ATA_SECONDARY_DRIVE_SELECT;
    port_status       = ATA_SECONDARY_STATUS;
    port_command      = ATA_SECONDARY_COMMAND;
    port_alt_status   = ATA_SECONDARY_ALT_STATUS;
}

static void ata_wait_bsy() {
    while (port_inb(port_status) & ATA_STATUS_BSY);
}

static void ata_wait_drq() {
    while (!(port_inb(port_status) & ATA_STATUS_DRQ));
}

static bool ata_check_error() {
    return (port_inb(port_status) & ATA_STATUS_ERR) != 0;
}

// Try to detect and init an ATAPI drive on the currently selected channel/drive
// drive_select: 0xA0 = master, 0xB0 = slave
static bool try_init_drive(uint8_t drive_select) {
    // Select drive
    port_outb(port_drive_select, drive_select);

    // Give the drive time to respond
    for (int i = 0; i < 15; i++) port_wait();

    // Check if anything is there at all — 0xFF means no drive
    uint8_t status = port_inb(port_status);
    if (status == 0xFF || status == 0x00) return false;

    // Reset
    port_outb(port_alt_status, 0x04);
    for (int i = 0; i < 5; i++) port_wait();
    port_outb(port_alt_status, 0x00);
    for (int i = 0; i < 5; i++) port_wait();

    ata_wait_bsy();

    // Check ATAPI signature in LBA mid/high — ATAPI drives return 0x14/0xEB
    uint8_t mid = port_inb(port_lba_mid);
    uint8_t hi  = port_inb(port_lba_high);

    if (mid != ATAPI_SIG_MID || hi != ATAPI_SIG_HI) return false;

    // Send IDENTIFY PACKET command to confirm
    port_outb(port_command, ATA_CMD_IDENTIFY_PACKET);
    for (int i = 0; i < 5; i++) port_wait();

    ata_wait_bsy();

    if (port_inb(port_status) == 0) return false;
    if (ata_check_error())          return false;

    ata_wait_drq();

    // Read and discard identify data
    for (int i = 0; i < 256; i++) port_inw(port_data);

    return true;
}

bool atapi_init() {
    // Try all 4 possible locations: primary master, primary slave,
    // secondary master, secondary slave
    //use_primary();
    //if (try_init_drive(0xA0)) return true;  // primary master
    //if (try_init_drive(0xB0)) return true;  // primary slave

    use_secondary();
    if (try_init_drive(0xA0)) return true;  // secondary master
    if (try_init_drive(0xB0)) return true;  // secondary slave

    return false;
}

bool atapi_read_sector(uint32_t lba, uint8_t* buffer) {
    port_outb(port_drive_select, 0xA0);
    port_outb(port_error,        0x00);
    port_outb(port_lba_mid,      0x00);   // byte count low
    port_outb(port_lba_high,     0x08);   // byte count high (0x0800 = 2048)
    port_outb(port_command,      ATA_CMD_PACKET);

    ata_wait_bsy();
    ata_wait_drq();
    if (ata_check_error()) return false;

    // 12-byte ATAPI READ(12) packet
    uint8_t packet[12] = {
        ATAPI_CMD_READ12,
        0x00,
        (uint8_t)((lba >> 24) & 0xFF),
        (uint8_t)((lba >> 16) & 0xFF),
        (uint8_t)((lba >> 8)  & 0xFF),
        (uint8_t)((lba >> 0)  & 0xFF),
        0x00,
        0x00,
        0x00,
        0x01,   // read 1 sector
        0x00,
        0x00,
    };

    for (int i = 0; i < 6; i++) {
        uint16_t word = (uint16_t)packet[i * 2] | ((uint16_t)packet[i * 2 + 1] << 8);
        port_outw(port_data, word);
    }

    ata_wait_bsy();
    ata_wait_drq();
    if (ata_check_error()) return false;

    for (int i = 0; i < 1024; i++) {
        uint16_t word = port_inw(port_data);
        buffer[i * 2]     = (uint8_t)(word & 0xFF);
        buffer[i * 2 + 1] = (uint8_t)(word >> 8);
    }

    return true;
}