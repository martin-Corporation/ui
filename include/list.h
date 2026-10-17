#pragma once
#include <stddef.h>

typedef struct List {
  void **items;
  size_t size;
  size_t capacity;
  size_t item_size;
} List;

#if defined(__cplusplus)
extern "C" {
#endif
List *list_initialize(size_t capacity, size_t item_size);
void list_append(List *list, void *item);
#if defined(__cplusplus)
}
#endif
