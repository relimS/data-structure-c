# My implementation based on C++ absl:flat_hash_set. Use that instead.

## Notes:
- There is no
    + dynamic resizing.
    + accounting for load factor.
- Flat hash set relies heavily on SIMD.
## extern int fhs_init(Flat_hash_set \*set, size_t n, size_t size);

Initialize the set with an initial capacity of n (rounded up to a multiple of 16).

## extern ssize_t fhs_add(Flat_hash_set \*set, const void \*data, size_t size, uint64_t hash);

Insert/Add an item to the set.

Returns -1 if the item cannot be added. (not enough capacity)
Returns the index of the item in the set.

## extern void \*fhs_get(Flat_hash_set \*set, size_t index, size_t size);

Return a void pointer to the item in the set.

## extern ssize_t fhs_find(Flat_hash_set \*set, const void \*data, size_t size, uint64_t hash, int (\*compar)(const void \*, const void \*));

Return -1 if the item is not in the set.
Return the index of the item.

## extern int fhs_resize(Flat_hash_set \*set, size_t n, size_t size, uint64_t (\*hash)(const void\*));

Resize to n capacity (rounded up to a multiple of 16).

## extern void fhs_delete(Flat_hash_set \*set, size_t index);

Erasing an item from the set.

## extern void fhs_free(Flat_hash_set \*set);

Free the set.
