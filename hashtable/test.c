#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/random.h>
#include <time.h>
#include "hashtable.h"

#define FNV_offset_basis 14695981039346656037U
#define FNV_prime 1099511628211U

#define n 1000000
#define n_str "1000000"
#define ten_n_str "10000000"

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
    keys = (size_t*)malloc(n*sizeof(size_t));
    vals = (size_t*)malloc(n*sizeof(size_t));
    if (!keys || !vals) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    fputs("filling "n_str" keys and "n_str" values arrays up with random data... ", stdout);
    fflush(stdout);
    start = clock();
    getrandom(vals, n*sizeof(*vals), 0);
    getrandom(keys, n*sizeof(*keys), 0);
    qsort(keys, n, sizeof(*keys), compar);
    ranoutofidea = true;
    while (ranoutofidea) {
        ranoutofidea = false;
        for (i = 0; i != n-1; ++i) {
            if (keys[i] == keys[i+1]) {
                ranoutofidea = true;
                getrandom(keys+i, sizeof(*keys), 0);
                for (++i; i != n-1; ++i) {
                    if (keys[i] == keys[i+1]) getrandom(keys+i, sizeof(*keys), 0);
                }
                qsort(keys, n, sizeof(*keys), compar);
                break;
            }
        }
    }
    end = clock();
    printf("done. took %fs.\ninitializing the hash table of "ten_n_str"... ", (float)(end-start)/CLOCKS_PER_SEC);
    fflush(stdout);
    start = clock();
    ht_init(&table, n*10);
    end = clock();
    fputs("done.\nfilling the hash table with the keys and values... ", stdout);
    fflush(stdout);
    start = clock();
    buckets = (Hash_bucket**)malloc(n*sizeof(Hash_bucket*));
    for (i = 0; i != n; ++i) {
        bucket = ht_add(&table, keys+i, sizeof(size_t), vals+i, sizeof(size_t), fnv1a(keys+i, sizeof(size_t)), compar);
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
    for (i = 0; i != n; ++i) {
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
    j = 0;
    while ((j %= n) == 0) getrandom(&j, sizeof(j), 0);
    lot = ht_lookup_location(&table, keys+j, fnv1a(keys+j, sizeof(size_t)), compar);
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