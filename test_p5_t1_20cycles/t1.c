#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include "heap/heap.h"
#include "heap/panic.h"

#define M 2000
#define N 1000000

void* last[M] = {};

int main() {
    for (uint32_t i=0; i<M; i++) {
        last[i] = 0;
    }

    uint64_t m1 = mCount;
    uint64_t f1 = fCount;

    int child_id = fork();

    for (uint32_t i=0; i<N; i++) {
        uint32_t x = rand() % M;
        if (last[x] != 0) {
            SharedHeap_free(last[x]);
            last[x] = 0;
        }
        size_t sz = (size_t) (rand() % 1000 + 1);
        last[x] = SharedHeap_malloc(sz);
        if (last[x] == 0) {
            panic("*** failed to allocate %d\n",sz);
        }
        char* ptr = (char*) last[x];
        ptr[0] = 66;
        ptr[sz-1] = 77;
    }

    for (uint32_t i=0; i<M; i++) {
        if (last[i] != 0) {
            SharedHeap_free(last[i]);
        }
    }

    uint64_t m2 = mCount - m1;
    uint64_t f2 = fCount - f1;

    char* me = "child";

    if (child_id != 0) {
        /* parent, wait for the child to terminate */
        int status = 0;
        int rc = waitpid(child_id, &status, 0);
        if (rc != child_id) {
            perror("waitpid");
            exit(-1);
        }
        me = "parent";
        printf("[%s] child terminated with status %d\n", me, status);
    }
  
    if (m2 != f2) {
        printf("[%s] m2 %ld\n",me, m2);
        printf("[%s] f2 %ld\n",me, f2);
    } else {
        printf("[%s] count match\n", me);
    }

    if (m2 != N) {
        printf("*** [%s] wrong count %ld\n",me, m2);
    } else {
        printf("[%s] count ok\n", me);
    }

    return 0;
}
