#ifndef __DEBUG_H
#define __DEBUG_H

// Comment the below thing to stop debug
//#define DEBUG // comment back in for debugging output
// Comment the above thing to stop debug
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

#endif
