// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Type definitions
typedef struct
{
    int codim;
    int deg0;
    int degree;
    long genfun[GFSIZE];
} hilbFunc;

// Exported functions
void h_init(hilbFunc* hf, int deg0);
void h_addto(hilbFunc* hf, long gf[], int deg0);
void h_divAll(hilbFunc* hf);
void h_div(hilbFunc* hf);
void h_display(hilbFunc* hf);
long h_genus(hilbFunc* hf);
boolean hilb(hilbFunc* hf, gmatrix M);
void hilbDisplay(hilbFunc* hf, gmatrix M);
int hilb_cmd(int argc, char* argv[]);
void numer_tell(arrow head, int plus);
boolean hilb_numer(gmatrix M);
int hilb_numer_cmd(int argc, char* argv[]);
poly monom_prod(int exp[], int n, gmatrix rmap);
int monoms_cmd(int argc, char* argv[]);
int binoms_cmd(int argc, char* argv[]);
