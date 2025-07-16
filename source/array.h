// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Array manipulation functions
void array_init(void);
void init_array(array *a, int element_size);
void free_array(array *a);
int length(array *a);
void *ref(array *a, int i);
void *ins_array(array *a);

// Global variable
extern char *array_stash;
