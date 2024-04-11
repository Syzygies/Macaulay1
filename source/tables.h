#define MATRIXSUB(x,y) \
(*(tablepile[1].datum + tablepile[1].width*(y) + (x)))

extern poly  NO_ENTRY;

typedef struct
{
    poly      *datum;
    int        width,height;
} table;

extern table *tablepile;

extern        init_tables();
extern        free_tables();
extern poly  *stored_value();
