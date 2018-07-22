#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include "diskimage.h"

/************ function prototypes ****************/
void convert_to_network(superblock_entry_t* sb);

// convert values in superblock from host byte order to network byte order
// https://linux.die.net/man/3/htonl
void convert_to_network(superblock_entry_t* sb){
    sb->block_size = htons(sb->block_size);
    sb->num_blocks = htonl(sb->num_blocks);
    sb->fat_start = htonl(sb->fat_start);
    sb->fat_blocks = htonl(sb->fat_blocks);
    sb->dir_start = htonl(sb->dir_start);
    sb->dir_blocks = htonl(sb->dir_blocks);
}

int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE  *f;
    int   *fat_data;
    diskimage_t diskimage;
    diskimage.superblock = &sb;
    diskimage.num_free_blocks = 0;
    diskimage.num_resv_blocks = 0;
    diskimage.num_alloc_blocks = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: statuvfs --image <imagename>\n");
        exit(1);
    }
    /****************** end of instructor-provided started code *************************/

    // check that image exists and can be opened
    // https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/fopen.html
    if ( (f = fopen(imagename, "r+")) == NULL) {      // "r+" is permission to open file for update (read and write)
        fprintf(stderr, "Error: could not open file %s\n", imagename);
        exit(EXIT_FAILURE);
    }

    diskimage.imagename = imagename;

    // set pointer to start of file
    // https://www.tutorialspoint.com/c_standard_library/c_function_fseek.htm
    fseek(f, 0, SEEK_SET);

    // read file into superblock
    // https://www.tutorialspoint.com/c_standard_library/c_function_fread.htm
    fread(diskimage.superblock, sizeof(superblock_entry_t), 1, f); 

    // validate format of diskimage
    if (strncmp(diskimage.superblock->magic, FILE_SYSTEM_ID, FILE_SYSTEM_ID_LEN) != 0) {
        fprintf(stderr, "Error: format of image (%s) does not match expected format: %s\n", diskimage.superblock->magic, FILE_SYSTEM_ID);
        exit(EXIT_FAILURE);
    }

    // convert values in superblock from host to network
    convert_to_network(diskimage.superblock);

    // read FAT

    // print stats
    printf("%s (%s)\n", diskimage.superblock->magic, diskimage.imagename);
    printf("\n-----------------------------------------------------------\n");
    printf("  Bsz   Bcnt   FATst   FATcnt   DIRst   DIRcnt\n");
    printf("%5d  %5d  %5d  %5d  %5d  %5d \n", diskimage.superblock->block_size, diskimage.superblock->num_blocks, diskimage.superblock->fat_start, diskimage.superblock->fat_blocks, diskimage.superblock->dir_start, diskimage.superblock->dir_blocks);
    printf("\n-----------------------------------------------------------\n");
    printf("  Free    Resv    Alloc\n");
    printf("\n" );

    // close file
    fclose(f);   

    return 0; 
}
