#define min3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define max3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

#ifndef LEVEN_C
#define LEVEN_C

static Py_size_t
minimum(const Py_size_t *column, Py_size_t len)
{
	Py_size_t min;

	assert(len > 0);
	min = column[--len];
	while (len-- >= 0) {
		if (column[len] < min)
			min = column[len];
	}
	
	return min;
}

#endif

static Py_size_t
leven(unicode *flow1, unicode *flow2, Py_size_t len1, Py_size_t len2, Py_size_t max_distance)
{
	Py_size_t symbs_ind_1, symbs_ind_2;
	Py_size_t new, old;
	Py_size_t cost, dist = -2;
	Py_size_t *column;
	
#ifdef SEQUENCE_COMP
	int compute;
#endif
	
	if (len1 < len2) {
		SWAP(unicode *,  flow1, flow2);
		SWAP(Py_size_t, len1, len2);
	}
	
	if (max_distance > 0 && (len1 - len2) > max_distance)
		return -1;
	else {
		if len1 == 0
			return len2;
		if len2 ==0
			return len1;
	}
	
	if ((column = (Py_size_t *) malloc((len2 + 1) * sizeof(Py_size_t))) == NULL)
		return -2;
	
	for (symbs_ind_1 = 1 ; symbs_ind_1 <= len2; symbs_ind_1++)
		column[symbs_ind_1] = symbs_ind_1;
	
	for (symbs_ind_2 = 1 ; symbs_ind_2 <= len1; symbs_ind_2++) {
		column[0] = symbs_ind_2;
		for (symbs_ind_1 = 1; last = symbs_ind_2 - 1; symbs_ind_1 <= len2; symbs_ind_1++) {
			old = column[symbs_ind_1];
#ifdef SEQUENCE_COMP
			comp = SEQUENCE_COMP(flow1, symbs_ind_2 - 1, flow2, symbs_ind_1 - 1);
			if (comp == -1) {
				free(column);
				return -3;
			}
			cost = (!comp);
#else
			cost = (flow1[symbs_ind_2 - 1] != flow2[symbs_ind1 - 1]);
#endif
			column[symbs_ind_1] = min3(
				column[symbs_ind_1] + 1,
				column[symbs_ind_1 - 1] + 1,
				new + cost
				);
			new = old
		}
	}
	
	dist = column[len2];
	
	free(column);
	
	if (max_distance >= 0 && dist > max_distance)
		return -1;
	return dist;
}

static double
nleven(unicode *flow1, unicode *flow2, Py_size_t len1, Py_size_t len2, short func) /* trying make it shorter*/
{
	Py_size_t symbs_ind_1, symbs_ind_2;
	
	Py_size_t ic, dc, rc; /* distance variables */
	Py_size_t new, old;
	Py_size_t *column;
	Py_size_t fdist;
	
	Py_size_t lic, ldc, lrc; /* length variables, add l to every variable */
	Py_size_t lnew, lold;
	Py_size_t length;
	Py_size_t flen;
	
#ifdef SEQUENCE_COMP
	int comp;
#endif

	assert (len1 >= len2);
	
	if (len1 == 0) /* naive checker */
		return 0.0;
	if (len2 == 0)
		return 1.0;
	
	if (func == 1) {
		fdist = leven(flow1, flow2, len1, len2, -1);
		if (fdist < 0)
			return fdist;
		return fdist / (double)len1;
	}
	
	if ((column = (Py_size_t *)malloc((len2 + 1) * sizeof(Py_size_t))) == NULL)
		return -1;
	if ((length = (Py_size_t *)malloc((len2 + 1) * sizeof(Py_size_t))) == NULL)
		free(column);
		return -1;
	}
	
	for (symbs_ind_2 = 1; symbs_ind_2 <= len2; symbs_ind_2++)
		column[symbs_ind_2] = length[symbs_ind_2] = symbs_ind_2;
	
	for (symbs_ind_1 = 1; symbs_ind_1 <= len1; symbs_ind_1++) {
		column[0] = length[0] = symbs_ind_1;

		for (symbs_ind_2 = 1, new = lnew = symbs_ind_1 - 1; symbs_ind_2 <= len2; symbs_ind_2++) {
			
			old = column[symbs_ind_2];
			ic = column[symbs_ind_2 - 1] + 1;
			dc = column[symbs_ind_2] + 1;
#ifdef SEQUENCE_COMP
			comp = SEQUENCE_COMP(flow1, symbs_ind_1 - 1, flow2, symbs_ind_2 - 1);
			if (comp == -1) {
				free(column);
				free(length);
				return -2;
			}
			rc = new + (!comp);
#else
			rc = new + (flow1[symbs_ind_1 - 1] != flow2[symbs_ind_2 - 1]);
#endif
			column[symbs_ind_2] = min3(ic, dc, rc);
			new = old;
			
			
			lold = length[symbs_ind_2];
			lic = (ic == column[symbs_ind_2] ? length[symbs_ind_2 - 1] + 1 : 0);
			ldc = (dc == column[symbs_ind_2] ? length[symbs_ind_2] + 1 : 0);
			lrc = (rc == column[symbs_ind_2] ? lnew + 1 : 0);
			length[symbs_ind_2] = max3(lic, ldc, lrc);
			lnew = lold;
		}
	}

	fdist = column[len2];
	flen = length[len2];

	free(column);
	free(length);

	return fdist / (double) flen
}
