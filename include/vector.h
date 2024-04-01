#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>

#define Vector(T) struct vector*

struct vector;

struct vector* vector_new(size_t sizeof_elm, size_t initial_cap);
void vector_destroy(struct vector* v);
void* vector_to_array(struct vector* v);
size_t vector_length(struct vector* v);
size_t vector_capacity(struct vector* v);
bool vector_is_empty(struct vector* v);

// The func and predicate parameters take the pointer on the current element as it first argument
void vector_forall(struct vector* v, void (*func)(void* elm, void* args), void* args);
size_t vector_find(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);
bool vector_all(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);
bool vector_any(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args);

// expand the capacity of the vector: new_cap must be strictly gretter than the current capacity of the vector
bool vector_resize(struct vector* v, size_t new_cap);
// the elm parameter must be the address of the element to push in the vector
bool vector_push(struct vector* v, void* restrict elm);
// return the address of the last element in the vector (or NULL if empty)
void* vector_pop(struct vector* v);

#endif // VECTOR_H
