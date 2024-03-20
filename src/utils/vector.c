#include "vector.h"

#include <stdlib.h>
#include <string.h>

struct vector {
    void* array;
    size_t sizeof_elm;
    size_t length;
    size_t capacity;
};

struct vector* vector_new(size_t sizeof_elm, size_t initial_cap) {
    if (!initial_cap)
        initial_cap = 100;
    struct vector* v = malloc(sizeof(*v));
    if (!v) return NULL;
    v->array = malloc(sizeof_elm * initial_cap);
    if (!v->array) {
        free(v);
        return NULL;
    }
    v->capacity = initial_cap;
    v->length = 0;
    v->sizeof_elm = sizeof_elm;
    return v;
}

void vector_destroy(struct vector* v) {
    free(v->array);
    free(v);
}

void vector_forall(struct vector* v, void (*func)(void* elm, void* args), void* args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        func(((char*)v->array) + i, args);
    }
}

void* vector_to_array(struct vector* v) {
    return v->array;
}

size_t vector_length(struct vector* v) {
    return v->length;
}

bool vector_resize(struct vector* v, size_t new_cap) {
    if (new_cap <= v->capacity)
        return false;
    void* arr = realloc(v->array, v->sizeof_elm * new_cap);
    if (!arr)
        return false;
    v->array = arr;
    v->capacity = new_cap;
    return true;
}

bool vector_push(struct vector* v, void* restrict elm) {
    if (v->length >= v->capacity && !vector_resize(v, 2 * v->capacity))
        return false;
    memcpy(((char*)v->array) + v->length, elm, v->sizeof_elm);
    (v->length)++;
    return true;
}

void* vector_pop(struct vector* v) {
    if (!v->length)
        return NULL;
    return ((char*)v->array) + --(v->length);
}

bool vector_is_empty(struct vector* v) {
    return v->length == 0;
}

size_t vector_find(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (predicate(((char*)v->array) + i, extra_args))
            return i / v->sizeof_elm;
    }
    return v->length;
}

bool vector_all(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (!predicate(((char*)v->array) + i, extra_args))
            return false;
    }
    return true;
}

bool vector_any(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (predicate(((char*)v->array) + i, extra_args))
            return true;
    }
    return false;
}
