#ifndef MASTER_BOOT_RECORD_H
#define MASTER_BOOT_RECORD_H

#include "utils.h"

#pragma pack(push, 1) // without this, the compiler might (will?) insert padding and the code will not work.
typedef struct partition_entry_t
{
    uint8_t boot_indicator;
    uint8_t start_head;
    uint8_t start_sector; // (Bits 6-7 are the upper two bits for the Starting Cylinder field.)
    uint8_t start_cylinder;
    uint8_t system_id;
    uint8_t end_head;
    uint8_t end_sector; // (Bits 6-7 are the upper two bits for the ending cylinder field)
    uint8_t end_cylinder;
    uint32_t start_lba;
    uint32_t total_sectors;
} partition_entry; // size: 16 bytes
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct master_boot_record_t
{
    uint8_t bootstrap_code[446];
    partition_entry partition_table[N_PARTITION];
    uint16_t signature;
} master_boot_record; // size: 512 bytes
#pragma pack(pop)

/*
 * extract master boot record from image file into structure
 */
int extract_mbr(FILE *disk_image, master_boot_record *mbr);

/*
 * print the partitions information as asked in the homework
 */
void print_partitions(master_boot_record *mbr);

#endif