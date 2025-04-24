#include "arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

// create an arena of size 1MB, please leave this code alone

static int arenas = 0;

__attribute__((destructor))
static void when_done() {
	printf("*** arenas:%d\n", arenas);
}

void* create_arena(void) {
	arenas += 1;
	void* p = mmap(
			0,
			1 << 20,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_SHARED,
			-1,
			0);

	if (p == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	// waste some space, will use a different algorithm when you submit
	// your code
	mmap(
			0,
			1 << 17,
			PROT_NONE,
			MAP_ANONYMOUS | MAP_SHARED | MAP_NORESERVE, 
			-1,
			0);

	return p;
}

