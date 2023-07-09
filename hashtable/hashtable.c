#include "hashtable.h"

extern int ht_init(Hash_table *table, size_t n) {
    size_t i;
    if (n == 0) return -1;
    table->buckets = (Hash_bucket**)malloc(n*sizeof(Hash_bucket*));
    if (table->buckets == NULL) return -1;
    table->n = n;
    for (i = 0; i != n; ++i) {
        table->buckets[i] = NULL;
    }
    return 0;
}

extern Hash_bucket *ht_add(Hash_table *table, const void *key, size_t key_size, const void *value, size_t value_size, size_t hash, int (*compar)(const void *, const void *)) {
    size_t index;
    Hash_bucket *bucket;
    index = hash % table->n;
    if (!(table->buckets[index])) {
        table->buckets[index] = (Hash_bucket*)malloc(sizeof(Hash_bucket));
        table->buckets[index]->next_bucket = NULL;
        table->buckets[index]->key = NULL;
    }
    bucket = table->buckets[index];
    while (bucket->next_bucket) {
        bucket = bucket->next_bucket;
        if (!compar(key, bucket->key)) return NULL;
    }
    if (bucket->key) {
        bucket->next_bucket = (Hash_bucket*)malloc(sizeof(Hash_bucket));
        bucket = bucket->next_bucket;
    }
    bucket->key = malloc(key_size);
    if (!bucket->key) return NULL;
    bucket->value = malloc(value_size);
    if (!bucket->value) {
        free(bucket->key);
        return NULL;
    }
    memcpy(bucket->key, key, key_size);
    memcpy(bucket->value, value, value_size);
    bucket->next_bucket = NULL;
    return bucket;
}

extern Hash_bucket *ht_add_fast(Hash_table *table, const void *key, size_t key_size, const void *value, size_t value_size, size_t hash) {
    size_t index;
    Hash_bucket *bucket;

    bucket = (Hash_bucket*)malloc(sizeof(Hash_bucket));
    if (bucket == NULL) return NULL;
    index = hash % table->n;
    bucket->next_bucket = table->buckets[index];
    bucket->key = malloc(key_size);
    if (!bucket->key) return NULL;
    bucket->value = malloc(value_size);
    if (!bucket->value) {
        free(bucket->key);
        return NULL;
    }
    memcpy(bucket->key, key, key_size);
    memcpy(bucket->value, value, value_size);
    table->buckets[index] = bucket;
    return bucket;
}

extern void __ht_bucket_free_list(Hash_bucket *bucket) {
    Hash_bucket *bucket2;
    if (!bucket) return;
    while (bucket->next_bucket) {
        free(bucket->key);
        free(bucket->value);
        bucket2 = bucket->next_bucket;
        free(bucket);
        bucket = bucket2;
    }
    if (bucket->key) {
        free(bucket->key);
        free(bucket->value);
    }
    free(bucket);
}

extern Hash_bucket *ht_lookup(Hash_table *table, const void *key, size_t hash, int (*compar)(const void *, const void *)) {
    Hash_bucket *bucket;
    size_t index;
    index = hash % table->n;
    bucket = table->buckets[index];
    if (!bucket) return NULL;
    if (bucket->key) {
        if (!compar(key, bucket->key)) return bucket;
        while (bucket->next_bucket != NULL) {
            bucket = bucket->next_bucket;
            if (!compar(key, bucket->key)) return bucket;
        }
    }
    return NULL;
}

extern Hash_bucket_location ht_lookup_location(Hash_table *table, const void *key, size_t hash, int (*compar)(const void *, const void *)) {
    Hash_bucket *bucket;
    Hash_bucket_location lot;
    lot.index = hash % table->n;
    lot.depth = 1;
    bucket = table->buckets[lot.index];
    if (bucket) {
        if (bucket->key) {
            if (!compar(key, bucket->key)) {
                return lot;
            }
            while (bucket->next_bucket) {
                bucket = bucket->next_bucket;
                ++lot.depth;
                if (!compar(key, bucket->key)) return lot;
            }
        }
    }
    lot.depth = 0;
    return lot;
}

extern Hash_bucket *ht_get_bucket(Hash_table *table, size_t index, size_t depth) {
    Hash_bucket *bucket;
    size_t i;
    if (depth == 0) return NULL;
    bucket = table->buckets[index];
    if (!bucket) return NULL;
    for (i = 1; i != depth; ++i) {
        if (!bucket->next_bucket) return NULL;
    }
    return bucket;
}

extern int ht_bucket_remove(Hash_table *table, size_t index, size_t depth) {
    Hash_bucket *bucket, *tmp;
    if (depth == 0) return -1;
    if (depth == 1) {
        free(table->buckets[index]->key);
        free(table->buckets[index]->value);
        tmp = table->buckets[index]->next_bucket;
        free(table->buckets[index]);
        table->buckets[index] = tmp;
        return 0;
    }
    bucket = ht_get_bucket(table, index, depth);
    if (!bucket->next_bucket) return -1;
    free(bucket->next_bucket->key);
    free(bucket->next_bucket->value);
    if (!bucket->next_bucket->next_bucket) {
        free(bucket->next_bucket);
        bucket->next_bucket = NULL;
    } else {
        tmp = bucket->next_bucket->next_bucket;
        free(bucket->next_bucket);
        bucket->next_bucket = tmp;
    }
    return 0;
}

extern void ht_free(Hash_table *table) {
    size_t i;
    for (i = 0; i != table->n; ++i) __ht_bucket_free_list(table->buckets[i]);
    free(table->buckets);
}
