#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "flat_hash_set.h"


#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define N 116322030
#define TABLE_N 134217728

// #define N 1024
// #define TABLE_N 2048

#define N_STR STRINGIZE(N)
#define TABLE_N_STR STRINGIZE(TABLE_N)

#define FNV_offset_basis 14695981039346656037U
#define FNV_prime 1099511628211U


uint64_t fnv1a(const void *data) {
    uint64_t i, Hash;
    Hash = FNV_offset_basis;
    for (i = 0; i != sizeof(size_t); ++i) {
        Hash ^= *((char*)data+i);
        Hash *= FNV_prime;
    }
    return Hash;
}

int compar(const void *a, const void *b) {
    if (*(size_t*)a == *(size_t*)b) return 0;
    if (*(size_t*)a < *(size_t*)b) return -1;
    return 1;
}
#ifdef __SSE2__
    #define add(set, data) fhs_add_sse(set, data, sizeof(size_t), fnv1a(data))
    #define find(set, data) fhs_find_sse(set, data, sizeof(size_t), fnv1a(data), compar)
    #define resize(set, n) fhs_resize_sse(set, n, sizeof(size_t), fnv1a)
#else
    #define add(set, data) fhs_add(set, data, sizeof(size_t), fnv1a(data))
    #define find(set, data) fhs_find(set, data, sizeof(size_t), fnv1a(data), compar)
    #define resize(set, n) fhs_resize(set, n, sizeof(size_t), fnv1a)
#endif

#define init(set, n) fhs_init(set, n, sizeof(size_t))
#define get(set, index) fhs_get(set, index, sizeof(size_t))

int main(void) {
    size_t i, j, *keys;
    ssize_t ret;
    int ret2;
    bool ranoutofidea;
    clock_t start, end;
    Flat_hash_set hashset;
    keys = (size_t*)malloc(N*sizeof(size_t));
    if (!keys) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    fputs("filling "N_STR" array up with random data... ", stdout);
    fflush(stdout);
    start = clock();
    srand(time(NULL));
    for (i = 0; i != N*sizeof(size_t)/sizeof(int); ++i) {
        ((int*)keys)[i] = rand();
    }
    qsort(keys, N, sizeof(*keys), compar);
    ranoutofidea = true;
    while (ranoutofidea) {
        ranoutofidea = false;
        for (i = 0; i != N-1; ++i) {
            if (keys[i] != keys[i+1]) {
                continue;
            }
            ranoutofidea = true;
            for (j = 0; j != sizeof(size_t)/sizeof(int); ++j) ((int*)keys)[j] = rand();
            for (++i; i != N-1; ++i) {
                if (keys[i] == keys[i+1]) for (j = 0; j != sizeof(size_t)/sizeof(int); ++j) ((int*)keys)[j] = rand();
            }
            qsort(keys, N, sizeof(*keys), compar);
            break;
        }
    }
    end = clock();
    printf("done. took %fs.\ninitializing the flat hash set of "TABLE_N_STR"... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    init(&hashset, TABLE_N);
    fputs("done.\nfilling the flat hash set... ", stdout);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = add(&hashset, keys+i);
        if (ret == -1) {
            fprintf(stderr, "Cannot add key to flat hash set ret %zi\n", ret);
            return 1;
        }
        if (compar(get(&hashset, ret), keys+i)) {
            fprintf(stderr, "error 1");
            return 1;
        }
    }
    end = clock();
    printf("done. took %fs.\nreading data from the flat hash set... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = find(&hashset, keys+i);
        if (ret == -1) {
            fprintf(stderr, "error\n");
            return 1;
        }
        if (compar(get(&hashset, ret), keys+i)) {
            fprintf(stderr, "idk what to put\n");
            return 1;
        }
    }
    end = clock();
    printf("done. took %fs.\nresizing flat hash set... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    ret2 = resize(&hashset, 2*TABLE_N);
    if (ret2 < 0) {
        fprintf(stderr, "failed\n");
        return -1;
    }
    end = clock();
    printf("done. took %fs.\nreading data from the flat hash set again... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = find(&hashset, keys+i);
        if (ret == -1) {
            fprintf(stderr, "error\n");
            return 1;
        }
        if (compar(get(&hashset, ret), keys+i)) {
            fprintf(stderr, "idk what to put\n");
            return 1;
        }
    }
    end = clock();
    printf("done. %fs.\nerasing a key... too lazy to implement test\nfreeing the flat hash set... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    free(keys);
    start = clock();
    fhs_free(&hashset);
    end = clock();
    printf("done. took %fs.\n", (float)(end-start)/CLOCKS_PER_SEC);
    return 0;
}
