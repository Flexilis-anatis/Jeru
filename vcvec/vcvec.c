#include "vcvec.h"
#include <stdlib.h>
#include <string.h>

#define GROWTH_FACTOR 1.5
#define DEFAULT_COUNT_OF_ELEMENETS 8
#define MINIMUM_COUNT_OF_ELEMENTS 2

// ----------------------------------------------------------------------------

// vcvec structure

struct vcvec {
  size_t count;
  size_t element_size;
  size_t reserved_size;
  char* data;
  vcvec_deleter* deleter;
};

// ----------------------------------------------------------------------------

// auxillary methods

bool vcvec_realloc(vcvec* vector, size_t new_count) {
  const size_t new_size = new_count * vector->element_size;
  char* new_data = (char*)realloc(vector->data, new_size);
  if (!new_data) {
    return false;
  }

  vector->reserved_size = new_size;
  vector->data = new_data;
  return true;
}

// [first_index, last_index)
void vcvec_call_deleter(vcvec* vector, size_t first_index, size_t last_index) {
  for (size_t i = first_index; i < last_index; ++i) {
    vector->deleter(vcvec_at(vector, i));
  }
}

void vcvec_call_deleter_all(vcvec* vector) {
  vcvec_call_deleter(vector, 0, vcvec_count(vector));
}

// ----------------------------------------------------------------------------

// Contol

vcvec* vcvec_create(size_t count_elements, size_t size_of_element, vcvec_deleter* deleter) {
  vcvec* v = (vcvec*)malloc(sizeof(vcvec));
  if (v != NULL) {
    v->data = NULL;
    v->count = 0;
    v->element_size = size_of_element;
    v->deleter = deleter;

    if (count_elements < MINIMUM_COUNT_OF_ELEMENTS) {
      count_elements = DEFAULT_COUNT_OF_ELEMENETS;
    }

    if (size_of_element < 1 ||
                 !vcvec_realloc(v, count_elements)) {
      free(v);
      v = NULL;
    }
  }

  return v;
}

vcvec* vcvec_create_copy(const vcvec* vector) {
  vcvec* new_vector = vcvec_create(vector->reserved_size / vector->count,
                                           vector->element_size,
                                           vector->deleter);
  if (!new_vector) {
    return new_vector;
  }

  if (memcpy(vector->data,
                      new_vector->data,
                      new_vector->element_size * vector->count) == NULL) {
    vcvec_release(new_vector);
    new_vector = NULL;
    return new_vector;
  }

  new_vector->count = vector->count;
  return new_vector;
}

void vcvec_release(vcvec* vector) {
  if (vector->deleter != NULL) {
    vcvec_call_deleter_all(vector);
  }

  if (vector->reserved_size != 0) {
    free(vector->data);
  }

  free(vector);
}

bool vcvec_is_equals(vcvec* vector1, vcvec* vector2) {
  const size_t size_vector1 = vcvec_size(vector1);
  if (size_vector1 != vcvec_size(vector2)) {
    return false;
  }

  return memcmp(vector1->data, vector2->data, size_vector1) == 0;
}

float vcvec_get_growth_factor() {
  return GROWTH_FACTOR;
}

size_t vcvec_get_default_count_of_elements() {
  return DEFAULT_COUNT_OF_ELEMENETS;
}

size_t vcvec_struct_size() {
  return sizeof(vcvec);
}

// ----------------------------------------------------------------------------

// Element access

void* vcvec_at(vcvec* vector, size_t index) {
  return vector->data + index * vector->element_size;
}

void* vcvec_front(vcvec* vector) {
  return vector->data;
}

void* vcvec_back(vcvec* vector) {
  return vector->data + (vector->count - 1) * vector->element_size;
}

void* vcvec_data(vcvec* vector) {
  return vector->data;
}

// ----------------------------------------------------------------------------

// Iterators

void* vcvec_begin(vcvec* vector) {
  return vector->data;
}

void* vcvec_end(vcvec* vector) {
  return vector->data + vector->element_size * vector->count;
}

void* vcvec_next(vcvec* vector, void* i) {
  return ((char *)i) + vector->element_size; // shut up -Wextra
}

// ----------------------------------------------------------------------------

// Capacity

bool vcvec_empty(vcvec* vector) {
  return vector->count == 0;
}

size_t vcvec_count(const vcvec* vector) {
  return vector->count;
}

size_t vcvec_size(const vcvec* vector) {
  return vector->count * vector->element_size;
}

size_t vcvec_max_count(const vcvec* vector) {
  return vector->reserved_size / vector->element_size;
}

size_t vcvec_max_size(const vcvec* vector) {
  return vector->reserved_size;
}

bool vcvec_reserve_count(vcvec* vector, size_t new_count) {
  if (new_count < vector->count) {
    return false;
  }

  size_t new_size = vector->element_size * new_count;
  if (new_size == vector->reserved_size) {
    return true;
  }

  return vcvec_realloc(vector, new_count);
}

bool vcvec_reserve_size(vcvec* vector, size_t new_size) {
  return vcvec_reserve_count(vector, new_size / vector->element_size);
}

// ----------------------------------------------------------------------------

// Modifiers

void vcvec_clear(vcvec* vector) {
  if (vector->deleter != NULL) {
    vcvec_call_deleter_all(vector);
  }

  vector->count = 0;
}

bool vcvec_insert(vcvec* vector, size_t index, const void* value) {
  if (vcvec_max_count(vector) < vector->count + 1) {
    if (!vcvec_realloc(vector, vcvec_max_count(vector) * GROWTH_FACTOR)) {
      return false;
    }
  }

  if (!memmove(vcvec_at(vector, index + 1),
                        vcvec_at(vector, index),
                        vector->element_size * (vector->count - index))) {

    return false;
  }

  if (memcpy(vcvec_at(vector, index),
                      value,
                      vector->element_size) == NULL) {
    return false;
  }

  ++vector->count;
  return true;
}

bool vcvec_erase(vcvec* vector, size_t index) {
  if (vector->deleter != NULL) {
    vector->deleter(vcvec_at(vector, index));
  }

  if (!memmove(vcvec_at(vector, index),
                        vcvec_at(vector, index + 1),
                        vector->element_size * (vector->count - index))) {
    return false;
  }

  vector->count--;
  return true;
}

bool vcvec_erase_range(vcvec* vector, size_t first_index, size_t last_index) {
  if (vector->deleter != NULL) {
    vcvec_call_deleter(vector, first_index, last_index);
  }

  if (!memmove(vcvec_at(vector, first_index),
                        vcvec_at(vector, last_index),
                        vector->element_size * (vector->count - last_index))) {
    return false;
  }

  vector->count -= last_index - first_index;
  return true;
}

bool vcvec_append(vcvec* vector, const void* values, size_t count) {
  const size_t count_new = count + vcvec_count(vector);

  if (vcvec_max_count(vector) < count_new) {
    size_t max_count_to_reserved = vcvec_max_count(vector) * GROWTH_FACTOR;
    while (count_new > max_count_to_reserved) {
      max_count_to_reserved *= GROWTH_FACTOR;
    }

    if (!vcvec_realloc(vector, max_count_to_reserved)) {
      return false;
    }
  }

  if (memcpy(vector->data + vector->count * vector->element_size,
                      values,
                      vector->element_size * count) == NULL) {
    return false;
  }

  vector->count = count_new;
  return true;
}

bool vcvec_push_back(vcvec* vector, const void* value) {
  if (!vcvec_append(vector, value, 1)) {
    return false;
  }

  return true;
}

bool vcvec_pop_back(vcvec* vector) {
  if (vector->deleter != NULL) {
    vector->deleter(vcvec_back(vector));
  }

  vector->count--;
  return true;
}

bool vcvec_replace(vcvec* vector, size_t index, const void* value) {
  if (vector->deleter != NULL) {
    vector->deleter(vcvec_at(vector, index));
  }

  return memcpy(vcvec_at(vector, index),
                value,
                vector->element_size) != NULL;
}

bool vcvec_replace_multiple(vcvec* vector, size_t index, const void* values, size_t count) {
  if (vector->deleter != NULL) {
    vcvec_call_deleter(vector, index, index + count);
  }

  return memcpy(vcvec_at(vector, index),
                values,
                vector->element_size * count) != NULL;
}
