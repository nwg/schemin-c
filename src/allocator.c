#include "allocator.h"
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include "error.h"

#define ALLOCATOR_PAGES_REALLOC_COUNT 16
#define BYTE_ALLOCATOR_LARGE_THRESHOLD_DIVISOR 16

struct allocator_s {
  allocator_byte_t **pages;
  uint64_t current_page;
  uint64_t max_pages;
  uint64_t current_index;
  uint64_t total_elements;
  uint64_t remaining_elements_in_page;
  size_t element_size;
  size_t page_size;
};

static inline allocator_byte_t *make_page(size_t page_size) {
  allocator_byte_t *page = (allocator_byte_t*)mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  ASSERT_OR_ERROR(page != MAP_FAILED, "map failed");

  return page;
}

allocator_t *make_allocator(size_t element_size, size_t page_size) {
  assert(page_size % element_size == 0 && "Page size must be a multiple of element size");
  assert(page_size % (uint64_t)getpagesize() == 0 && "Page size must be a multiple of system page size");
  assert(ALLOCATOR_PAGES_REALLOC_COUNT > 0);
  allocator_t *allocator = (allocator_t*)malloc(sizeof(allocator_t));
  allocator->pages = (allocator_byte_t**)malloc(sizeof(allocator_byte_t*) * ALLOCATOR_PAGES_REALLOC_COUNT);
  allocator_byte_t *page = make_page(page_size);
  allocator->pages[0] = page;
  allocator->current_page = 0;
  allocator->max_pages = ALLOCATOR_PAGES_REALLOC_COUNT;
  allocator->current_index = 0;
  allocator->total_elements = 0;
  allocator->remaining_elements_in_page = page_size / element_size;
  allocator->element_size = element_size;
  allocator->page_size = page_size;

  return allocator;
}

void destroy_allocator(allocator_t *allocator) {
  for (uint64_t i = 0; i <= allocator->current_page; i++) {
    int result;
    allocator_byte_t *page = allocator->pages[i];
    result = munmap(page, allocator->page_size);
    ASSERT_OR_ERROR(result == 0, "Munmap failed");
  }

  free(allocator->pages);
  free(allocator);
}

void *allocator_allocate(allocator_t *allocator, uint64_t *outidx) {
  if (allocator->remaining_elements_in_page > 0) {
    allocator->remaining_elements_in_page--;
    uint64_t old_index = allocator->current_index;
    allocator->current_index++;
    if (outidx != NULL) *outidx = allocator->total_elements;
    allocator->total_elements++;
    return &(allocator->pages)[allocator->current_page][old_index * allocator->element_size];
  }

  allocator->current_page++;

  if (allocator->current_page >= allocator->max_pages) {
    allocator->max_pages += ALLOCATOR_PAGES_REALLOC_COUNT;
    allocator->pages = (allocator_byte_t**)realloc(allocator->pages, allocator->max_pages * sizeof(allocator_byte_t*));
  }

  assert(allocator->current_page < allocator->max_pages);
  allocator_byte_t *page = make_page(allocator->page_size);
  allocator->pages[allocator->current_page] = page;
  allocator->remaining_elements_in_page = allocator->page_size / allocator->element_size - allocator->element_size;
  allocator->current_index = 1;
  if (outidx != NULL) *outidx = allocator->total_elements;
  allocator->total_elements++;
  return &allocator->pages[allocator->current_page][0];
}

void *allocator_get_item_at_index(allocator_t *allocator, uint64_t idx) {
  assert(idx < allocator->total_elements && "Indexed beyond allocated elements");
  ASSERT_OR_ERROR(idx < ((uint64_t)1<<63), "index too large"); // @TODO implement unsigned lldiv
  lldiv_t div = lldiv((int64_t)idx * (int64_t)allocator->element_size, (int64_t)allocator->page_size);
  uint64_t page_idx = (uint64_t)div.quot;
  uint64_t idx_in_page = (uint64_t)div.rem;
  return &(allocator->pages)[page_idx][idx_in_page];
}

typedef struct byte_allocator_large_entry_s byte_allocator_large_entry_t;
struct byte_allocator_large_entry_s {
  allocator_byte_t *mem;
  size_t len;
  byte_allocator_large_entry_t *next;
};

struct byte_allocator_s {
  byte_allocator_large_entry_t *large_entries;
  allocator_byte_t **pages;
  uint64_t current_page;
  uint64_t max_pages;
  size_t current_offset;
  size_t total_bytes;
  size_t remaining_bytes_in_page;
  size_t page_size;
  size_t large_threshold;
};

typedef struct byte_allocator_header_s {
  size_t len;
} byte_allocator_header_t;

byte_allocator_t *make_byte_allocator(size_t page_size) {
  assert(page_size % (uint64_t)getpagesize() == 0 && "Page size must be a multiple of system page size");

  byte_allocator_t *allocator = (byte_allocator_t*)malloc(sizeof(allocator_t));
  allocator->large_entries = NULL;
  assert(ALLOCATOR_PAGES_REALLOC_COUNT > 0);
  allocator->pages = (allocator_byte_t**)malloc(sizeof(allocator_byte_t*) * ALLOCATOR_PAGES_REALLOC_COUNT);
  allocator_byte_t *page = make_page(page_size);
  allocator->pages[0] = page;
  allocator->current_page = 0;
  allocator->max_pages = ALLOCATOR_PAGES_REALLOC_COUNT;
  allocator->current_offset = 0;
  allocator->total_bytes = 0;
  allocator->remaining_bytes_in_page = page_size;
  allocator->page_size = page_size;
  allocator->large_threshold = page_size / BYTE_ALLOCATOR_LARGE_THRESHOLD_DIVISOR;

  return allocator;
}

void destroy_byte_allocator(byte_allocator_t *allocator) {
  for (uint64_t i = 0; i <= allocator->current_page; i++) {
    int result;
    allocator_byte_t *page = allocator->pages[i];
    result = munmap(page, allocator->page_size);
    ASSERT_OR_ERROR(result == 0, "Munmap failed");
  }

  for (byte_allocator_large_entry_t *large = allocator->large_entries; large != NULL; large = large->next) {
    munmap(large->mem, large->len);
  }

  free(allocator->pages);
  free(allocator);
}

allocator_byte_t *byte_allocator_allocate(byte_allocator_t *allocator, size_t size) {
  if (size < allocator->remaining_bytes_in_page) {
    allocator->remaining_bytes_in_page -= size;
    size_t old_offset = allocator->current_offset;
    allocator->current_offset += size;
    allocator->total_bytes += size;

    return &(allocator->pages)[allocator->current_page][old_offset];
  }

  if (size >= allocator->large_threshold) {
    byte_allocator_large_entry_t *entry = (byte_allocator_large_entry_t*)malloc(sizeof(byte_allocator_large_entry_t));
    entry->mem = make_page(size);
    entry->len = size;
    entry->next = allocator->large_entries;
    allocator->large_entries = entry;
    return entry->mem;
  }

  allocator->current_page++;

  if (allocator->current_page >= allocator->max_pages) {
    allocator->max_pages += ALLOCATOR_PAGES_REALLOC_COUNT;
    allocator->pages = (allocator_byte_t**)realloc(allocator->pages, allocator->max_pages * sizeof(allocator_byte_t*));
  }

  assert(allocator->current_page < allocator->max_pages);
  allocator_byte_t *page = make_page(allocator->page_size);
  allocator->pages[allocator->current_page] = page;
  allocator->remaining_bytes_in_page = allocator->page_size - size;
  allocator->current_offset = size;
  allocator->total_bytes += size;
  return &(allocator->pages)[allocator->current_page][0];
}
