#ifndef HASHTBL_H
#define HASHTBL_H

#include <stdbool.h>
#include <stddef.h>

#define ERROR_PTR (void*)~0ul
#define Hashtbl(T1, T2) struct hashtbl*
#define HASHTBL_NEW(OUT, TYPE_KEY, TYPE_VALUE, ...) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Woverride-init\"") \
    OUT = hashtbl_new(sizeof(TYPE_KEY), sizeof(TYPE_VALUE), &(struct hashtbl_creation_args){ .capacity = 256, .hash_func = hashtbl_default_hashfunc, .cmp_func = hashtbl_default_cmpfunc, __VA_ARGS__ }); \
_Pragma("GCC diagnostic pop")

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
void hashtbl_destroy(struct hashtbl* h);

bool hashtbl_add(struct hashtbl* h, void* key, void* value, bool is_unique);
struct hashtbl_element hashtbl_remove(struct hashtbl* h, void* key);
struct hashtbl_element hashtbl_find(struct hashtbl* h, void* key);
struct hashtbl_element hashtbl_find_filter(struct hashtbl* h, void* key, bool (*filter)(void* value, void* extra_args), void* extra_args);
struct hashtbl_element hashtbl_update(struct hashtbl* h, void* key, void* value);
bool hashtbl_update_with_func(struct hashtbl* h, void* key, struct hashtbl_element (*update_func)(struct hashtbl_element elm, void* args), void* extra_args);

void hashtbl_forall(struct hashtbl* h, void (*func)(struct hashtbl_element elm, void* args), void* extra_args);

#endif // HASHTBL_H
