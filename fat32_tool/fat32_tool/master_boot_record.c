#include "master_boot_record.h"

// print information on single partition (PART 1)
static void print_partition_info(partition_entry *entry, int partition_number)
{
	uint8_t system_id = entry->system_id;
	uint8_t start_sector = entry->start_sector & 0x3F;
	uint8_t end_sector = entry->end_sector & 0x3F;
	uint32_t total_sectors = entry->total_sectors; // size
	uint32_t start_lba = entry->start_lba;
	uint64_t start_byte = (uint64_t)start_lba * SECTOR_SIZE;					  // partition starting byte
	uint64_t end_byte = start_byte + ((uint64_t)total_sectors * SECTOR_SIZE) - 1; // partition ending byte
	uint64_t size_bytes = (uint64_t)total_sectors * SECTOR_SIZE;				  // partition size

	printf("Partition %d:\n", partition_number);
	printf("  System ID:    0x%02x (FAT32 with LBA addressing)\n", system_id);
	printf("  Start Sector: %u\n", start_sector);
	printf("  End Sector:   %u\n", end_sector);
	printf("  Sector size:  %u bytes\n", SECTOR_SIZE);
	printf("  Start LBA:    %u\n", start_lba);
	printf("  Start Byte:   %lu\n", start_byte);
	printf("  End Byte:     %lu\n", end_byte);
	printf("  Size:         %u sectors (%ld bytes)\n", total_sectors, size_bytes);
}

int extract_mbr(FILE *disk_image, master_boot_record *mbr)
{
	if (fread(mbr, sizeof(*mbr), 1, disk_image) != 1)
	{
		perror("Error");
		return 1;
	}

	// Verify that the signature is 0xAA55
	if (mbr->signature != 0xAA55)
	{
		fprintf(stderr, "Wrong Master Boot Record signature\n");
		return 1;
	}
	return 0;
}

void print_partitions(master_boot_record *mbr)
{
	for (int i = 0; i < N_PARTITION; i++)
	{
		if (mbr->partition_table[i].system_id == PARTITION_TYPE)
		{
			print_partition_info(&mbr->partition_table[i], i + 1);
		}
	}
}