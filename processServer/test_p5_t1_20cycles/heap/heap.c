#include "heap.h"
#include "panic.h"
#include "arena.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdio.h>

       #include <sys/syscall.h>      /* Definition of SYS_* constants */
       #include <unistd.h>

#define arenaMaxCount 40
#define arenaMaxSize 1048576
#define maxPointer 131072
#define SH sharedHeap
#define lastAddress 131070

// per process counters
uint64_t mCount = 0;
uint64_t fCount = 0;
uint64_t rCount = 0;


long size64(size_t i){
	long size=i/8;
    if(i%8!=0)size++;
	return size;
}






typedef struct SharedHeap {
long* arenas[40];
atomic_char* lock[40];
char* arenaCounter;
}SharedHeap;

SharedHeap sharedHeap;

char findArena(char *addr){
	int arena = 0;
	
	for(;arena<arenaMaxCount;arena++){
		char *startPointer = (char*)(SH.arenas[arena]);
		char *endPointer = startPointer+arenaMaxSize;
		if(addr<endPointer && addr>=startPointer)break;

	}
	return (char)arena;
}

long* getNextPointer(long *ptr){
	long *newPointer;
if(*ptr==0)newPointer=ptr+ptr[1];
else newPointer = ptr+ptr[0]+1;
return newPointer;
}

void joinFreeSpace(long *ptr,int arena){
	long *base = ptr;
	long *newPtr = ptr;
	long freeCount = 0;
	long consumed = 0;                             
	while(newPtr[0]<0){
		consumed++;
		freeCount += ~newPtr[0];
		if(consumed>1)freeCount++;
		if((long*)(SH.arenas[arena])+lastAddress<=newPtr+(~newPtr[0]))break;
		newPtr+=(~newPtr[0])+1;

	}
	base[0]=~freeCount;
}

long* findFreeSpace(uint8_t arena,long sizetemp){

	long *ptr = (long*)SH.arenas[arena];
	long freeSpace=0;

	while(1){
		long size = sizetemp;
		if((long*)(SH.arenas[arena])+lastAddress<ptr)return 0;
		if((long*)(SH.arenas[arena])+lastAddress<ptr+(~ptr[0]))return 0;
		if(ptr[0]<0){
			if((uint64_t)ptr%16==0)size++;
			joinFreeSpace(ptr,arena);
			freeSpace=~ptr[0];
			if(freeSpace>=size)break;
		}
		if(ptr[0]<0)ptr+=~ptr[0]+1;
		else ptr+=ptr[0]+1;
	}
	return ptr;
}

__attribute((constructor))

void SharedHeap_init(void) {
	for(int i=0; i<arenaMaxCount;i++){
		SH.arenas[i]=((long*)create_arena())+1;
		long *firstHeader = (long*)SH.arenas[i];
		firstHeader[0] = ~(lastAddress);
		SH.lock[i] = (atomic_char*)(firstHeader-1);
		SH.lock[i][0] = 0;	
	}
}





void* SharedHeap_malloc(size_t size) {
	long size8B = size64(size);
	uint8_t arena = 0;
	long *ptr=0;
	int counter = getpid();
	int loopCounter=0;

	for(;arena<arenaMaxCount*3;arena++){
		if(SH.lock[(counter+arena)%40][0]==1)continue;
		SH.lock[(counter+arena)%40][0]=1;
		ptr = findFreeSpace((counter+arena)%40,size8B);
		if(ptr!=0)break;
		SH.lock[(counter+arena)%40][0]=0;
		loopCounter++;
	}
	if(ptr==0)return NULL;
	//SH.arenaCounter[0]++;
	//if(SH.arenaCounter[0]>=100)SH.arenaCounter[0]=0;
	if((uint64_t)ptr%16==0)size8B++;
	if(~ptr[0]!=size8B)ptr[size8B+1]=~(~ptr[0]-(size8B+1));
	ptr[0]=size8B;
	mCount++;
	//printf("malloc: %p of size: %d\n ",ptr+1,size8B);
	SH.lock[(counter+arena)%40][0]=0;
	if((uint64_t)(&ptr[1])%16==0)return &ptr[1];
	ptr[1]=0;
	return &ptr[2];
}

void SharedHeap_free(void* Preptr) {
	if(Preptr==0)return;
	char arena = findArena(Preptr);
	while (SH.lock[(int)arena][0]==1);
	SH.lock[(int)arena][0]=1;
	long* ptr =(long*)Preptr;
	if(ptr[-1]==0)ptr--;
	//long size = ((long*)ptr)[-1];
	
	((long*)ptr)[-1] = ~((long*)ptr)[-1];
	SH.lock[(int)arena][0]=0;
	//printf("free: %p of size: %d\n",ptr,size);
	fCount++;
}

void* SharedHeap_realloc(void* Preptr, size_t size) {
	char arena = findArena(Preptr);
	long* ptr =(long*)Preptr;
	long* starter =ptr;

	if(ptr[-1]==0)ptr--;
	while (SH.lock[(int)arena][0]==1);
	
	SH.lock[(int)arena][0]=1;
	uint64_t oldSize = ptr[-1];

	
	long *newPtr = (long*)SharedHeap_malloc(size);
	mCount--;
	for(int i=0;i<oldSize&&i<size;i++){
		newPtr[i]=starter[i];
	}

	((long*)ptr)[-1] = ~((long*)ptr)[-1];
	SH.lock[(int)arena][0]=0;
	//printf("realloc");
	rCount += 1;

	return newPtr;
}

