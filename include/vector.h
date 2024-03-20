#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>

#define Vector(T) struct vector*

struct vector;

struct vector* vector_new(size_t sizeof_elm, size_t initial_cap);
void vector_destroy(struct vector* v);
void vector_forall(struct vector* v, void (*func)(void* elm, void* args), void* args);
void* vector_to_array(struct vector* v);
size_t vector_length(struct vector* v);
bool vector_resize(struct vector* v, size_t new_cap);
bool vector_push(struct vector* v, void* restrict elm);
void* vector_pop(struct vector* v);
bool vector_is_empty(struct vector* v);
size_t vector_find(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);
bool vector_all(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);
bool vector_any(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);

#endif // VECTOR_H
