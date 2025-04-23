#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


// per process counters
extern uint64_t mCount;
extern uint64_t fCount;


typedef struct SharedHeap SharedHeap;
typedef struct Chunk Chunk;

extern SharedHeap sharedHeap;

extern void SharedHeap_init();

extern void* SharedHeap_malloc(size_t size);
extern void SharedHeap_free(void* ptr);
extern void* SharedHeap_realloc(void* ptr, size_t size);


