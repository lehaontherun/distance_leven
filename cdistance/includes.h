#define SEQUENCE_COMPARE(s1, i1, s2, i2) \
(PyObject_RichCompareBool( \
	PySequence_Fast_GET_ITEM((s1), (i1)), \
	PySequence_Fast_GET_ITEM((s2), (i2)), \
	Py_EQ) \
)

#define unicode unicode
#define leven ulevenshtein
#define nleven unlevenshtein
#include "levenshtein.c"
#undef unicode
#undef leven
#undef nleven

#define unicode byte
#define leven ulevenshtein
#define nleven unlevenshtein
#include "levenshtein.c"
#undef unicode
#undef leven
#undef nleven

#define SEQUENCE_COMP SEQUENCE_COMPARE
#define unicode array
#define leven ulevenshtein
#define nleven unlevenshtein
#include "levenshtein.c"
#undef unicode
#undef leven
#undef nleven
#undef SEQUENCE_COMP
