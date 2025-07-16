// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Process original generators
void IHorig_gens(gmatrix M, int intval, variable* B);

// Main standard basis calculation - always returns TRUE
boolean IHcalc_standard(gmatrix M, variable* B);

// Send polynomial to output or insert as new basis element
void IHsend_off(gmatrix M, variable* B, poly h, poly hrep);

// Insert element into basis
void IHins_elem(gmatrix M, poly h, poly hrep);

// Monomial operations
void IHmn_adjoin(gmatrix M, arrow head, expterm nexp, char* bag);
void IHmn_lcm(arrow head, arrow* newh, expterm m, arrow mloc);
void IHmo_insert(gmatrix M, term m1, mn_standard bag);
boolean IHmo_next_pair(gmatrix M, mn_standard* i, mn_standard* j);
