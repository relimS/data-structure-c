#include <stddef.h>
#include <stdlib.h>
#include <string.h>
typedef struct  Hash_bucket {
    void *key;
    void *value;
    struct Hash_bucket *next_bucket;
} Hash_bucket;

typedef struct Hash_bucket_location {
    size_t index;
    size_t depth; // if (depth = 0) it failed
} Hash_bucket_location;

typedef struct Hash_table {
    Hash_bucket **buckets;
    size_t n;
} Hash_table;

extern int ht_init(Hash_table *table, size_t n);
extern Hash_bucket *ht_add(Hash_table *table, const void *key, size_t key_size, const void *value, size_t value_size, size_t, int (*)(const void *, const void *));
extern void __ht_bucket_free_recursive(Hash_bucket *table);
extern Hash_bucket *ht_lookup(Hash_table *table, const void *key, size_t size, int (*)(const void *, const void *));
extern Hash_bucket_location ht_lookup_location(Hash_table *table, const void *key, size_t hash, int (*)(const void *, const void *));
extern Hash_bucket *ht_get_bucket(Hash_table *table, size_t index, size_t depth);
extern int ht_bucket_erase(Hash_table *table, size_t index, size_t depth);
extern void ht_free(Hash_table *table);