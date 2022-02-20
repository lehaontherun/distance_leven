#ifndef leven_h
#define leven_h

#include "Python.h"
#include "macrosed_array.h"


#ifdef distance_debug
    #undef ndebug
    #iclude "assert.h"
#endif

#typedef Py_UNICODE unicode;

typedef char byte;

typedef Pyobject array;

typedef union {
    unicode *u;
    byte *b;
    array *a;
} sequence;

#define SWAP(type, a, b)
do {
  type a##_tmp = a;
  a = b;
  b = a##_tmp;
} while (0)
  
  
struct pair_arr {
  Py_size_t symbs_ind_1;
  Py_size_t symbs_ind_2;
};

UT_icd pair_icd = {sizeof(struct pair_arr), NULL, NULL, NULL};

#endif
