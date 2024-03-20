#ifndef HASHTBL_H
#define HASHTBL_H

#include <stdbool.h>
#include <stddef.h>

#define ERROR_PTR (void*)~0ul
#define Hashtbl(T1, T2) struct hashtbl*
#define HASHTBL_NEW(SIZE_KEY, SIZE_VALUE, ...) \
do { \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Woverride-init\"") \
        hashtbl_new((SIZE_KEY), (SIZE_VALUE), &(struct hashtbl_creation_args){ .capacity = 100, .hash_func = hashtbl_default_hashfunc, .cmp_func = hashtbl_default_cmpfunc, __VA_ARGS__ }); \
    _Pragma("GCC diagnostic pop") \
} while(0)

struct hashtbl;

struct hashtbl_element {
    void* key;
    void* value;
};

struct hashtbl_creation_args {
    size_t capacity;
    size_t (*hash_func)(const void* key);
    size_t (*cmp_func)(const void* key1, const void* key2);
};

size_t hashtbl_default_cmpfunc(const void* key1, const void* key2);
size_t hashtbl_default_hashfunc(const void* key);
struct hashtbl* hashtbl_new(size_t sizeof_key, size_t sizeof_value, struct hashtbl_creation_args* extra_args);
bool hashtbl_add(struct hashtbl* h, void* key, void* value);

#endif // HASHTBL_H
