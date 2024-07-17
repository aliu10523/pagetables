#define _XOPEN_SOURCE 700 
//#include <stdalign.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "../tlb/mlpt.h"
#include "config.h"



#include <assert.h>

int main() {
    // 0 pages have been allocated
    assert(ptbr == 0);
    printf("main = point 1\n");

    page_allocate(0x456789abcdef);
    // 5 pages have been allocated: 4 page tables and 1 data
    assert(ptbr != 0);
    printf("main = point 2\n");

    page_allocate(0x456789abcd00);
    printf("main = point 3\n");
    // no new pages allocated (still 5)

    int *p1 = (int *)translate(0x456789abcd00);
    printf("main = point 4\n");
    *p1 = 0xaabbccdd;
    short *p2 = (short *)translate(0x456789abcd02);
    printf("%04hx\n", *p2); // prints "aabb\n"

    assert(translate(0x456789ab0000) == 0xFFFFFFFFFFFFFFFF);

    page_allocate(0x456789ab0000);
    // 1 new page allocated (now 6; 4 page table, 2 data)

printf("main: translate = a%lx\n", translate(0x456789ab0000));
    assert(translate(0x456789ab0000) != 0xFFFFFFFFFFFFFFFF);

    page_allocate(0x456780000000);
    // 2 new pages allocated (now 8; 5 page table, 3 data)
}
