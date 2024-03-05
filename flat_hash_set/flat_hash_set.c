#include "flat_hash_set.h"
#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

extern int fhs_init(Flat_hash_set *set, size_t n, size_t size) {
    n = set->n = (n+15)/16;
    set->data = malloc(n*16*(size+1));
    if (!set->data) return -1;
    memset(set->data, 0xFF, n*16);
    return 0;
}

static inline ssize_t __add__(void *ptr, size_t n, const void *data, size_t size, uint64_t hash) {
    size_t group1, group;
    uint8_t i;
    group1 = (hash>>7) % n;
    for (i = 0; i != 16; ++i) {
        if (((uint8_t*)ptr)[group1*16+i] & (uint8_t)0x80) {
            memset((uint8_t*)ptr+group1*16+i, hash&0x7F, 1);
            memcpy((void*)((uint8_t*)ptr+n*16+(group1*16+i)*size), data, size);
            return group1*16+i;
        }
    }
    for (group = (group1+1) % n; group != group1; group = (group+1) %n) {
        for (i = 0; i != 16; ++i) {
            if (((uint8_t*)ptr)[group*16+i] & (uint8_t)0x80) {
                memset((uint8_t*)ptr+group*16+i, hash&0x7F, 1);
                memcpy((void*)((uint8_t*)ptr+n*16+(group*16+i)*size), data, size);
                return group*16+i;                
            }
        }
    }
    return -1;

}

extern ssize_t fhs_add(Flat_hash_set *set, const void *data, size_t size, uint64_t hash) {
    return __add__(set->data, set->n, data, size, hash);
}

extern void *fhs_get(Flat_hash_set *set, size_t index, size_t size) {
    return (void*)((uint8_t*)set->data+set->n*16+index*size);
}

extern ssize_t fhs_find(Flat_hash_set *set, const void *data, size_t size, uint64_t hash, int (*compar)(const void *, const void *)) {
    size_t group1, group;
    uint8_t i, h2;

    h2 = hash&0x7F;
    group1 = (hash>>7) % set->n;
    for (i = 0; i != 16; ++i) {
        if (((uint8_t*)set->data)[group1*16+i] == h2) {
            if (!compar(fhs_get(set, group1*16+i, size), data)) {
                return group1*16+i;
            }
            continue;
        }
        if (((uint8_t*)set->data)[group1*16+i] == 0xFF) return -1;
    }
    for (group = (group1+1) % set->n; group != group1; group = (group+1) % set->n) {
        for (i = 0; i != 16; ++i) {
            if (((uint8_t*)set->data)[group*16+i] == h2) {
                if (!compar(fhs_get(set, group*16+i, size), data)) {
                    return group*16+i;
                }
                continue;
            }
            if (((uint8_t*)set->data)[group*16+i] == 0xFF) return -1;
        }
    }
    return -1;
}

extern void fhs_delete(Flat_hash_set *set, size_t index) {
    memset((uint8_t*)set->data+index, 0x80, 1);
}

void fhs_free(Flat_hash_set *set) {
    free(set->data);
}

extern int fhs_resize(Flat_hash_set *set, size_t n, size_t size, uint64_t (*hash)(const void*)) {
    size_t i, hash_;
    ssize_t ret;
    uint8_t j;
    void *data, *ptr;

    n = (n+15)/16;
    ptr = malloc(n*16*(size+1));
    if (!ptr) return -1;

    memset(ptr, 0xFF, n*16);

    for (i = 0; i != set->n; ++i) {
        for (j = 0; j != 16; ++j) {
            if (((uint8_t*)set->data)[i*16+j] == 0xFF) break;
            if (((uint8_t*)set->data)[i*16+j] == 0x80) continue;
            data = fhs_get(set, i*16+j, size);
            hash_ = hash(data);
            ret = __add__(ptr, n, data, size, hash_);
            if (ret == -1) {
                free(ptr);
                return -1;
            }
        }
    }
    free(set->data);
    set->data = ptr;
    set->n = n;
    return 0;
}


#ifdef __SSE2__
    extern ssize_t fhs_find_sse(Flat_hash_set *set, const void *data, size_t size, uint64_t hash, int (*compar)(const void *, const void *)) {
        size_t group1, group;
        __m128i bigvect;
        uint8_t h2;
        int mask, t;

        h2 = hash&0x7F;

        group1 = (hash>>7) % set->n;
        bigvect =_mm_set1_epi8(h2);
        mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)set->data+group1*16)));
        while (mask) {
            t = __builtin_ctz(*(unsigned int*)&mask);
            mask ^= 0x01<<t;
            if (!compar(fhs_get(set, group1*16+t, size), data)) return group1*16+t;
        }
        bigvect = _mm_set1_epi8(0xFF);
        mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)set->data+group1*16)));
        if (mask) return -1;

        for (group = (group1+1) % set->n; group != set->n; group = (group+1) % set->n) {
            bigvect =_mm_set1_epi8(h2);
            mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)set->data+group*16)));
            while (mask) {
                t = __builtin_ctz(*(unsigned int*)&mask);
                mask ^= 0x01<<t;
                if (!compar(fhs_get(set, group*16+t, size), data)) return group*16+t;
            }
            bigvect = _mm_set1_epi8(0xFF);
            mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)set->data+group*16)));
            if (mask) return -1;
        }
        return -1;
    }

    static inline ssize_t __add_sse__(void *ptr, size_t n, const void *data, size_t size, uint64_t hash) {
        size_t group1, group;
        int mask, t;

        group1 = (hash>>7) % n;
        mask = _mm_movemask_epi8(*(__m128i*)((uint8_t*)ptr+group1*16));
        if (mask) {
            t = __builtin_ctz(*(unsigned int*)&mask);
            memset((uint8_t*)ptr+group1*16+t, hash&0x7F, 1);
            memcpy((uint8_t*)ptr+n*16+(group1*16+t)*size, data, size);
            return group1*16+t;
        }
        
        for (group = (group1+1) % n; group != n; group = (group+1) % n) {
            mask = _mm_movemask_epi8(*(__m128i*)((uint8_t*)ptr+group*16));
            if (!mask) continue;
            t = __builtin_ctz(*(unsigned int*)&mask);
            memset((uint8_t*)ptr+group*16+t, hash&0x7F, 1);
            memcpy((uint8_t*)ptr+n*16+(group*16+t)*size, data, size);
            return group*16+t;
        }
        return -1;
    }

    extern ssize_t fhs_add_sse(Flat_hash_set *set, const void *data, size_t size, uint64_t hash) {
        return __add_sse__(set->data, set->n, data, size, hash);
    }

    extern void fhs_delete_sse(Flat_hash_set *set, size_t index) {
        __m128i bigvect = _mm_set1_epi8(0xFF);
        int mask;
        mask = _mm_movemask_epi8(_mm_cmpeq_epi8(bigvect, *(__m128i*)((uint8_t*)set->data+(index&~(size_t)0x0F))));
        if (mask) {
            memset((uint8_t*)set->data+index, 0xFF, 1);
            return;
        }
        memset((uint8_t*)set->data+index, 0x80, 1);
    }
    extern int fhs_resize_sse(Flat_hash_set *set, size_t n, size_t size, uint64_t (*hash)(const void*)) {
        size_t i, hash_;
        ssize_t ret;
        int mask, t;
        void *ptr, *data;

        n = (n+15)/16;
        ptr = malloc(n*16*(size+1));
        if (!ptr) return -1;

        memset(ptr, 0xFF, n*16);

        for (i = 0; i != set->n; ++i) {
            mask = 0xFFFF ^ _mm_movemask_epi8(*(__m128i*)((uint8_t*)set->data+i*16));
            while (mask) {
                t = __builtin_ctz(*(unsigned int*)&mask);
                mask ^= 0x01<<t;
                data = fhs_get(set, i*16+t, size);
                hash_ = hash(data);
                ret = __add_sse__(ptr, n, data, size, hash_);
                if (ret == -1) {
                    free(ptr);
                    return -1;
                }
            }
        }
        set->data = ptr;
        set->n = n;
        return 0;
    }
#endif
