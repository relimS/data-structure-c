#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FNV_offset_basis 14695981039346656037U
#define FNV_prime 1099511628211U

typedef struct  hash_bucket {
    void *key;
    void *value; // both are null terminated (because I am lazy)
    struct hash_bucket *next_bucket; // linked list (because I am lazy)
} hash_bucket;

typedef struct hash_bucket_location {
    size_t index;
    size_t depth; // if (depth = 0) it failed
} hash_bucket_location;

typedef struct hash_table {
    hash_bucket **buckets;
    size_t nmemb;
} hash_table;

extern size_t fnv1a(void *data, size_t bytes);
extern int ht_init(hash_table *table, size_t n);
extern hash_bucket *ht_add(hash_table *table, void *key, size_t key_size, void *value, size_t value_size, size_t (*hash)(void *, size_t));
extern void __ht_bucket_free_recursive(hash_bucket *bucket);
extern hash_bucket *ht_lookup(hash_table *table, void *key, size_t key_size, size_t (*hash)(void *, size_t));
extern hash_bucket_location ht_lookup_location(hash_table *table, void *key, size_t key_size, size_t (*hash)(void *, size_t));
extern hash_bucket *ht_get_bucket(hash_table *table, size_t index, size_t depth);
extern void __ht_bucket_free_recursive(hash_bucket *bucket);
extern int ht_bucket_remove(hash_table *table, size_t index, size_t depth);
extern void ht_free(hash_table *table);