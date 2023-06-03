#include "partition.h"

/*
 * calculate the starting sector number of a cluster
 */
static uint32_t calculate_sector(const boot_sector *bs, const partition_entry *entry, uint32_t cluster)
{
    uint32_t first_data_sector = entry->start_lba + bs->reserved_sectors_count + (bs->num_fats * bs->fat_size_32);
    return first_data_sector + (cluster - 2) * bs->sectors_per_cluster;
}

/*
 * read a specified amount of data from a disk image, starting at a given sector and offset, into a buffer
 */
static void read_data(FILE *disk_image, uint32_t sector, uint32_t offset, uint32_t size, void *buffer)
{
    fseek(disk_image, (long)sector * SECTOR_SIZE + offset, SEEK_SET);
    fread(buffer, size, 1, disk_image);
}

/*
 * put the entire content of a cluster into a buffer.
 */
static void read_cluster(FILE *disk_image, uint32_t cluster, const boot_sector *bs, const partition_entry *entry, uint8_t *buffer)
{
    uint32_t sector = calculate_sector(bs, entry, cluster);
    read_data(disk_image, sector, 0, bs->sectors_per_cluster * SECTOR_SIZE, buffer);
}

/*
 *  read the next cluster number from the FAT
 */
static uint32_t get_next_cluster(FILE *disk_image, uint32_t current_cluster, const boot_sector *bs, const partition_entry *entry)
{
    uint32_t fat_offset = current_cluster * sizeof(uint32_t);
    uint32_t fat_sector = entry->start_lba + bs->reserved_sectors_count + (fat_offset / bs->bytes_per_sector);
    uint32_t entry_offset = fat_offset % bs->bytes_per_sector;

    uint32_t next_cluster;
    read_data(disk_image, fat_sector, entry_offset, sizeof(uint32_t), &next_cluster);

    // remove the upper 4 bits (reserved)
    return next_cluster & 0x0FFFFFFF;
}

/*
 * print the name and extension of a directory entry
 */
static void print_directory_entry_name(const directory_table_entry *entry)
{
    int dot_printed = 0; // flag so the file extension dot is printed once
    for (int i = 0; i < SHORT_NAME_LENGTH; i++)
    {
        // is not a deleted directory entry
        if (entry->name[i] == 0x05)
        {
            putchar(0xE5);
        }
        else if (entry->name[i] != ' ' && entry->name[i] != 0)
        {
            putchar(tolower(entry->name[i]));
        }
    }

    for (int i = 0; i < 3; i++)
    {
        if (entry->extension[i] != ' ')
        {
            if (!dot_printed)
            {
                putchar('.');
                dot_printed = 1;
            }
            putchar(tolower(entry->extension[i]));
        }
    }
}

int extract_bs(FILE *disk_image, const partition_entry *entry, boot_sector *bs)
{
    fseek(disk_image, (long)entry->start_lba * SECTOR_SIZE, SEEK_SET);
    if (fread(bs, sizeof(*bs), 1, disk_image) != 1)
    {
        perror("Error");
        return 1;
    }

    // Verify that the partition is FAT32 formatted
    if (strncmp((char *)bs->fs_type, "FAT32", 5) != 0)
    {
        fprintf(stderr, "Error: partition is not properly FAT32 formatted\n");
        return 1;
    }
    return 0;
}

void print_bootsector(boot_sector *bs)
{
    // Extract information from the BPB
    uint8_t sectors_per_cluster = bs->sectors_per_cluster;
    uint16_t bytes_per_sector = bs->bytes_per_sector;
    uint8_t num_fats = bs->num_fats;
    uint16_t reserved_sectors_count = bs->reserved_sectors_count;
    uint32_t root_cluster = bs->root_cluster;
    uint32_t fat_size_32 = bs->fat_size_32;

    printf("Sectors per cluster: %u\n", sectors_per_cluster);
    printf("Bytes per sector: %u\n", bytes_per_sector);
    printf("Number of FATs: %u\n", num_fats);
    printf("FAT size: %u\n", fat_size_32);
    printf("Reserved sectors count: %u\n", reserved_sectors_count);
    printf("Root cluster: %u\n", root_cluster);
}

void print_tree(FILE *disk_image, uint32_t cluster, const boot_sector *bs, const partition_entry *entry, int level)
{
    uint32_t cluster_size = bs->sectors_per_cluster * bs->bytes_per_sector;
    uint8_t *cluster_data = malloc(cluster_size); // buffer
    uint32_t entries_per_cluster = cluster_size / sizeof(directory_table_entry);

    // loop through the clusters
    while (cluster >= 2 && cluster < END_OF_CLUSTER_CHAIN)
    {
        read_cluster(disk_image, cluster, bs, entry, cluster_data);

        // cast cluster data to a pointer to an array of directory_table_entry
        directory_table_entry *entries = (directory_table_entry *)cluster_data;

        // loop through each directory entry in the current cluster
        for (uint32_t i = 0; i < entries_per_cluster; i++)
        {
            if (entries[i].name[0] == END_ENTRY_NAME)
                break;

            if (entries[i].name[0] != FREE_ENTRY_NAME && !(entries[i].attributes & VOLUME_LABEL_ATTRIBUTE))
            {
                for (int j = 0; j < level; j++)
                    putchar(' ');

                print_directory_entry_name(&entries[i]);
                putchar('\n');

                // if the entry is a subdirectory, print its tree recursively
                if ((entries[i].attributes & DIRECTORY_ATTRIBUTE) && strncmp((char *)entries[i].name, ".", 1) != 0 && strncmp((char *)entries[i].name, "..", 2) != 0)
                {
                    uint32_t sub_directory_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                    print_tree(disk_image, sub_directory_cluster, bs, entry, level + 1);
                }
            }
        }

        // move to the next cluster in the chain
        cluster = get_next_cluster(disk_image, cluster, bs, entry);
    }

    free(cluster_data);
}