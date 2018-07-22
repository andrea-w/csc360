#ifndef _DISKIMAGE_H_
#define _DISKIMAGE_H_ 

typedef struct diskimage diskimage_t;
struct diskimage {
                  char*   imagename;
    superblock_entry_t*   superblock;
           unsigned int   num_free_blocks;
           unsigned int   num_resv_blocks;
           unsigned int   num_alloc_blocks;
} __attribute__ ((packed));

#endif