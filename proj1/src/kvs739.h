#ifndef LIB739KVPY_H
#define LIB739KVPY_H

#include <memory>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <Python.h>
#include "lib739kv.h"


#ifdef __cplusplus
extern "C" {
#endif

//Provide a null-terminated array of server names (similarly to argv[]). 
//Each server name has the format "host:port" and initialize the client code. 
//Returns 0 on success and -1 on failure. 
static PyObject* kvs_init(PyObject *self, PyObject *args);

//Shutdown the connection to a server and free state.
//After calling this, client code should be able to call kv739_init() again to the same or a different server. 
static PyObject* kvs_shutdown(PyObject *self, PyObject *args);

//Retrieve the value corresponding to the key. 
//If the key is present, it should return 0 and store the value in the provided string. 
//The string must be at least 1 byte larger than the maximum allowed value. If the key is not present, it should return 1. 
//If there is a failure, it should return -1. 
static PyObject* kvs_get(PyObject *self, PyObject *args);

//Perform a get operation on the current value into old_value and then store the specified value. 
//Should return 0 on success if there is an old value, 1 on success if there was no old value, and -1 on failure. 
//The old_value parameter must be at least one byte larger than the maximum value size. 
static PyObject* kvs_put(PyObject *self, PyObject *args);

#ifdef __cplusplus
}
#endif


PyMODINIT_FUNC PyInit_kvs(void);

static PyMethodDef module_methods[] = { 
    {   
        "init", 
        (PyCFunction) kvs_init, 
        METH_VARARGS,
        "Initialize the KVS with a list of sserver name. Each server name has the format \"host:port\" and initialize the client code."
    },  
    {   
        "shutdown", 
        (PyCFunction) kvs_shutdown, 
        METH_NOARGS,
        "Shutdown the connection to a server and free state."
    }, 
    {   
        "get", 
        (PyCFunction) kvs_get, 
        METH_VARARGS,
        "Retrieve the value corresponding to the key."
    }, 
    {   
        "put", 
        (PyCFunction) kvs_put, 
        METH_VARARGS,
        "Perform a get operation on the current value into old_value and then store the specified value."
    }, 

    {NULL, NULL, 0, NULL}
};



typedef struct {
    PyObject_HEAD    
    PyObject *frame_callback;
} DataStore;


static PyMethodDef DataStore_methods[] = {    
    // {
    //     // "features",
    //     // (PyCFunction)Camera_features, 
    //     // METH_NOARGS,
    //     // "get list of camera features"
    // },
    
    {NULL, NULL, 0, NULL}
};


// static PyMemberDef DataStore_members[] = {
//   // {
//   //   "number", 
//   //   T_INT, 
//   //   offsetof(DataStore, number), 
//   //   0,
//   //   "DataStore number"
//   // },
//   {NULL, 0, 0, 0, NULL}  /* Sentinel */
// };


// static PyGetSetDef DataStore_getseters[] = {

//   // {
//   //   "get",
//   //   (getter)DataStore_get, 
//   //   NULL,
//   //   "get key",
//   //   NULL
//   // },

//   {NULL, 0, 0, NULL, NULL}  /* Sentinel */
// };

static PyObject *DataStore_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int DataStore_init(DataStore *self, PyObject *args, PyObject *kwds);
static void DataStore_dealloc(DataStore* self);

static PyTypeObject DataStoreType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "kvs.DataStore",            /* tp_name */
    sizeof(DataStore),          /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)DataStore_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "DataStore objects",        /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    DataStore_methods,          /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)DataStore_init,   /* tp_init */
    0,                          /* tp_alloc */
    DataStore_new,              /* tp_new */
};



typedef struct {
    PyObject_HEAD    
    PyObject *frame_callback;
} DataStoreClient;


static PyMethodDef DataStoreClient_methods[] = {    
    // {
    //     // "features",
    //     // (PyCFunction)Camera_features, 
    //     // METH_NOARGS,
    //     // "get list of camera features"
    // },
    
    {NULL, NULL, 0, NULL}
};


static PyObject *DataStoreClient_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int DataStoreClient_init(DataStoreClient *self, PyObject *args, PyObject *kwds);
static void DataStoreClient_dealloc(DataStoreClient* self);

static PyTypeObject DataStoreClientType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "kvs.DataStoreClient",            /* tp_name */
    sizeof(DataStoreClient),          /* tp_basicsize */
    0,                                /* tp_itemsize */
    (destructor)DataStoreClient_dealloc,    /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "DataStore remote client",  /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    DataStoreClient_methods,    /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)DataStoreClient_init,  /* tp_init */
    0,                          /* tp_alloc */
    DataStoreClient_new,        /* tp_new */
};

#endif