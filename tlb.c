#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tlb.h"

#define SETS 16
#define WAYS 4

typedef struct {
	int valid_bit;
	int lru;
	size_t tag;
	size_t ppn;
} TLBEntry;

TLBEntry tlb[SETS][WAYS];

void tlb_clear() {
	for (int r = 0; r < SETS; r++) {
		for (int c = 0; c < WAYS; c++) {
			tlb[r][c].valid_bit = 0;
		}
	}
}

int tlb_peek(size_t va) {
	size_t vpn = va >> POBITS;
	size_t index = vpn & 0xF;
	size_t tag_to_check = vpn >> 4;

	for (int i = 0; i < WAYS; i++) {
		if (tlb[index][i].valid_bit && tlb[index][i].tag == tag_to_check) {
			return tlb[index][i].lru;
		}
	}

	return 0;
}


size_t tlb_translate(size_t va) {
	size_t vpn = va >> POBITS;
	size_t mask = ~(~0UL << POBITS);
	size_t page_offset = va & mask;

	size_t row = vpn & 0xF;
	size_t tag_to_check = vpn >> 4;

	int old_lru = 0;
	int col = -1;

	for (int i = 0; i < WAYS; i++) {
		if (tlb[row][i].valid_bit && tlb[row][i].tag == tag_to_check) {
			old_lru = tlb[row][i].lru;
			tlb[row][i].lru = 1;
			col = i;
			break;
		}
	}
	// if not found, we translate and store in tlb
	if (col == -1) {
		ssize_t pa = translate(vpn << POBITS);

		if (pa == -1) return -1;

		size_t ppn = pa >> POBITS;

		for (int c = 0; c < WAYS; c++) {
			// if something is invalid or last used and valid we update tlb
			if (tlb[row][c].valid_bit == 0 || (tlb[row][c].valid_bit == 1 && tlb[row][c].lru == 4)) {
				tlb[row][c].valid_bit = 1;
				tlb[row][c].tag = tag_to_check;
				tlb[row][c].lru = 1;
				tlb[row][c].ppn = ppn;
				col = c;
				break;
			}
		}

		// update other lru bits
		for (int c = 0; c < WAYS; c++) {
			if (c != col && tlb[row][c].valid_bit == 1) {
				tlb[row][c].lru += 1;
			}
		}

	} else {
		for (int i = 0; i < WAYS; i++) {
			if (i != col && tlb[row][i].valid_bit == 1 && tlb[row][i].lru < old_lru) {
				tlb[row][i].lru += 1;
			}
		}
	}
	return (tlb[row][col].ppn << POBITS) + page_offset;
}
