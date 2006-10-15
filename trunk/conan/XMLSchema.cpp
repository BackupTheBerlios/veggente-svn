#include <Python.h>

/* Type-specific fields go here. */
typedef struct {
    PyObject_HEAD
} XMLSchema;

/* ----- Allocation functions ----- */

static int XMLSchema_init(XMLSchema *self,PyObject *args, PyObject *kwds) {
    /* Completamente da prendere come un esempio*/
    PyObject *first=NULL, *last=NULL, *tmp;

    static char *kwlist[] = {"first", "last", "number", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist,&first, &last, &self->number))
        return -1; 

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }
    if (last) {
        tmp = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(tmp);
    }
    return 0;

}

static PyObject* XMLSchema_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    XMLSchema *self;

    self = (XMLSchema *)type->tp_alloc(type, 0);
    if (self != NULL) {
        /* Inizializza gli attributi di classi */
        /*self->first = PyString_FromString("");
        if (self->first == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->number = 0;
        */
    }
    return (PyObject *)self;
}

/* Class members registration*/
static PyMemberDef XMLSchema_members[] = {
    {NULL}
};

static PyObject* XMLSchema_parse{} 

/* Class methods registration */
static PyMemberDef XMLSchema_methods[] = {
    {"parse",(PyCFunction)XMLSchema_parse,,"Parses an XML Schema model and builds a simpler object model"},
    {NULL}
};

/* ----- Deallocation functions ----- */
static int Noddy_traverse(Noddy *self, visitproc visit, void *arg) {
    /*Py_VISIT(self-> variabile da piallare);*/
    return 0;
}

static int XMLSchema_clear(XMLSchema *self) {
    PyObject *tmp;
    /*Annullare ogni componente della struct come da esempio*/
/*    tmp = self->first;
    self->first = NULL;
    Py_XDECREF(tmp);*/
    return 0;
}

static void XMLSchema_dealloc(Noddy* self) {
    XMLSchema_clear(self);
    self->ob_type->tp_free((PyObject*)self);
}

/* Python type definition */
PyTypeObject PyXMLSchema_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"conan.XMLSchema",
	sizeof(XMLSchema_obj),
	0,                  /* tp_itemsize */
	(destructor)XMLSchema_dealloc,					/* tp_dealloc */
	0,      			/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,		        	/* tp_repr */
	0,	        		/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
    0,					/* tp_call */
    0,			        /* tp_str */
	0,					/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES, /* tp_flags */
	"XML Schema object",		        		/* tp_doc */
	(traverseproc)XMLSchema_traverse,			/* tp_traverse */
	(inquiry)XMLSchema_clear,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	XMLSchema_methods,					/* tp_methods */
	XMLSchema_members,					/* tp_members */
	0,					/* tp_getset */
	0,  				/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	(initproc)XMLSchema_init,					/* tp_init */
	0,					/* tp_alloc */
	XMLSchema_new,		/* tp_new */
};
