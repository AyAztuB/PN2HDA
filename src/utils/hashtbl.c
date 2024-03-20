#include "hashtbl.h"

#include <stdlib.h>
#include <string.h>

enum hashtbl_data_state {
    FREED,
    OCCUPED,
    DELETED,
};

struct hashtbl {
    struct hashtbl_data {
        void* key;
        void* value;
        size_t hash_value;
        enum hashtbl_data_state state;
    }* data;
    size_t capacity;
    size_t sizeof_key;
    size_t sizeof_value;
    size_t nb_free_nodes;
    size_t (*hash_func)(const void* key);
    size_t (*cmp_func)(const void* key1, const void* key2);
};

size_t hashtbl_default_cmpfunc(const void* key1, const void* key2) {
    return strcmp(key1, key2);
}

size_t hashtbl_default_hashfunc(const void* key) {
    // JenkinsOAAT
    const char* str = key;
    size_t hash = 0;
    for(; *str; str++) {
        hash += *str;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

struct hashtbl* hashtbl_new(size_t sizeof_key, size_t sizeof_value, struct hashtbl_creation_args* extra_args) {
    struct hashtbl* h = malloc(sizeof(*h));
    if (!h) return NULL;
    if (!(h->data = calloc(extra_args->capacity, sizeof(*(h->data))))) {
        free(h);
        return NULL;
    }
    h->capacity = extra_args->capacity;
    h->cmp_func = extra_args->cmp_func;
    h->hash_func = extra_args->hash_func;
    h->nb_free_nodes = 0;
    h->sizeof_key = sizeof_key;
    h->sizeof_value = sizeof_value;
    return h;
}

static bool hashtbl_expand(struct hashtbl* h) {
    if (h->nb_free_nodes <= h->capacity / 4) {
        struct hashtbl_data* new_data = calloc(2 * h->capacity, sizeof(*new_data));
        if (!new_data)
            return false;
        size_t new_cap = h->capacity * 2;
        for (size_t i = 0; i < h->capacity; i++) {
            struct hashtbl_data d = h->data[i];
            if (d.state == OCCUPED) {
                size_t h = d.hash_value;
                for (; new_data[h % new_cap].state == OCCUPED; h++);
                new_data[h % new_cap] = d;
            }
        }
        h->nb_free_nodes = new_cap - (h->capacity - h->nb_free_nodes);
        h->capacity = new_cap;
        free(h->data);
        h->data = new_data;
    }
    return true;
}

bool hashtbl_add(struct hashtbl* h, void* key, void* value) {
    if (!hashtbl_expand(h))
        return false;
    (void)key;
    (void)value;
    return true;
}
