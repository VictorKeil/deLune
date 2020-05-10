#include <stdlib.h>
#include <stdio.h>

#define ARRAY_EXPAND_LENGTH 32

void mem_dump(unsigned char *ptr, int size) {
  for (int i = 0; i < size; i++) {
    printf("%02x ", *(ptr + i));
    if ((i + 1) % 8 == 0)
      printf("\n");
  }
  printf("\n");
}

void init_array(void **array, int elem_size, int length) {
  int size = elem_size * length;
  *array = calloc(1, size);
}

void resize_array(void **array, int elem_size, int length) {
  int new_size = elem_size * length;
  *array = realloc(*array, new_size);
}

