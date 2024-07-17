#define _XOPEN_SOURCE 700

#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

size_t get_pagesize() {
    size_t page_size = 1UL << POBITS;
    return page_size;

}

size_t get_segmentsize() {
    size_t vpn_segments = POBITS - 3; // each page table entry is 8 bytes
    return vpn_segments;
}


/**
 * Page table base register.
 * Declared here so tester code can look at it; because it is extern
 * you'll need to define it (without extern) in exactly one .c file.
 */
size_t ptbr;

/**
 * Given a virtual address, return the physical address.
 * Return a value consisting of all 1 bits
 * if this virtual address does not have a physical address.
 */
size_t translate(size_t va) {
    if (ptbr == 0) {
        return (~0L);
    }

    size_t vpn = va >> POBITS;
    size_t segment_size = get_segmentsize();
    int segments[LEVELS];

    for (int level = 0; level < LEVELS; level++) {
        size_t segment_mask = (1 << segment_size) - 1;
        segments[level] = vpn & segment_mask;
        vpn >>= segment_size;
    }

    size_t *next_level = (size_t *) ptbr;

    for (int level = LEVELS - 1; level >= 0; level--) {
        size_t *table_p = next_level;
        vpn = segments[level];
        size_t pte = table_p[vpn];

        if ((pte & 0x1) == 0) {
            printf("invalid");
            return (~0L);
        }

        size_t ppn = (pte >> POBITS);
        next_level = (size_t *) (ppn << POBITS);
    }

    size_t mask = ~(~0UL << POBITS);
    size_t page_offset = va & mask;
    size_t pa = (size_t) next_level + page_offset;
    return pa;
}

/**
 * Use posix_memalign to create page tables and other pages sufficient
 * to have a mapping between the given virtual address and some physical address.
 *If there already is such a page, does nothing.
 */
void page_allocate(size_t va) {
    size_t page_size = get_pagesize();
    size_t segment_size = get_segmentsize();

    if (ptbr == 0) {
        int ret = posix_memalign((void **) &ptbr, page_size, page_size);
        if (ret != 0) {
            printf("memalign failed");
            exit(1);
        }
        memset((void *) ptbr, 0, page_size);

    }

    size_t vpn = va >> POBITS;
    int segments[LEVELS];
    for (int level = 0; level < LEVELS; level++) {
        size_t segment_mask = (1 << segment_size) - 1;
        segments[level] = vpn & segment_mask;
        vpn >>= segment_size;
    }

    size_t *next_level = (size_t *) ptbr;

    for (int level = LEVELS - 1; level >= 0; level--) {
        size_t *table_p = next_level;
        vpn = segments[level];
        size_t pte = table_p[vpn];

        if (pte & 0x1) {
            size_t ppn = (pte >> POBITS);
            next_level = (size_t *) (ppn << POBITS);
            continue;
        }

        void *ptr = NULL;
        int ret = posix_memalign((void **) &ptr, page_size, page_size);

        if (ret != 0) {
            printf("memaligned failed");
            exit(1);
        }
        memset((void *) ptr, 0, page_size);

        size_t pa = (size_t) ptr;
        size_t ppn = pa >> POBITS;
        pte = ppn << POBITS;
        pte = pte | 0x1;
        table_p[vpn] = pte;
        next_level = ptr;
    }
}

