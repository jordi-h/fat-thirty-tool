#ifndef PARTITION_H
#define PARTITION_H

#include "utils.h"
#include "master_boot_record.h"

#pragma pack(push, 1)
typedef struct boot_sector_t
{
	uint8_t jmp_boot[3]; // jump instruction to boot code
	uint8_t oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors_count;
	uint8_t num_fats;
	uint16_t root_entries_count;
	uint16_t total_sectors_16;
	uint8_t media;
	uint16_t fat_size_16;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors_32;
	uint32_t fat_size_32;
	uint16_t ext_flags;
	uint16_t fs_version;
	uint32_t root_cluster;
	uint16_t fs_info;
	uint16_t backup_boot_sector;
	uint8_t reserved[12];
	uint8_t drive_number;
	uint8_t reserved1;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t fs_type[8];
} boot_sector; // size: 512 bytes
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct directory_table_entry_t
{
	uint8_t name[8];
	uint8_t extension[3];
	uint8_t attributes;
	uint8_t reseved; // Reserved for use by Windows NT
	uint8_t created_time_tenths;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t last_access_date;
	uint16_t first_cluster_high;
	uint16_t write_time; // Time of last write
	uint16_t write_date; // Date of last write
	uint16_t first_cluster_low;
	uint32_t file_size;
} directory_table_entry; // size: 32 bytes
#pragma pack(pop)

/*
 * extract boot sector from image file into structure
 */
int extract_bs(FILE *disk_image, const partition_entry *entry, boot_sector *bs);

/*
 * print the boot sector information as asked in the homework
 */
void print_bootsector(boot_sector *bs);

/*
 *  print the directory/file tree structure by iterating through the clusters and directory entries recursively
 */
void print_tree(FILE *disk_image, uint32_t cluster, const boot_sector *bs, const partition_entry *entry, int level);

#endif