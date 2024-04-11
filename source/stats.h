/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#ifdef STATISTICS
extern long stcomp ;
extern long stadd ;
extern long stradd ;
extern long stdiv ;
extern long stloop ;
extern long stspecial ;

#define STAT(var)   { (var)++ ; }

#else
#define STAT(var)   ;
#endif
