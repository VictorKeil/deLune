#ifndef _UTILS_H_
#define _UTILS_H_

void mem_dump(unsigned char *ptr, int size);

void init_array(void **array, int elem_size, int length);

void resize_array(void **array, int elem_size, int length);

#endif /* _UTILS_H_ */
