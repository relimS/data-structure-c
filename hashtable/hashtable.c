#include "hashtable.h"
#include <stdlib.h>

extern size_t fnv1a (void *data, size_t bytes) {
    size_t i, hash;
    hash = FNV_offset_basis;
    for (i = 0; i != bytes; ++i) {
        hash ^= *((char*)data+i);
        hash *= FNV_prime;
    }
    return hash;
}

extern int ht_init(hash_table *table, size_t n) {
    size_t i;
    if (n == 0) return -1;
    table->buckets = (hash_bucket**)malloc(n*sizeof(hash_bucket*));
    if (table->buckets == NULL) return -1;
    table->nmemb = n;
    for (i = 0; i != n; ++i) {
        table->buckets[i] = NULL;
    }
    return 0;
}

extern hash_bucket *ht_add(hash_table *table, void *key, size_t key_size, void *value, size_t value_size, size_t (*hash)(void *, size_t)) {
    size_t index;
    hash_bucket *bucket;
    index = hash(key, key_size) % table->nmemb;
    if (!(table->buckets[index])) {
        table->buckets[index] = malloc(sizeof(hash_bucket));
        table->buckets[index]->next_bucket = NULL;
        table->buckets[index]->key = NULL;
    }
    bucket = table->buckets[index];
    while (bucket->next_bucket) {
        bucket = bucket->next_bucket;
        if (!strncmp((char*)key, (char*)(bucket->key), key_size)) return NULL;
    }
    if (bucket->key) {
        bucket->next_bucket = (hash_bucket*)malloc(sizeof(hash_bucket));
        bucket = bucket->next_bucket;
    }
    if (!(bucket->key = malloc(key_size+sizeof(char)))|| !(bucket->value = malloc(value_size+sizeof(char)))) return NULL;
    memcpy(bucket->key, key, key_size);
    memcpy(bucket->value, value, value_size);
    memset((char*)(bucket->key)+key_size, '\0', sizeof(char));
    memset((char*)(bucket->value)+value_size, '\0', sizeof(char));
    bucket->next_bucket = NULL;
    return bucket;
}

extern void __ht_bucket_free_recursive(hash_bucket *bucket) {
    // hash_bucket *bucket2;
    // while (bucket->next_bucket) {
    //     free(bucket->key);
    //     free(bucket->value);
    //     bucket2 = bucket->next_bucket;
    //     free(bucket);
    //     bucket = bucket2;
    // }
    if (!bucket) return;
    if (bucket->next_bucket) __ht_bucket_free_recursive(bucket->next_bucket);
    if (bucket->key) {
        free(bucket->key);
        free(bucket->value);
    }
    free(bucket);
}

extern hash_bucket *ht_lookup(hash_table *table, void *key, size_t key_size, size_t (*hash)(void *, size_t)) {
    hash_bucket *bucket;
    size_t index;
    index = hash(key, key_size) % table->nmemb;
    bucket = table->buckets[index];
    if (!bucket) return NULL;
    if (bucket->key) {
        if (!strncmp((char*)(bucket->key), (char*)key, key_size)) return bucket;
        while (bucket->next_bucket != NULL) {
            bucket = bucket->next_bucket;
            if (!strncmp((char*)(bucket->key), (char*)key, key_size)) return bucket;
        }
    }
    return NULL;
}

extern hash_bucket_location ht_lookup_location(hash_table *table, void *key, size_t key_size, size_t (*hash)(void *, size_t)) {
    hash_bucket *bucket;
    hash_bucket_location lot;
    lot.index = hash(key, key_size) % table->nmemb;
    lot.depth = 1;
    bucket = table->buckets[lot.index];
    if (bucket) {
        if (bucket->key) {
            if (!strncmp((char*)(bucket->key), (char*)key, key_size)) {
                return lot;
            }
            while (bucket->next_bucket) {
                bucket = bucket->next_bucket;
                ++lot.depth;
                if (!strncmp((char*)(bucket->key), (char*)key, key_size)) return lot;
            }
        }
    }
    lot.depth = 0;
    return lot;
}

extern hash_bucket *ht_get_bucket(hash_table *table, size_t index, size_t depth) {
    hash_bucket *bucket;
    size_t i;
    if (depth == 0) return NULL;
    bucket = table->buckets[index];
    if (!bucket) return NULL;
    for (i = 1; i != depth; ++i) {
        if (!bucket->next_bucket) return NULL;
    }
    return bucket;
}

extern int ht_bucket_remove(hash_table *table, size_t index, size_t depth) {
    // works by freeing the bucket and replacing the pointer to it with the next on chain 
    hash_bucket *bucket, *tmp;
    if (depth == 0) return -1;
    if (depth == 1) {
        free(table->buckets[index]->key);
        free(table->buckets[index]->value);
        tmp = table->buckets[index]->next_bucket;
        free(table->buckets[index]);
        // replace pointer from the table to next bucket
        table->buckets[index] = tmp;
        return 0;
    }
    bucket = ht_get_bucket(table, index, depth);
    if (!bucket->next_bucket) return -1;
    free(bucket->next_bucket->key);
    free(bucket->next_bucket->value);
    if (!bucket->next_bucket->next_bucket) {
        // already at end of chain
        free(bucket->next_bucket);
        bucket->next_bucket = NULL;
    } else {
        // point next_bucket to the next in the chain
        tmp = bucket->next_bucket->next_bucket;
        free(bucket->next_bucket);
        bucket->next_bucket = tmp;
    }
    return 0;
}

extern void ht_free(hash_table *table) {
    size_t i;
    for (i = 0; i != table->nmemb; ++i) __ht_bucket_free_recursive(table->buckets[i]);
    free(table->buckets);
}
