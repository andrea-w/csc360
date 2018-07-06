#ifndef _RW_H_
#define _RW_H_

#define MAX_VALUE_LEN 80

void initialize_readers_writer();
void rw_read(char *, int);
void rw_read_rp(char *value, int len);
void rw_write(char *, int);
void rw_write_rp(char *, int);

#endif
