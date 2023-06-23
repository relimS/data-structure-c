#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/random.h>
#include "hashtable.h"

bool is_unique(uint16_t *ret, size_t *arr, uint16_t n) {
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

int main(void) {
    hash_table table;
    hash_bucket *bucket, **buckets;
    uint16_t n, i, ret;
    size_t *keys, *vals;
    hash_bucket_location lot;
    printf("filling up with random data... ");
    fflush(stdout);
    n = 0;
    while (n == 0) getrandom(&n, sizeof(n), 0);
    keys = (size_t*)malloc(n*sizeof(*keys));
    vals = (size_t*)malloc(n*sizeof(*vals));
    buckets = (hash_bucket**)malloc(n*sizeof(hash_bucket*));
    if (keys == NULL || vals == NULL) {
        fprintf(stderr, "Cannot allocate memory!\n");
        return 1;
    }
    getrandom(vals, n*sizeof(size_t), 0);
    getrandom(keys, n*sizeof(size_t), 0);
    while (!is_unique(&ret, keys, n)) {
        getrandom(keys+ret, sizeof(*keys), 0);
    }
    printf("done\ninitializing the hash table... ");
    fflush(stdout);
    ht_init(&table, n*1000);
    printf("done\nadding data to the hash table... ");
    fflush(stdout);
    for (i = 0; i != n; ++i) {
        bucket = ht_add(&table, keys+i, sizeof(size_t), vals+i, sizeof(size_t), fnv1a);
        if (!bucket || strncmp((char*)(bucket->value), (char*)(vals+i), sizeof(size_t)) || strncmp((char*)(bucket->key), (char*)(keys+i), sizeof(size_t))) {
            printf("failed\n");
            break;
        }
        buckets[i] = bucket;
    }
    printf("done\nreading data from the hash table ... ");
    fflush(stdout);
    for (i = 0; i != n; ++i) {
        bucket = ht_lookup(&table, keys+i, sizeof(size_t), fnv1a);
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
    while (ret %= n == 0) getrandom(&ret, sizeof(ret), 0);
    lot = ht_lookup_location(&table, keys+ret, sizeof(*keys), fnv1a);
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