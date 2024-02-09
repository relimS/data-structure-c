#include "flat_hash_map.h"
#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

extern int fhm_init(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size) {
    n = map->n = (n+15)/16;
    map->data = malloc(n*16*(key_size+val_size+1));
    if (!map->data) return -1;
    memset(map->data, 0xFF, n*16);
    return 0;
}

static inline ssize_t __add__(void *ptr, size_t n, const void *key, size_t key_size, const void *val, size_t val_size, uint64_t hash) {
    size_t group;
    uint8_t i;

    for (group = (hash>>7) % n; group != n; ++group) {
        for (i = 0; i != 16; ++i) {
            if (((uint8_t*)ptr)[group*16+i] & 0x80) {
                memset((uint8_t*)ptr+group*16+i, hash&0x7F, 1);
                memcpy((void*)((uint8_t*)ptr+n*16+(group*16+i)*(key_size+val_size)), key, key_size);
                memcpy((void*)((uint8_t*)ptr+n*16+(group*16+i)*(key_size+val_size)+key_size), val, val_size);
                return group*16+i;                
            }
        }
    }
    return -1;

}

extern ssize_t fhm_add(Flat_hash_map *map, const void *key, size_t key_size, const void *val, size_t val_size, uint64_t hash) {
    // size_t group;
    // uint8_t i;

    // for (group = (hash>>7) % map->n; group != map->n; ++group) {
    //     for (i = 0; i != 16; ++i) {
    //         if (((uint8_t*)map->data)[group*16+i] & 0x80) {
    //             memset((uint8_t*)map->data+group*16+i, hash&0x7F, 1);
    //             memcpy(fhm_get_key(map, i, key_size, val_size), key, key_size);
    //             memcpy(fhm_get_val(map, group*16+i, key_size, val_size), val, val_size);
    //             return group*16+i;                
    //         }
    //     }
    // }
    // return map->n*16;
    return __add__(map->data, map->n, key, key_size, val, val_size, hash);
}

extern void *fhm_get_key(Flat_hash_map *map, size_t index, size_t key_size, size_t val_size) {
    return (uint8_t*)map->data+map->n*16+index*(key_size+val_size);
}

extern void *fhm_get_val(Flat_hash_map *map, size_t index, size_t key_size, size_t val_size) {
    return (uint8_t*)map->data+map->n*16+index*(key_size+val_size)+key_size;
}

extern ssize_t fhm_find(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, uint64_t hash, int (*compar)(const void *, const void *)) {
    size_t group;
    uint8_t i, h2;

    h2 = hash&0x7F;
    for (group = (hash>>7) % map->n; group != map->n; ++group) {
        for (i = 0; i != 16; ++i) {
            if (((uint8_t*)map->data)[group*16+i] == h2) {
                if (!compar(fhm_get_key(map, group*16+i, key_size, val_size), key)) {
                    return group*16+i;
                }
                continue;
            }
            if (((uint8_t*)map->data)[group*16+i] == 0xFF) return -1;
        }
    }
    return -1;
}

extern void *fhm_lookup(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, uint64_t hash, int (*compar)(const void *, const void *)) {
    return fhm_get_val(map, fhm_find(map, key, key_size, val_size, hash, compar), key_size, val_size);
}

extern void fhm_erase(Flat_hash_map *map, size_t index) {
    memset((uint8_t*)map->data+index, 0x80, 1);
}

void fhm_free(Flat_hash_map *map) {
    free(map->data);
}

extern int fhm_resize(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size, uint64_t (*hash)(const void*)) {
    size_t i, hash_;
    ssize_t ret;
    uint8_t j;
    void *key, *ptr;

    n = (n+15)/16;
    ptr = malloc(n*16*(key_size+val_size+1));
    if (!ptr) return -1;

    memset(ptr, 0xFF, n*16);

    for (i = 0; i != map->n; ++i) {
        for (j = 0; j != 16; ++j) {
            if (((uint8_t*)map->data)[i*16+j] == 0xFF) break;
            if (((uint8_t*)map->data)[i*16+j] == 0x80) continue;
            key = fhm_get_key(map, i*16+j, key_size, val_size);
            hash_ = hash(key);
            ret = __add__(ptr, n, key, key_size, (void*)((uint8_t*)key+key_size), val_size, hash_);
            if (ret == -1) {
                free(ptr);
                return -1;
            }
        }
    }
    free(map->data);
    map->data = ptr;
    map->n = n;
    return 0;
}

#ifdef __SSE2__
    extern ssize_t fhm_find_sse(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, uint64_t hash, int (*compar)(const void *, const void *)) {
        size_t group;
        __m128i bigvect;
        uint8_t h2;
        int mask, t;

        h2 = hash&0x7F;
        for (group = (hash>>7) % map->n; group != map->n; ++group) {
            bigvect =_mm_set1_epi8(h2);
            mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)map->data+group*16)));
            while (mask) {
                t = __builtin_ctz(*(unsigned int*)&mask);
                mask ^= 0x01<<t;
                if (!compar(fhm_get_key(map, group*16+t, key_size, val_size), key)) return group*16+t;
            }
            bigvect = _mm_set1_epi8(0xFF);
            mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)map->data+group*16)));
            if (mask) return -1;
        }
        return -1;
    }

    extern void *fhm_lookup_sse(Flat_hash_map *map, const void *key, size_t key_size, size_t val_size, uint64_t hash, int (*compar)(const void *, const void *)) {
        return fhm_get_val(map, fhm_find_sse(map, key, key_size, val_size, hash, compar), key_size, val_size);
    }

    static inline ssize_t __add_sse__(void *ptr, size_t n, const void *key, size_t key_size, const void *val, size_t val_size, uint64_t hash) {
        size_t group;
        int mask, t;

        for (group = (hash>>7) % n; group != n; ++group) {
            mask = _mm_movemask_epi8(*(__m128i*)((uint8_t*)ptr+group*16));
            if (!mask) continue;
            t = __builtin_ctz(*(unsigned int*)&mask);
            memset((uint8_t*)ptr+group*16+t, hash&0x7F, 1);
            memcpy((uint8_t*)ptr+n*16+(group*16+t)*(key_size+val_size), key, key_size);
            memcpy((uint8_t*)ptr+n*16+(group*16+t)*(key_size+val_size)+key_size, val, val_size);
            return group*16+t;
        }
        return -1;
    }

    extern ssize_t fhm_add_sse(Flat_hash_map *map, const void *key, size_t key_size, const void *val, size_t val_size, uint64_t hash) {
        // size_t group;
        // int mask, t;

        // for (group = (hash>>7) % map->n; group != map->n; ++group) {
        //     mask = _mm_movemask_epi8(*(__m128i*)((uint8_t*)map->data+group*16));
        //     if (!mask) continue;
        //     t = __builtin_ctz(*(unsigned int*)&mask);
        //     memset((uint8_t*)map->data+group*16+t, hash&0x7F, 1);
        //     memcpy(fhm_get_key(map, group*16+t, key_size, val_size), key, key_size);
        //     memcpy(fhm_get_val(map, group*16+t, key_size, val_size), val, val_size);
        //     return group*16+t;
        // }
        // return -1;
        return __add_sse__(map->data, map->n, key, key_size, val, val_size, hash);
    }

    extern void fhm_erase_sse(Flat_hash_map *map, size_t index) {
        __m128i bigvect = _mm_set1_epi8(0xFF);
        int mask;
        mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)map->data+(index&~(size_t)0x0F))));
        if (mask) {
            memset((uint8_t*)map->data+index, 0xFF, 1);
            return;
        }
        memset((uint8_t*)map->data+index, 0x80, 1);
    }

    extern int fhm_resize_sse(Flat_hash_map *map, size_t n, size_t key_size, size_t val_size, uint64_t (*hash)(const void*)) {
        size_t i, hash_;
        ssize_t ret;
        int mask, t;
        void *ptr, *key;

        n = (n+15)/16;
        ptr = malloc(n*16*(key_size+val_size+1));
        if (!ptr) return -1;

        memset(ptr, 0xFF, n*16);

        for (i = 0; i != map->n; ++i) {
            mask = 0xFFFF ^ _mm_movemask_epi8(*(__m128i*)((uint8_t*)map->data+i*16));
            while (mask) {
                t = __builtin_ctz(*(unsigned int*)&mask);
                mask ^= 0x01<<t;
                key = fhm_get_key(map, i*16+t, key_size, val_size);
                hash_ = hash(key);
                ret = __add_sse__(ptr, n, key, key_size, fhm_get_val(map, i*16+t, key_size, val_size), val_size, hash_);
                if (ret == -1) {
                    free(ptr);
                    return -1;
                }
            }
        }
        map->data = ptr;
        map->n = n;
        return 0;
    }

#endif
