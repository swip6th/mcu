/*
 * my_lib.h
 */

#define SYSTEM_TIMER_0_31_0 *(unsigned int *)(0xF0000000+0x10)

void mem_copy( char *dst,  char *src, unsigned int length);
void mem_copy_64x( char *dst,  char *src, unsigned int length);


