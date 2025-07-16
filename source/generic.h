// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Variable type constants
constexpr int VNOTYPE = 0;
constexpr int VINT = 1;
constexpr int VRING = 2;
constexpr int VPOLY = 3;
constexpr int VMODULE = 4;
constexpr int VCOMPLEX = 5;
constexpr int VSTARTER = 6;
constexpr int VEMIT = 7;
constexpr int VSTDEMIT = 8;
constexpr int VRES = 9;
constexpr int VSHIFT = 10;
constexpr int VCOLLECT = 11;
constexpr int VTRASH = 12;
constexpr int VLIFT = 13;
constexpr int VMERGE = 14;
constexpr int VNRES = 15;
constexpr int VSTD = 16;
constexpr int VISTD = 17;

// Message type constants
constexpr int MTYPE = 0;
constexpr int MKILL = 1;
constexpr int MSTART = 2;
constexpr int MDODEG = 2;
constexpr int MDEGS_RECEIVE = 2;
constexpr int MPOLY_RECEIVE = 3;
constexpr int MENDDEG = 4;

// Exported globals
extern const char* type_names[];
