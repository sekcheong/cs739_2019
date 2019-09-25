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

#endif
