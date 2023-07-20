#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hashtable.h"

#define FNV_offset_basis 14695981039346656037U
#define FNV_prime 1099511628211U


#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define N 67108864
#define TABLE_N 67108864

#define N_STR STRINGIZE(N)
#define TABLE_N_STR STRINGIZE(TABLE_N)


extern size_t fnv1a(const void *data, size_t bytes) {
    size_t i, Hash;
    Hash = FNV_offset_basis;
    for (i = 0; i != bytes; ++i) {
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

int main(void) {
    Hash_table table;
    Hash_bucket *bucket, **buckets;
    size_t i, j, *keys, *vals;
    bool ranoutofidea;
    Hash_bucket_location lot;
    clock_t start, end;
    keys = (size_t*)malloc(N*sizeof(size_t));
    if (!keys) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    vals = (size_t*)malloc(N*sizeof(size_t));
    if (!vals) {
        free(keys);
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    fputs("filling "N_STR" keys and "N_STR" values arrays up with random data... ", stdout);
    fflush(stdout);
    start = clock();
    srand(time(NULL));
    for (i = 0; i != N*sizeof(size_t)/sizeof(int); ++i) {
        ((int*)keys)[i] = rand();
        ((int*)vals)[i] = rand();
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
    printf("done. took %fs.\ninitializing the hash table of "TABLE_N_STR"... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    ht_init(&table, TABLE_N);
    end = clock();
    fputs("done.\nfilling the hash table with the keys and values... ", stdout);
    fflush(stdout);
    start = clock();
    buckets = (Hash_bucket**)malloc(N*sizeof(Hash_bucket*));
    if(!buckets) {
        free(keys);
        free(vals);
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    for (i = 0; i != N; ++i) {
        bucket = ht_add_fast(&table, keys+i, sizeof(size_t), vals+i, sizeof(size_t), fnv1a(keys+i, sizeof(size_t)));
        if (!bucket || compar(keys+i, bucket->key) || compar(vals+i, bucket->value)) {
            printf("failed\n");
            break;
        }
        buckets[i] = bucket;
    }
    end = clock();
    printf("done. took %fs.\nreading data from the hash table ... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    for (i = 0; i != N; ++i) {
        bucket = ht_lookup(&table, keys+i, fnv1a(keys+i, sizeof(size_t)), compar);
        if (!bucket) {
            fprintf(stderr, "failed\n");
            break;
        }
        if (bucket != buckets[i]) {
            fprintf(stderr, "failed\n");
        }
    }
    end = clock();
    free(buckets);
    printf("done. took %fs.\nremoving a bucket ... ", (float)(end-start)/CLOCKS_PER_SEC);
    i = 0;
    while (!(i%=N)) {
        for (j = 0; j != sizeof(size_t)/sizeof(int); ++j) ((int*)&i)[j] = rand();
    }

    lot = ht_lookup_location(&table, keys+i, fnv1a(keys+i, sizeof(size_t)), compar);
    if (!lot.depth) printf("failed\n");
    else {
        if(ht_bucket_erase(&table, lot.index, lot.depth) < 0) printf("failed\n");
    }
    fputs("done.\nfreeing the hash table... ", stdout);
    fflush(stdout);
    free(keys);
    free(vals);
    start = clock();
    ht_free(&table);
    end = clock();
    printf("done. took %fs.\n", (float)(end-start)/CLOCKS_PER_SEC);
    return 0;
}