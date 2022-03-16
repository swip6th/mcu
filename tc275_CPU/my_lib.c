/*
 * my_lib.c
 */

#include "my_lib.h"

void mem_copy( char *dst, char *src, unsigned int length)
{
    for( int i=0; i<length; i++)
        *dst++ = *src++;
}


void mem_copy_64x( char *dst, char *src, unsigned int length)
{
    int *idst = (int *)dst;
    int *isrc = (int *)src;

    for( int i=0; i<length/64; i++)
    {
        *(idst+ 0) = *(isrc+ 0);
        *(idst+ 1) = *(isrc+ 1);
        *(idst+ 2) = *(isrc+ 2);
        *(idst+ 3) = *(isrc+ 3);
        *(idst+ 4) = *(isrc+ 4);
        *(idst+ 5) = *(isrc+ 5);
        *(idst+ 6) = *(isrc+ 6);
        *(idst+ 7) = *(isrc+ 7);
        *(idst+ 8) = *(isrc+ 8);
        *(idst+ 9) = *(isrc+ 9);
        *(idst+10) = *(isrc+10);
        *(idst+11) = *(isrc+11);
        *(idst+12) = *(isrc+12);
        *(idst+13) = *(isrc+13);
        *(idst+14) = *(isrc+14);
        *(idst+15) = *(isrc+15);

        idst += 16;
        isrc += 16;
    }
}


