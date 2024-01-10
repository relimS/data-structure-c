#include <stddef.h>
#include <sys/types.h>

typedef struct Flat_hash_map {
    void *data;
    size_t n;
} Flat_hash_map;

extern int fhm_init(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size);
extern ssize_t fhm_add(Flat_hash_map *map, const void *key, size_t key_size, const void *val, size_t val_size, size_t hash);
extern ssize_t fhm_add_sse(Flat_hash_map *map, const void *key, size_t key_size, const void *val, size_t val_size, size_t hash);
extern void *fhm_get_key(Flat_hash_map *map, size_t index, size_t key_size, size_t val_size);
extern void *fhm_get_val(Flat_hash_map *map, size_t index, size_t key_size, size_t val_size);
extern ssize_t fhm_find(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, size_t hash, int (*compar)(const void *, const void *));
extern ssize_t fhm_find_sse(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, size_t hash, int (*compar)(const void *, const void *));
extern void *fhm_lookup(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, size_t hash, int (*compar)(const void *, const void *));
extern void *fhm_lookup_sse(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, size_t hash, int (*compar)(const void *, const void *));
extern int fhm_resize(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size, size_t (*hash)(const void*));
extern int fhm_resize_sse(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size, size_t (*hash)(const void*));
extern void fhm_erase(Flat_hash_map *map, size_t index);
extern void fhm_erase_sse(Flat_hash_map *map, size_t index);
extern void fhm_free(Flat_hash_map *map);
