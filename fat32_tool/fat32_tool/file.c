#include "file.h"

/* Given a hard-disk image file (.img) containing multiple partitions, a partition number
related to a FAT32 formatted partition in the volume and the absolute path of a file in
the volume, your tool should be able to parse through the FAT32 formatting and print
the content of the file on the standard output. Your tool should report an error if the
path specified cannot be found. */

static int is_absolute(char *path)
{
    return path[0] == '/';
}

char **tokenize_path(char *path, int *num_tokens)
{
    // if the path is not absolute, error
    if (!is_absolute(path))
    {
        fprintf(stderr, "Error: specified path is not absolute.\n");
        exit(1);
    }

    // if the last character is a slash, error
    int length = strlen(path);
    if (path[length - 1] == '/')
    {
        fprintf(stderr, "Error: specified path cannot end with a slash.\n");
        exit(1);
    }

    // each token must be 8 characters long
    char **tokens = (char **)malloc(sizeof(char *) * 100);
    int i = 0;
    char *token = strtok(path, "/");
    while (token != NULL)
    {
        tokens[i] = (char *)malloc(sizeof(char) * 8);
        // initialize token with spaces
        for (int j = 0; j < 8; j++)
        {
            tokens[i][j] = ' ';
        }

        // copy only the length of the token, and leave the rest of the token as spaces
        int length = strlen(token);
        if (length > 8)
        {
            fprintf(stderr, "Token length must be less than 8.\n");
            exit(1);
        }
        for (int j = 0; j < length; j++)
        {
            tokens[i][j] = token[j];
        }
        token = strtok(NULL, "/");
        i++;
    }
    *num_tokens = i;
    return tokens;
}

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

void print_file_content(FILE *disk_image, uint32_t cluster, const boot_sector *bs, const partition_entry *entry, int level, char **path_tokens, int num_tokens, int current_token, int *found)
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
                // if the entry is a directory
                if (!(entries[i].attributes & DIRECTORY_ATTRIBUTE))
                {
                    // take the name of the entry and pad it with spaces if needed, in a 8 bytes buffer
                    char filename[8];
                    for (int j = 0; j < 8; j++)
                    {
                        if (entries[i].name[j] != ' ' && entries[i].name[j] != 0)
                        {
                            filename[j] = tolower(entries[i].name[j]);
                        }
                        else
                        {
                            filename[j] = ' ';
                        }
                    }

                    /* --------------- FIXME: It is said we don't need to account for extensions, remove below if not needed --------------*/

                    if (entries[i].extension[0] != ' ')
                    {
                        // make the 5th byte of the buffer a dot
                        filename[4] = '.';

                        // take the extension of the entry and copy it to the 3 last bytes of the buffer
                        for (int j = 0; j < 3; j++)
                        {
                            if (entries[i].extension[j] != ' ')
                            {
                                filename[j + 5] = tolower(entries[i].extension[j]);
                            }
                            else
                            {
                                filename[j + 5] = ' ';
                            }
                        }
                    }
                    /* --------------- until here --------------*/

                    // if the entry name is the same as the current token, and the token is the last one, print the content of the file
                    if (strncmp(filename, path_tokens[current_token], 8) == 0 && current_token == num_tokens - 1)
                    {
                        *found = 1;
                        // compute the first cluster of the file
                        uint32_t first_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                        // compute the size of the file in clusters
                        uint32_t file_size_in_clusters = (entries[i].file_size + cluster_size - 1) / cluster_size;

                        // read and print the content of the file in hex, cluster by cluster
                        uint32_t current_cluster = first_cluster;
                        uint8_t *cluster_buffer = malloc(cluster_size);
                        for (uint32_t j = 0; j < file_size_in_clusters; j++)
                        {
                            read_cluster(disk_image, current_cluster, bs, entry, cluster_buffer);

                            // print the content in hex, 16 bytes per line, and the ascii representation of the bytes on the right
                            for (uint32_t k = 0; k < cluster_size; k++)
                            {
                                if (k % 16 == 0)
                                    fprintf(stdout, "\n%08X: ", k);

                                fprintf(stdout, "%02X ", cluster_buffer[k]);

                                if (k % 16 == 15)
                                {
                                    fprintf(stdout, " | ");
                                    for (uint32_t l = k - 15; l <= k; l++)
                                    {
                                        if (isprint(cluster_buffer[l]))
                                            fprintf(stdout, "%c", cluster_buffer[l]);
                                        else
                                            fprintf(stdout, ".");
                                    }
                                }
                            }

                            current_cluster = get_next_cluster(disk_image, current_cluster, bs, entry);
                        }

                        free(cluster_buffer);
                    }
                }

                // if the entry is a subdirectory,
                if ((entries[i].attributes & DIRECTORY_ATTRIBUTE) && strncmp((char *)entries[i].name, ".", 1) != 0 && strncmp((char *)entries[i].name, "..", 2) != 0)
                {
                    // take the name of the entry and pad it with spaces if needed, in a 8 bytes buffer
                    char subdirectory_name[8];
                    memset(subdirectory_name, ' ', 8);
                    for (int j = 0; j < SHORT_NAME_LENGTH; j++)
                    {
                        if (entries[i].name[j] != ' ' && entries[i].name[j] != 0)
                        {
                            subdirectory_name[j] = tolower(entries[i].name[j]);
                        }
                    }

                    if (current_token < num_tokens && strncmp(subdirectory_name, path_tokens[current_token], 8) == 0)
                    {
                        // print the content of the subdirectory
                        uint32_t sub_directory_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                        print_file_content(disk_image, sub_directory_cluster, bs, entry, level + 1, path_tokens, num_tokens, current_token + 1, found);
                    }
                }
            }
        }

        // move to the next cluster in the chain
        cluster = get_next_cluster(disk_image, cluster, bs, entry);
    }

    free(cluster_data);
}