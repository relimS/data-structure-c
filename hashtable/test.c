#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/random.h>
#include "hashtable.h"

#define FNV_offset_basis 14695981039346656037U
#define FNV_prime 1099511628211U

#define n 10000

extern size_t fnv1a(const void *data, size_t bytes) {
    size_t i, Hash;
    Hash = FNV_offset_basis;
    for (i = 0; i != bytes; ++i) {
        Hash ^= *((char*)data+i);
        Hash *= FNV_prime;
    }
    return Hash;
}

bool is_unique(uint16_t *ret, size_t *arr) {
    uint16_t i, j;
    for (i = 0; i != n-1; ++i) {
        for (j = i+1; j != n; ++j) {
            if (arr[i] == arr[j]) {
                *ret = i;
                return false;
            }
        }
    }
    return true;
}

int compar(const void *a, const void *b) {
    if (*(size_t*)a == *(size_t*)b) return 0;
    return 1;
}

int main(void) {
    Hash_table table;
    Hash_bucket *bucket, **buckets;
    uint16_t i, ret;
    size_t *keys, *vals;
    Hash_bucket_location lot;
    printf("filling up with random data... ");
    fflush(stdout);
    keys = (size_t*)malloc(n*sizeof(*keys));
    vals = (size_t*)malloc(n*sizeof(*vals));
    buckets = (Hash_bucket**)malloc(n*sizeof(Hash_bucket*));
    if (keys == NULL || vals == NULL) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    getrandom(vals, n*sizeof(size_t), 0);
    getrandom(keys, n*sizeof(size_t), 0);
    while (!is_unique(&ret, keys)) {
        getrandom(keys+ret, sizeof(*keys), 0);
    }
    printf("done\ninitializing the Hash table... ");
    fflush(stdout);
    ht_init(&table, n*1000);
    printf("done\nadding data to the Hash table... ");
    fflush(stdout);
    for (i = 0; i != n; ++i) {
        bucket = ht_add(&table, keys+i, sizeof(size_t), vals+i, sizeof(size_t), fnv1a(keys+i, sizeof(size_t)), compar);
        if (!bucket || compar(keys+i, bucket->key) || compar(vals+i, bucket->value)) {
            printf("failed\n");
            break;
        }
        buckets[i] = bucket;
    }
    printf("done\nreading data from the Hash table ... ");
    fflush(stdout);
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
    printf("done\nremoving bucket ... ");
    ret = 0;
    while ((ret %= n) == 0) getrandom(&ret, sizeof(ret), 0);
    lot = ht_lookup_location(&table, keys+ret, fnv1a(keys+ret, sizeof(size_t)), compar);
    if (!lot.depth) printf("failed\n");
    else {
        if(ht_bucket_remove(&table, lot.index, lot.depth) < 0) printf("failed\n");
    }
    ht_free(&table);
    free(keys);
    free(vals);
    free(buckets);
    printf("done\n");
    return 0;
}