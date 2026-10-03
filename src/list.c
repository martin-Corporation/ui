#include <list.h>
#include <stdlib.h>

list_t *list_initialize(size_t capacity, size_t item_size) {
  list_t *list = malloc(sizeof(list_t));
  list->size = 0;
  list->capacity = capacity;
  list->item_size = item_size;
  list->items = malloc(capacity * item_size);

  return list;
}

void list_append(list_t *list, void *item) {
  if (list->size >= list->capacity) {
    list->capacity++;
    list->items = realloc(list->items, list->item_size * list->capacity);
  }

  list->items[list->size] = item;
  list->size++;
}
