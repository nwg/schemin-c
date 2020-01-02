#ifndef SCHEMIN_ALLOCATOR_H
#define SCHEMIN_ALLOCATOR_H
SCHEMIN_ALLOCATOR_H
#include <stdlib.h>


typedef char allocator_byte_t;
typedef struct allocator_s allocator_t;
typedef struct byte_allocator_s byte_allocator_t;

allocator_t *make_allocator(size_t element_size, size_t page_size);
void destroy_allocator(allocator_t *allocator);
void *allocator_allocate(allocator_t *allocator, uint64_t *outidx);
void *allocator_get_item_at_index(allocator_t *allocator, uint64_t idx);

byte_allocator_t *make_byte_allocator(size_t page_size);
void destroy_byte_allocator(byte_allocator_t *allocator);
allocator_byte_t *byte_allocator_allocate(byte_allocator_t *allocator, size_t size);

#endif

