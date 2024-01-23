#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

typedef struct Flat_hash_set {
    void *data;
    size_t n;
} Flat_hash_set;

extern int fhs_init(Flat_hash_set *set, size_t n, size_t size);
extern ssize_t fhs_add(Flat_hash_set *set, const void *data, size_t size, uint64_t hash);
extern ssize_t fhs_add_sse(Flat_hash_set *set, const void *data, size_t size, uint64_t hash);
extern void *fhs_get(Flat_hash_set *set, size_t index, size_t size);
extern ssize_t fhs_find(Flat_hash_set *set, const void *data, size_t size, uint64_t hash, int (*compar)(const void *, const void *));
extern ssize_t fhs_find_sse(Flat_hash_set *set, const void *data, size_t size, uint64_t hash, int (*compar)(const void *, const void *));
extern int fhs_resize(Flat_hash_set *set, size_t n, size_t size, uint64_t (*hash)(const void*));
extern int fhs_resize_sse(Flat_hash_set *set, size_t n, size_t size, uint64_t (*hash)(const void*));
extern void fhs_delete(Flat_hash_set *set, size_t index);
extern void fhs_delete_sse(Flat_hash_set *set, size_t index);
extern void fhs_free(Flat_hash_set *set);
