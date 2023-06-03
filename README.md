# fat-thirty-tool
A tool for conducting basic forensic analysis on FAT32 partitions within hard disk image files (.img)

## How To Use
### Compile
```
make
```
This command will create a `fat32_tool` executable file.
### Retrieve MBR general information
```
./fat32_tool <disk_image.img>
```
Parse through the Master Boot Record and retrieve the following information about partitions:
- **Partition size**
- **Start sector**
- **End sector**
- **Partition type**

### Retrieve FAT32 partition general information
```
./fat32_tool <disk_image.img> <partition_number>
```
Parse through a FAT32 formatting and extract the following information:

- The **number of sectors per cluster**
- The **size in bytes of a sector**
- The **number of FAT data structures** on the volume
- The **number of reserved sectors** in the Reserved region of the volume
- The **cluster number** marking the start of the root directory
- The **complete file/directory tree**.

### Retrieve given file content from FAT32 partition
```
./fat32_tool <disk_image.img> <partition_number> <absolute_path>
```
Parse through the FAT32 formatting and print the content of a file on the standard output.

## Additional Resources
Two [images](/images/) are provided for testing purposes.

## Authors
- Loïc Meunier
- Jordi Hoorelbeke
