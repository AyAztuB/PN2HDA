#include "vector.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief The vector struct represents a dynamic array.
 */
struct vector {
    void* array;            /**< Pointer to the array containing elements. */
    size_t sizeof_elm;      /**< Size of each element in bytes. */
    size_t length;          /**< Number of elements currently in the array. */
    size_t capacity;        /**< Maximum number of elements until resize. */
};

/**
 * @brief Creates a new vector instance.
 *
 * This function creates a new vector instance with the specified size of each element
 * and initial capacity. If the initial capacity is 0, a default value of 128 will be used.
 *
 * @param sizeof_elm Size of each element in bytes.
 * @param initial_cap Initial capacity of the vector. If 0, a default capacity of 128 will be used.
 * @return A pointer to the newly created vector struct on success, or NULL if memory allocation fails.
 *
 * @see vector_free
 */
struct vector* vector_new(size_t sizeof_elm, size_t initial_cap) {
    if (!initial_cap)
        initial_cap = 128;
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

/**
 * @brief Destroys a vector instance and frees memory.
 *
 * This function deallocates the memory used by the vector instance.
 * It's the responsibility of the caller to ensure that no other references
 * to the vector exist after calling this function to avoid memory leaks.
 * Note: This function does not free the memory of elements pushed into the vector.
 *
 * @param v Pointer to the vector instance to be destroyed. Must not be NULL.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 * @warning This function does not free the memory of elements pushed into the vector.
 *
 * @see vector_new
 */
void vector_destroy(struct vector* v) {
    free(v->array);
    free(v);
}

/**
 * @brief Applies a function to all elements of the vector.
 *
 * This function applies the specified function to all elements of the vector.
 * The function takes the address of each element as its first argument.
 * Any extra arguments passed to this function are ignored by `vector_forall`.
 * It is the responsibility of the caller to ensure that the vector is not NULL.
 *
 * @param v Pointer to the vector instance.
 * @param func Pointer to the function to be applied to each element.
 * @param args Extra arguments passed to the function `func`. Not used by `vector_forall`.
 * @warning Passing a NULL pointer to the vector will result in undefined behavior.
 * @warning The first argument `elm` of the function `func` is the address of the element in the vector.
 *
 * @see vector_destroy
 */
void vector_forall(struct vector* v, void (*func)(void* elm, void* args), void* args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        func(((char*)v->array) + i, args);
    }
}

/**
 * @brief Converts the vector to a contiguous array.
 *
 * This function returns a pointer to a contiguous array containing
 * all the elements of the vector. The ownership of the returned array
 * remains with the vector, and the vector can still be used afterwards.
 * The caller should not free the memory of the returned array manually.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @return Pointer to a contiguous array containing the elements of the vector.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 * @warning The caller should not free the memory of the returned array manually.
 * @note The ownership of the returned array remains with the vector, and the vector
 *       can still be used afterwards.
 *
 * @see vector_destroy
 */
void* vector_to_array(struct vector* v) {
    return v->array;
}

/**
 * @brief Retrieves the current length of the vector.
 *
 * This function returns the number of elements currently stored in the vector.
 * It does not modify the vector in any way.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @return The number of elements currently stored in the vector.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_capacity
 */
size_t vector_length(struct vector* v) {
    return v->length;
}

/**
 * @brief Retrieves the current capacity of the vector.
 *
 * This function returns the maximum number of elements that the vector
 * can currently hold without resizing its internal storage.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @return The current capacity of the vector.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_resize
 */
size_t vector_capacity(struct vector* v) {
    return v->capacity;
}

/**
 * @brief Resizes the capacity of the vector.
 *
 * This function resizes the capacity of the vector to the specified value.
 * If successful, the capacity of the vector will be set to `new_cap`.
 * If the reallocation fails or if `new_cap` is less than or equal to the current capacity,
 * the function will return false, and the previous vector will remain usable.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @param new_cap The new capacity to set for the vector.
 * @return true if the resizing operation is successful, false otherwise.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_capacity
 */
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

/**
 * @brief Pushes an element into the vector.
 *
 * This function pushes the specified element into the vector. If the vector is full,
 * its capacity will be doubled using `vector_resize`. If the reallocation fails, the
 * previous vector remains usable, and the function returns false. Otherwise, the element
 * is successfully pushed into the vector, and the function returns true.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @param elm Pointer to the element to be pushed into the vector.
 * @return true if the element is successfully pushed into the vector, false otherwise.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_resize
 */
bool vector_push(struct vector* v, void* restrict elm) {
    if (v->length >= v->capacity && !vector_resize(v, 2 * v->capacity))
        return false;
    memcpy(((char*)v->array) + v->length * v->sizeof_elm, elm, v->sizeof_elm);
    (v->length)++;
    return true;
}

/**
 * @brief Removes and returns the last element from the vector.
 *
 * This function removes and returns the last element from the vector.
 * If the vector is empty, it returns NULL. After pushing a new element
 * into the vector, the returned pointer from a previous call to `vector_pop`
 * must not be used, as it will lead to undefined behavior.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @return Pointer to the last element removed from the vector, or NULL if the vector is empty.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_push
 */
void* vector_pop(struct vector* v) {
    if (!v->length)
        return NULL;
    return ((char*)v->array) + --(v->length);
}

/**
 * @brief Checks if the vector is empty.
 *
 * This function checks if the vector is empty, i.e., it contains no elements.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @return true if the vector is empty, false otherwise.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_length
 */
bool vector_is_empty(struct vector* v) {
    return v->length == 0;
}

/**
 * @brief Finds the index of the first element satisfying the predicate function.
 *
 * This function finds the index of the first element in the vector that satisfies
 * the given predicate function. The predicate function takes the address of each
 * element as its first argument and any extra arguments passed to this function
 * are ignored by `vector_find`.
 *
 * If no element satisfies the predicate function, the function returns the length
 * of the vector.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @param predicate Pointer to the predicate function.
 * @param extra_args Extra arguments passed to the predicate function. Not used by `vector_find`.
 * @return The index of the first element satisfying the predicate function, or the length of the vector if not found.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_forall
 * @see vector_length
 */
size_t vector_find(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (predicate(((char*)v->array) + i, extra_args))
            return i / v->sizeof_elm;
    }
    return v->length;
}

/**
 * @brief Checks if all elements of the vector satisfy the predicate function.
 *
 * This function checks if all elements of the vector satisfy the given predicate function.
 * The predicate function takes the address of each element as its first argument, and any
 * extra arguments passed to this function are ignored by `vector_all`.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @param predicate Pointer to the predicate function.
 * @param extra_args Extra arguments passed to the predicate function. Not used by `vector_all`.
 * @return true if all elements of the vector satisfy the predicate function, false otherwise.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_forall
 */
bool vector_all(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (!predicate(((char*)v->array) + i, extra_args))
            return false;
    }
    return true;
}

/**
 * @brief Checks if at least one element of the vector satisfies the predicate function.
 *
 * This function checks if at least one element of the vector satisfies the given predicate function.
 * The predicate function takes the address of each element as its first argument, and any
 * extra arguments passed to this function are ignored by `vector_any`.
 *
 * @param v Pointer to the vector instance. Must not be NULL.
 * @param predicate Pointer to the predicate function.
 * @param extra_args Extra arguments passed to the predicate function. Not used by `vector_any`.
 * @return true if at least one element of the vector satisfies the predicate function, false otherwise.
 * @warning Passing a NULL pointer to this function will result in undefined behavior.
 *
 * @see vector_forall
 */
bool vector_any(struct vector* v, bool (*predicate)(void* elm, void* args), void* extra_args) {
    for(size_t i = 0; i < v->length * v->sizeof_elm; i += v->sizeof_elm) {
        if (predicate(((char*)v->array) + i, extra_args))
            return true;
    }
    return false;
}
