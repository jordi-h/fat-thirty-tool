#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "partition.h"
#include "master_boot_record.h"

void print_file_content(FILE *disk_image, uint32_t cluster, const boot_sector *bs, const partition_entry *entry, int level, char** path_tokens, int num_tokens, int current_token, int* found);

char** tokenize_path(char* path, int* num_tokens);

#endif // __FILE_H__