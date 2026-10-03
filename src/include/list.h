#pragma once
#include <stddef.h>

typedef struct list {
  void **items;
  size_t size;
  size_t capacity;
  size_t item_size;
} list_t;

list_t *list_initialize(size_t capacity, size_t item_size);
void list_append(list_t *list, void *item);
