#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define MAX3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

#ifndef LEVENSHTEIN_C
#define LEVENSHTEIN_C

static Py_ssize_t
minimum(const Py_ssize_t *column, Py_ssize_t len)
{
	Py_ssize_t min;

	assert(len > 0);
	min = column[--len];
	while (len-- >= 0) {
		if (column[len] < min)
			min = column[len];
	}
	
	return min;
}
