#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define SECTOR_SIZE 512
#define N_PARTITION 4
#define PARTITION_TYPE 0x0C // FAT32
#define END_OF_CLUSTER_CHAIN 0x0FFFFFF8
#define FREE_ENTRY_NAME 0xE5 // means entry is deleted
#define END_ENTRY_NAME 0 // means no more entries are present after the curent one
#define VOLUME_LABEL_ATTRIBUTE 0x08 // means the entry is not a file or a directory
#define DIRECTORY_ATTRIBUTE 0x10 // means the entry is a subdirectory
#define SHORT_NAME_LENGTH 8

#endif