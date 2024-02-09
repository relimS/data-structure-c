# My implementation based on C++ absl:flat_hash_map. Use that instead.

## Notes:
- There is no
    + dynamic resizing.
    + accounting for load factor.
- Flat hash map relies heavily on SIMD.
## extern int fhm_init(Flat_hash_map \*map, size_t n, size_t key_size, size_t val_size);

Initialize the map with an initial capacity of n (rounded up to a multiple of 16).

## extern ssize_t fhm_add(Flat_hash_map \*map, const void \*key, size_t key_size, const void \*val, size_t val_size, uint64_t hash);

Insert/Add an item to the map.

Returns -1 if the item cannot be added. (not enough capacity)
Returns the index of the item in the map.

## extern void \*fhm_get_key(Flat_hash_map \*map, size_t index, size_t key_size, size_t val_size);

Return a void pointer to the item's key in the map.

## extern void \*fhm_get_val(Flat_hash_map \*map, size_t index, size_t key_size, size_t val_size);

Return a void pointer to the item's value in the map.

## extern ssize_t fhm_find(Flat_hash_map \*map, const void \*key, size_t key_size, size_t val_size, uint64_t hash, int (\*compar)(const void \*, const void \*));

Return -1 if the item is not in the map.
Return the index of the item.

## extern void \*fhm_lookup(Flat_hash_map \*map, const void \*key, size_t key_size, size_t val_size, uint64_t hash, int (\*compar)(const void \*, const void \*));

Just a wrapper for get_val of find.

Return a null pointer of the item is not in the map.
Return a pointer to the item's value.

## extern int fhm_resize(Flat_hash_map \*map, size_t n, size_t key_size, size_t val_size, size_t (\*hash)(const void\*));

Resize to n capacity (rounded up to a multiple of 16).

## extern void fhm_erase(Flat_hash_map \*map, size_t index);

Erasing an item from the map.

## extern void fhm_free(Flat_hash_map \*map);

Free the map.
