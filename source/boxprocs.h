// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Vtable structure for type-safe message dispatch.
// Each object type implements the messages it supports.
// NULL entries indicate unsupported messages.
typedef struct object_vtable
{
    // MTYPE (0) - Get type information
    void (*get_type)(void* obj);

    // MKILL (1) - Destroy/deallocate object
    void (*kill)(void* obj);

    // MSTART/MDODEG/MDEGS_RECEIVE (2) - Context-dependent message
    union
    {
        void (*start)(void* obj, int argc, char** argv);        // For VSTARTER
        boolean (*do_degree)(void* obj, int deg);               // For degree operations
        void (*degs_receive)(void* obj, void* dlist, int deg);  // For degs operations
    } msg2;

    // MPOLY_RECEIVE (3) - Receive polynomial
    void (*poly_receive)(void* obj, poly p, int deg);

    // MENDDEG (4) - End degree processing
    boolean (*end_degree)(void* obj, int deg);
} object_vtable;

// Type-safe dispatch function
const object_vtable*get_vtable(int type);

// Get specific message handler with proper type
void*get_message_handler(int type, int message);

// Forward declaration
typedef struct start_rec start_rec;

// Start record structure
struct start_rec
{
    int maxgen;          // Highest degree generator
    int lastdeg;         // Last completed degree
    int numstart;        // Number of boxes to start
    boolean iscomplete;  // Computation complete flag
    int hideg;           // Highest degree to compute
    boolean doalldegs;   // Compute all degrees flag
};

// Make routines for various variable types
variable*mkcollect(const char* name);
variable*mktrash(const char* name);
variable*mknres(const char* name, int intval);
variable*mkstd(const char* name);
variable*mkistd(const char* name, int intval, gmatrix M);
void mk_sev_nres(int first, int num_to_make, int intval, int last_intval);
variable*mkres(const char* name, int intval, gmatrix M);
variable*mkemit(const char* name, variable* Mvar);
variable*mkstdemit(const char* name, variable* Mvar);
variable*mklift(const char* name, variable* Mvar);
variable*mkmerge(const char* name);
variable*mkshift(const char* name, int shiftval);
variable*mkstarter(const char* name, int maxgen, int lastdeg, int numstart);

// Alias variable routines
void alias_type(variable* p);

// VSTARTER box type functions
start_rec*st_init(int maxgen, int lastdeg, int numstart);
void st_kill(variable* box);
void st_bounds(variable* box, int argc, char* argv[]);
void st_start(variable* box, int argc, char* argv[]);
void calc(variable* box, boolean doalldegs, int hideg);
void docalc(variable* box);
void st_pprint(start_rec* s);

// VCOLLECT box type functions
void coll_degs(variable* box, dlist* dl, int deg);
void coll_poly(variable* box, poly f, int deg);

// VTRASH box type functions
void trash_poly(variable* box, poly f, int deg);

// VEMIT box type functions
boolean emit_dodeg(variable* box, int deg);

// VSTDEMIT box type functions
boolean semit_dodeg(variable* box, int deg);

// VISTD box type functions
boolean istd_dodeg(variable* box, int deg);

// VRES box type functions
boolean res_dodeg(variable* box, int deg);

// VNRES box type functions
void ins_nres(variable* box, poly f, int deg);
boolean nres_enddeg(variable* box, int deg);

// VSTD box type functions
boolean std_enddeg(variable* box, int deg);
void std_degs(variable* box, dlist* dl, int deg);

// VMERGE box type functions
void mg_degs(variable* box, dlist* dl, int deg);
void mg_poly(variable* box, poly f, int deg);
boolean mg_enddeg(variable* box, int deg);

// VSHIFT box type functions
boolean shift_dodeg(variable* box, int deg);

// VLIFT box type functions
boolean lift_enddeg(variable* box, int deg);
void lift_poly(variable* box, poly f, int deg);

// Generic message passing function
void send_poly(variable* box, poly f, int deg);
