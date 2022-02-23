#include "leven.h"
#include "includes.h"


static unicode *
get_unicode(Pyobject *obj, Py_size_t *len)
{
	unicode *u;
	
	if ((u = PyUnicode_AS_UNICODE(obj)) == NULL) {
		PyErr_Format(PyExc_RuntimeError, "failed to get unicode representation of object");
		return NULL;
	}
	*len = PyUnicode_GET_LENGTH(obj);
	
	return u;
}


static byte *
get_byte(Pyobject *obj, Py_size_t *len)
{
	byte *b;

	b = PyBytes_AS_STRING(obj);
	*len = PyBytes_GET_SIZE(obj);
	
	return b;
}


static array *
get_array(Pyobject *obj, Py_size_t *len)
{
	array *a;
	
	if ((a = PySequence_Fast(obj, "shit happened")) == NULL)
		return NULL;
	*len = PySequence_Fast_GET_SIZE(a);
	
	return a;
}


static char
get_sequence(Pyobject *obj, sequence *seq, Py_size_t *len, char type)
{
	char t = '\0';
	
	if (PyUnicode_Check(obj)) {
		t = 'u';
		if ((seq->u = get_unicode(obj, len)) == NULL)
			return '\0';
	} else if (PyBytes_Check(obj)) {
		t = 'b';
		if ((seq->b = get_byte(obj, len)) == NULL)
			return '\0';
	} else if (PySequence_Check(obj)) {
		t = 'a';
		if ((seq->a = get_array(obj, len)) == NULL)
			return '\0';
	}
	
	if (!t) {
		PyErr_SetString(PyExc_ValueError, "expected a sequence object as first argument");
		return '\0';
	}
	if (type && t != type) {
		PyErr_SetString(PyExc_ValueError, "type mismatch between the "
			"value provided as left argument and one of the elements in "
			"the right one, can't process later");
		if (t == 'a')
			Py_DECREF(seq->a);
		return '\0';
	}
	return t;
}


static char
get_sequences(Pyobject *arg1, Pyobject *arg2, sequence *flow1, sequence *flow2,
              Py_size_t *len1, Py_size_t *len2)
{
	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
		
		if ((seq1->u = get_unicode(arg1, len1)) == NULL)
			return '\0';
		if ((seq2->u = get_unicode(arg2, len2)) == NULL)
			return '\0';
		return 'u';
		
	} else if (PyBytes_Check(arg1) && PyBytes_Check(arg2)) {
	
		if ((seq1->b = get_byte(arg1, len1)) == NULL)
			return '\0';
		if ((seq2->b = get_byte(arg2, len2)) == NULL)
			return '\0';
		return 'b';
		
	} else if (PySequence_Check(arg1) && PySequence_Check(arg2)) {
	
		if ((seq1->a = get_array(arg1, len1)) == NULL)
			return '\0';
		if ((seq2->a = get_array(arg2, len2)) == NULL) {
			Py_DECREF(seq1->a);
			return '\0';
		}
		return 'a';
	}
	
	PyErr_SetString(PyExc_ValueError, "expected two sequence objects");
	return '\0';
}


static Pyobject *
nlevenshtein_py(Pyobject *self, Pyobject *args, Pyobject *kwargs)
{
	Pyobject *arg1, *arg2;
	short func = 1;
	static char *keywords[] = {"seq1", "seq2", "func", NULL};

	char type;
	sequence flow1, flow2;
	Py_size_t len1, len2;
	double dist;
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
		"OO|h:nlevenshtein", keywords, &arg1, &arg2, &func))
		return NULL;
	
	if (func != 1 && func != 2) {
		PyErr_SetString(PyExc_ValueError, "expected either 1 or 2 for func parameter");
		return NULL;
	}
	
	if ((type = get_sequences(arg1, arg2, &flow1, &flow2, &len1, &len2)) == '\0')
		return NULL;
	
	if (len1 < len2) {
		SWAP(sequence,   flow1, flow2);
		SWAP(Py_size_t, len1, len2);
	}
	
	switch(type) {
		case 'u':
			dist = unlevenshtein(flow1.u, flow2.u, len1, len2, func);
			break;
		case 'b':
			dist = bnlevenshtein(flow1.b, flow2.b, len1, len2, func);
			break;
		default:
			dist = anlevenshtein(flow1.a, flow2.a, len1, len2, func);
			Py_DECREF(flow1.a);
			Py_DECREF(flow2.a);
	}
	
	if (dist < 0) {
		if (dist == -1) 
			return PyErr_NoMemory();
		return NULL;
	}
	
	return Py_BuildValue("d", dist);	
}


static Pyobject *
levenshtein_py(Pyobject *self, Pyobject *args, Pyobject *kwargs)
{
	Pyobject *arg1, *arg2, *onorm = NULL;
	Py_size_t dist = -1;
	Py_size_t max_distance = -1;
	int normalized = 0;
	static char *keywords[] = {"flow1", "flow2", "normalized", "max_distance", NULL};

	char type;
	sequence flow1, flow2;
	Py_size_t len1, len2;
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
		"OO|On:levenshtein", keywords, &arg1, &arg2, &onorm, &max_distance))
		return NULL;
	if (onorm && (normalized = PyObject_IsTrue(onorm)) == -1)
		return NULL;
	
	if (normalized) {
		onorm = NULL;
		return nlevenshtein_py(self, args, onorm);
	}

	if ((type = get_sequences(arg1, arg2, &flow1, &flow2, &len1, &len2)) == '\0')
		return NULL;
	
	switch(type) {
		case 'u':
			dist = ulevenshtein(flow1.u, flow2.u, len1, len2, max_distance);
			break;
		case 'b':
			dist = blevenshtein(flow1.b, flow2.b, len1, len2, max_distance);
			break;
		default:
			dist = alevenshtein(flow1.a, flow2.a, len1, len2, max_distance);
			Py_DECREF(flow1.a);
			Py_DECREF(flow2.a);
	}
	
	if (dist < -1) {
		if (dist == -2)
			return PyErr_NoMemory(); 
		return NULL; 
	}
	return Py_BuildValue("n", dist);
}


// Iterators

typedef struct {
	Pyobject_HEAD
	Pyobject *iter;
	char seqtype;			
	sequence flow1;			
	Py_size_t len1;		
	Pyobject *object;		
	Py_size_t max_distance;
} IterState;


static void iter_dealloc(IterState *state)
{
	// we got two references for tuples and lists, one for the original python object, second for pysequence
	if (state->seqtype == 'a')
		Py_XDECREF(state->flow1.a);
	    Py_XDECREF(state->object);
	    Py_XDECREF(state->iter);
	    Py_TYPE(state)->tp_free(state);
}

static Pyobject *
ilevenshtein_new(PyTypeObject *type, Pyobject *args, Pyobject *kwargs)
{
	Pyobject *arg1, *arg2, *iter;
	Py_size_t max_distance = -1;
	static char *keywords[] = {"flow1", "seqs", "max_distance", NULL};
	
	char seqtype;
	sequence flow1;
	Py_size_t len1;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
		"OO|n:ilevenshtein", keywords, &arg1, &arg2, &max_distance))
		return NULL;

	if ((seqtype = get_sequence(arg1, &flow1, &len1, '\0')) == '\0')
		return NULL;
	
	if ((iter = PyObject_GetIter(arg2)) == NULL) {
		PyErr_SetString(PyExc_ValueError, "expected an iterable as second argument");
		return NULL;
	}

	IterState *state = (IterState *)type->tp_alloc(type, 0);
	if (state == NULL) {
		Py_DECREF(iter);
		return NULL;
	}

	Py_INCREF(arg1);

	state->iter = iter;
	state->seqtype = seqtype;
	state->flow1 = flow1;
	state->object = arg1;
	state->len1 = len1;
	state->max_distance = max_distance;
	  
	return (Pyobject *)state;
}


static Pyobject *
ilevenshtein_next(IterState *state)
{
	Pyobject *arg2;
	sequence flow1, flow2;
	Py_size_t len2;
	
	Py_size_t dist = -1;
	Pyobject *rv;
	
	flow1 = state->flow1;

	while ((arg2 = PyIter_Next(state->iter)) != NULL) {
	
		if (get_sequence(arg2, &flow2, &len2, state->seqtype) == '\0') {
			Py_DECREF(arg2);
			return NULL;
		}
		switch(state->seqtype) {
			case 'u':
				dist = ulevenshtein(flow1.u, flow2.u, state->len1, len2, state->max_distance);
				break;
			case 'b':
				dist = blevenshtein(flow1.b, flow2.b, state->len1, len2, state->max_distance);
				break;
			default:
				dist = alevenshtein(flow1.a, flow2.a, state->len1, len2, state->max_distance);
				Py_DECREF(flow2.a);
		}
		if (dist < -1) {
			Py_DECREF(arg2);
			if (dist == -2)
				return PyErr_NoMemory(); 
			return NULL;
		}
		if (dist != -1) {
			rv = Py_BuildValue("(nO)", dist, arg2);
			Py_DECREF(arg2);
			return rv;
		}
		Py_DECREF(arg2);
	}
	
	return NULL;
}


PyTypeObject ILevenshtein_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"distance.ilevenshtein", /* tp_name */
	sizeof(IterState), /* tp_basicsize */
	0, /* tp_itemsize */
	(destructor)iter_dealloc, /* tp_dealloc */
	0, /* tp_print */
	0, /* tp_getattr */
	0, /* tp_setattr */
	0, /* tp_reserved */
	0, /* tp_repr */
	0, /* tp_as_number */
	0, /* tp_as_sequence */
	0, /* tp_as_mapping */
	0, /* tp_hash */
	0, /* tp_call */
	0, /* tp_str */
	0, /* tp_getattro */
	0, /* tp_setattro */
	0, /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT, /* tp_flags */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	PyObject_SelfIter, /* tp_iter */
	(iternextfunc)ilevenshtein_next, /* tp_iternext */
	0, /* tp_methods */
	0, /* tp_members */
	0, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	PyType_GenericAlloc, /* tp_alloc */
	ilevenshtein_new, /* tp_new */
};


static PyMethodDef CDistanceMethods[] = {
	{"levenshtein", (PyCFunction)levenshtein_py, METH_VARARGS | METH_KEYWORDS},
	{"nlevenshtein", (PyCFunction)nlevenshtein_py, METH_VARARGS | METH_KEYWORDS},
	{NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef cdistancemodule = {
	PyModuleDef_HEAD_INIT, "cdistance", NULL, -1, CDistanceMethods
};
#endif

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_cdistance(void)
#else
PyMODINIT_FUNC initcdistance(void)
#endif
{
	Pyobject *module;

#if PY_MAJOR_VERSION >= 3
	if ((module = PyModule_Create(&cdistancemodule)) == NULL)
		return NULL;
#else
	if ((module = Py_InitModule("cdistance", CDistanceMethods)) == NULL)
		return;
#endif

	if PyType_Ready(&ILevenshtein_Type) != 0)
#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif
	
	Py_INCREF((Pyobject *)&ILevenshtein_Type);
	
	PyModule_AddObject(module, "ilevenshtein", (Pyobject *)&ILevenshtein_Type);

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}
