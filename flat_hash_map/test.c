#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "flat_hash_map.h"


#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define N 58161015
#define TABLE_N 67108864

// #define N 1024
// #define TABLE_N 2048

#define N_STR STRINGIZE(N)
#define TABLE_N_STR STRINGIZE(TABLE_N)

#define FNV_offmap_basis 14695981039346656037U
#define FNV_prime 1099511628211U


uint64_t fnv1a(const void *data) {
    uint64_t i, Hash;
    Hash = FNV_offmap_basis;
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
    #define add(map, key, val) fhm_add_sse(map, key, sizeof(size_t), val, sizeof(size_t), fnv1a(key))
    #define find(map, key) fhm_find_sse(map, key, sizeof(size_t), sizeof(size_t), fnv1a(key), compar)
    #define resize(map, n) fhm_resize_sse(map, n, sizeof(size_t), sizeof(size_t), fnv1a)
#else
    #define add(map, key, val) fhm_add(map, key, sizeof(size_t), val, sizeof(size_t), fnv1a(key))
    #define find(map, key) fhm_find(map, key, sizeof(size_t), sizeof(size_t), fnv1a(key), compar)
    #define resize(map, n) fhm_resize(map, n, sizeof(size_t), sizeof(size_t), fnv1a)
#endif

#define init(map, n) fhm_init(map, n, sizeof(size_t), sizeof(size_t))
#define get_key(map, index) fhm_get_key(map, index, sizeof(size_t), sizeof(size_t))
#define get_val(map, index) fhm_get_val(map, index, sizeof(size_t), sizeof(size_t))

int main(void) {
    size_t i, j, *keys, *vals;
    ssize_t ret;
    int ret2;
    bool ranoutofidea;
    clock_t start, end;
    Flat_hash_map hashmap;
    keys = malloc(N*sizeof(size_t));
    if (!keys) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    vals = malloc(N*sizeof(size_t));
    if (!vals) {
        fprintf(stderr, "Cannot allocate memory!\n");
        free(keys);
        return 1;
    }
    fputs("filling "N_STR" arrays up with random data... ", stdout);
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
    for (i = 0; i != N*sizeof(size_t)/sizeof(int); ++i) {
        ((int*)vals)[i] = rand();
    }
    qsort(vals, N, sizeof(*vals), compar);
    ranoutofidea = true;
    while (ranoutofidea) {
        ranoutofidea = false;
        for (i = 0; i != N-1; ++i) {
            if (vals[i] != vals[i+1]) {
                continue;
            }
            ranoutofidea = true;
            for (j = 0; j != sizeof(size_t)/sizeof(int); ++j) ((int*)vals)[j] = rand();
            for (++i; i != N-1; ++i) {
                if (vals[i] == vals[i+1]) for (j = 0; j != sizeof(size_t)/sizeof(int); ++j) ((int*)vals)[j] = rand();
            }
            qsort(vals, N, sizeof(*vals), compar);
            break;
        }
    }
    end = clock();
    printf("done. took %fs.\ninitializing the flat hash map of "TABLE_N_STR"... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    init(&hashmap, TABLE_N);
    fputs("done.\nfilling the flat hash map... ", stdout);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = add(&hashmap, keys+i, vals+i);
        if (ret == -1) {
            fprintf(stderr, "Cannot add key to flat hash map ret %zi\n", ret);
            return 1;
        }
        if (compar(get_val(&hashmap, ret), vals+i)) {
            fprintf(stderr, "error 1");
            return 1;
        }
    }
    end = clock();
    printf("done. took %fs.\nreading data from the flat hash map... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = find(&hashmap, keys+i);
        if (ret == -1) {
            fprintf(stderr, "error\n");
            return 1;
        }
        if (compar(get_val(&hashmap, ret), vals+i)) {
            fprintf(stderr, "idk what to put\n");
            return 1;
        }
    }
    end = clock();
    printf("done. took %fs.\nresizing flat hash map... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    ret2 = resize(&hashmap, 2*TABLE_N);
    if (ret2 < 0) {
        fprintf(stderr, "failed\n");
        return -1;
    }
    end = clock();
    printf("done. took %fs.\nreading data from the flat hash map again... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        ret = find(&hashmap, keys+i);
        if (ret == -1) {
            fprintf(stderr, "error\n");
            return 1;
        }
        if (compar(get_val(&hashmap, ret), vals+i)) {
            fprintf(stderr, "idk what to put\n");
            return 1;
        }
    }
    end = clock();
    printf("done. %fs.\nerasing a key... too lazy to implement test\nfreeing the flat hash map... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    free(keys);
    start = clock();
    fhm_free(&hashmap);
    end = clock();
    printf("done. took %fs.\n", (float)(end-start)/CLOCKS_PER_SEC);
    return 0;
}
