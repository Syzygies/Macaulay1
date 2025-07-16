// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Basic operations on polynomial lists
void pl_new(plist* result, int size);
void pl_add(plist* result, plist* a, plist* b);
void pl_sub(plist* result, plist* a, plist* b);

// Matrix operations on polynomial lists
void pl_transpose(plist* result, plist* a, int ncols);
void pl_mult(plist* result, plist* a, plist* b, int nrows);
void pl_dsum(plist* result, plist* a, plist* b, int shiftval);
void pl_submat(plist* result, plist* a, dlist* drows, dlist* dcols);

// Special constructors
void pl_diag(plist* result, poly f, int size);
