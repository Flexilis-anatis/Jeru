#ifndef VCVECTOR_H
#define VCVECTOR_H

#include <stdbool.h>
#include <stdio.h>

#define VECLOOP(type, name, vector) \
    for(type *name = (type *)vcvec_begin(vector); \
        (void *)name != vcvec_end(vector); \
        name = (type *)vcvec_next(vector, (void *)name))

typedef struct vcvec vcvec;
typedef void (vcvec_deleter)(void *);

// ----------------------------------------------------------------------------
// Control
// ----------------------------------------------------------------------------

// Constructs an empty vector with an reserver size for count_elements.
vcvec* vcvec_create(size_t count_elements, size_t size_of_element, vcvec_deleter* deleter);

// Constructs a copy of an existing vector.
vcvec* vcvec_create_copy(const vcvec* vector);

// Releases the vector.
void vcvec_release(vcvec* vector);

// Compares vector content
bool vcvec_is_equals(vcvec* vector1, vcvec* vector2);

// Returns constant value of the vector growth factor.
float vcvec_get_growth_factor();

// Returns constant value of the vector default count of elements.
size_t vcvec_get_default_count_of_elements();

// Returns constant value of the vector struct size.
size_t vcvec_struct_size();

// ----------------------------------------------------------------------------
// Element access
// ----------------------------------------------------------------------------

// Returns the item at index position in the vector.
void* vcvec_at(vcvec* vector, size_t index);

// Returns the first item in the vector.
void* vcvec_front(vcvec* vector);

// Returns the last item in the vector.
void* vcvec_back(vcvec* vector);

// Returns a pointer to the data stored in the vector. The pointer can be used to access and modify the items in the vector.
void* vcvec_data(vcvec* vector);

// ----------------------------------------------------------------------------
// Iterators
// ----------------------------------------------------------------------------

// Returns a pointer to the first item in the vector.
void* vcvec_begin(vcvec* vector);

// Returns a pointer to the imaginary item after the last item in the vector.
void* vcvec_end(vcvec* vector);

// Returns a pointer to the next element of vector after 'i'.
void* vcvec_next(vcvec* vector, void* i);

// ----------------------------------------------------------------------------
// Capacity
// ----------------------------------------------------------------------------

// Returns true if the vector is empty; otherwise returns false.
bool vcvec_empty(vcvec* vector);

// Returns the number of elements in the vector.
size_t vcvec_count(const vcvec* vector);

// Returns the size (in bytes) of occurrences of value in the vector.
size_t vcvec_size(const vcvec* vector);

// Returns the maximum number of elements that the vector can hold.
size_t vcvec_max_count(const vcvec* vector);

// Returns the maximum size (in bytes) that the vector can hold.
size_t vcvec_max_size(const vcvec* vector);

// Resizes the container so that it contains n elements.
bool vcvec_reserve_count(vcvec* vector, size_t new_count);

// Resizes the container so that it contains new_size / element_size elements.
bool vcvec_reserve_size(vcvec* vector, size_t new_size);

// ----------------------------------------------------------------------------
// Modifiers
// ----------------------------------------------------------------------------

// Removes all elements from the vector (without reallocation).
void vcvec_clear(vcvec* vector);

// The container is extended by inserting a new element at position.
bool vcvec_insert(vcvec* vector, size_t index, const void* value);

// Removes from the vector a single element by 'index'
bool vcvec_erase(vcvec* vector, size_t index);

// Removes from the vector a range of elements '[first_index, last_index)'.
bool vcvec_erase_range(vcvec* vector, size_t first_index, size_t last_index);

// Inserts multiple values at the end of the vector.
bool vcvec_append(vcvec* vector, const void* values, size_t count);

// Inserts value at the end of the vector.
bool vcvec_push_back(vcvec* vector, const void* value);

// Removes the last item in the vector.
bool vcvec_pop_back(vcvec* vector);

// Replace value by index in the vector.
bool vcvec_replace(vcvec* vector, size_t index, const void* value);

// Replace multiple values by index in the vector.
bool vcvec_replace_multiple(vcvec* vector, size_t index, const void* values, size_t count);

#endif // VCVECTOR_H
