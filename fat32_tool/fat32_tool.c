/*
 * Tool for performing forensic analysis on FAT32 file system.
 * auhtors: Loïc Meunier, Jordi Hoorelbeke
 */

#include "utils.h"
#include "master_boot_record.h"
#include "partition.h"
#include "file.h"

int main(int argc, char *argv[])
{
	// check if right number of arguments
	if (argc < 2 || argc > 4)
	{
		fprintf(stderr, "Usage 1: %s <disk_image.img>\n", argv[0]);
		fprintf(stderr, "Usage 2: %s <disk_image.img> <partition_number>\n", argv[0]);
		fprintf(stderr, "Usage 3: %s <disk_image.img> <partition_number> <absolute_path>\n", argv[0]);
		return 1;
	}

	// open the disk image
	FILE *disk_image = fopen(argv[1], "rb");
	if (!disk_image)
	{
		perror("Error");
		return 1;
	}

	// extract master boot record
	master_boot_record mbr;
	if (extract_mbr(disk_image, &mbr) == 1)
	{
		fclose(disk_image);
		return 1;
	}

	// PART 1
	if (argc == 2)
	{
		print_partitions(&mbr);
		fclose(disk_image);
		return 0;
	}

	// PART 2 & 3
	// extract boot sector from partition
	boot_sector bs;
	int partition_number = atoi(argv[2]) - 1;
	if (extract_bs(disk_image, &mbr.partition_table[partition_number], &bs) == 1)
	{
		fclose(disk_image);
		return 1;
	}

	// PART 2
	if (argc == 3)
	{
		print_bootsector(&bs);
		printf("\nfile / directory tree:\n");
		print_tree(disk_image, bs.root_cluster, &bs, &mbr.partition_table[partition_number], 0);
		fclose(disk_image);
		return 0;
	}

	// PART 3
	if (argc == 4)
	{
		int num_tokens;
		char** path_tokens = tokenize_path(argv[3], &num_tokens);
		int found = 0;
			
		print_file_content(disk_image, bs.root_cluster, &bs, &mbr.partition_table[partition_number], 0, path_tokens, num_tokens, 0, &found);
		if (!found)
		{
			fprintf(stderr, "Error: specified path does not exist.\n");
			fclose(disk_image);
			return 1;
		}
	}

	fclose(disk_image);
	return 0;
}