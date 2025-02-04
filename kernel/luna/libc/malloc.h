#ifdef __cplusplus
extern "C" {
#endif
#pragma once

typedef unsigned long size_t;
typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long long qword_t;

// extern void initalize_memory(void);
// extern void log_starting_memory(void);
extern void* malloc(size_t mem_length);
// extern void* aligned_malloc(size_t mem_length, size_t mem_alignment);
extern void* calloc(size_t mem_length, size_t num);


extern void* realloc(void* mem_pointer, size_t mem_length);
extern void free(void* mem_pointer);

#ifdef __cplusplus
}
#endif