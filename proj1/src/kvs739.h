#ifndef LIB739KVPY_H
#define LIB739KVPY_H

#include <memory>
#include <iostream>
#include <cstring>
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib739kv.h"
#include "datastore.h"
#include "dsclient.h"
#include "dsserver.h"
#include "message.h"
#include "rpcserver.h"
#include "strutil.h"

// static PyObject* make_server_list(char** server_list);

// static int init_callback(char** server_list);
// static int get_callback(char** server_list);
// static int put_callback(char** server_list);
// static int shutdown_callback(char** server_list);

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

//Get the OS timestamp in micro seconds
static PyObject* kvs_timestamp(PyObject *self, PyObject *args);

static PyObject* kvs_init_handler(PyObject *self, PyObject *args);

static PyObject* kvs_put_handler(PyObject *self, PyObject *args);

static PyObject* kvs_get_handler(PyObject *self, PyObject *args);

static PyObject* kvs_shutdown_handler(PyObject *self, PyObject *args);


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

    {   
        "timestamp", 
        (PyCFunction) kvs_timestamp, 
        METH_NOARGS,
        "Get the OS timestamp in micro seconds."
    },

    {   
        "init_handler", 
        (PyCFunction) kvs_init_handler, 
        METH_VARARGS,
        "Set the init handler"
    },  
    
    {   
        "put_handler", 
        (PyCFunction) kvs_put_handler, 
        METH_VARARGS,
        "Set the put handler"
    }, 

    {   
        "get_handler", 
        (PyCFunction) kvs_get_handler, 
        METH_VARARGS,
        "Set the get handler"
    }, 
    
    {   
        "shutdown_handler", 
        (PyCFunction) kvs_shutdown_handler, 
        METH_VARARGS,
        "Set the shutdown handler"
    },

    {NULL, NULL, 0, NULL}
};



typedef struct {
    PyObject_HEAD 
    data_store *store_p;
} DataStore;


static PyObject *DataStore_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int DataStore_init(DataStore *self, PyObject *args, PyObject *kwds);
static void DataStore_dealloc(DataStore* self);
static PyObject* DataStore_get(DataStore *self, PyObject *args);
static PyObject* DataStore_put(DataStore *self, PyObject *args);
static PyObject* DataStore_get_meta(DataStore *self, PyObject *args);
static PyObject* DataStore_put_meta(DataStore *self, PyObject *args);
static PyObject* DataStore_timestamp(DataStore *self, PyObject *args);
static PyObject* DataStore_last_timestamp(DataStore *self, PyObject *args);
static PyObject* DataStore_first_timestamp(DataStore *self, PyObject *args);


static PyMethodDef DataStore_methods[] = {    
    {   
        "get", 
        (PyCFunction) DataStore_get, 
        METH_VARARGS,
        "Retrieve the (value, timestamp) tuple corresponding to the key. If the key does not exist an exception will be thrown."
    }, 
    
    {   
        "put", 
        (PyCFunction) DataStore_put, 
        METH_VARARGS,
        "Perform a get operation on the current value into and then store the specified value. "
    }, 
    {   
        "get_meta", 
        (PyCFunction) DataStore_get_meta, 
        METH_VARARGS,
        "Retrive a metadata of a corresponding key. If the key does not exist an exception will be thrown."
    }, 
    
    {   
        "put_meta", 
        (PyCFunction) DataStore_put_meta, 
        METH_VARARGS,
        "Set the metadata value of a corresponding key."
    }, 
    
    {   
        "timestamp", 
        (PyCFunction) DataStore_timestamp, 
        METH_VARARGS,
        "Get the timestamp of a key. If the key doesn't exit it returns -1."
    }, 

    {   
        "last_timestamp", 
        (PyCFunction) DataStore_last_timestamp, 
        METH_VARARGS,
        "Get the latest timestamp value."
    }, 

    {   
        "first_timestamp", 
        (PyCFunction) DataStore_first_timestamp, 
        METH_VARARGS,
        "Set the oldest timtstamp value."
    }, 
    
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
    dsserver *server_p;
    PyObject *message_callback;
} DataStoreServer;


static PyObject *DataStoreServer_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int DataStoreServer_init(DataStoreServer *self, PyObject *args, PyObject *kwds);
static void DataStoreServer_dealloc(DataStoreServer* self);
static PyObject* DataStoreServer_serve(DataStoreServer *self, PyObject *args);
static PyObject* DataStoreServer_stop(DataStoreServer *self, PyObject *args);
static PyObject* DataStoreServer_running(DataStoreServer *self, PyObject *args);

static PyMethodDef DataStoreServer_methods[] = {    
    {   
        "serve", 
        (PyCFunction) DataStoreServer_serve, 
        METH_NOARGS,
        "Starting serving clients"
    },
    {   
        "stop", 
        (PyCFunction) DataStoreServer_stop, 
        METH_NOARGS,
        "Stop serving and shutdown"
    },
    {   
        "running", 
        (PyCFunction) DataStoreServer_running, 
        METH_NOARGS,
        "Returns true if the server is running"
    }, 
    {NULL, NULL, 0, NULL}
};

static PyTypeObject DataStoreServerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "kvs.DataStoreServer",                  /* tp_name */
    sizeof(DataStoreServer),                /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)DataStoreServer_dealloc,    /* tp_dealloc */
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
    "DataStore server",         /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    DataStoreServer_methods,    /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)DataStoreServer_init,  /* tp_init */
    0,                          /* tp_alloc */
     DataStoreServer_new,        /* tp_new */
};


typedef struct {
    PyObject_HEAD    
    dsclient *client_p;
    PyObject *frame_callback;
} DataStoreClient;


static PyObject *DataStoreClient_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int DataStoreClient_init(DataStoreClient *self, PyObject *args, PyObject *kwds);
static void DataStoreClient_dealloc(DataStoreClient* self);
static PyObject* DataStoreClient_get(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_put(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_get_meta(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_put_meta(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_last_timestamp(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_first_timestamp(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_timestamp(DataStoreClient *self, PyObject *args);
static PyObject* DataStoreClient_shutdown_server(DataStoreClient *self, PyObject *args);


static PyMethodDef DataStoreClient_methods[] = {    
    {   
        "get", 
        (PyCFunction) DataStoreClient_get, 
        METH_VARARGS,
        "Retrieve the (value, timestamp) tuple corresponding to the key. If the key does not exist an exception will be thrown."
    }, 
    
    {   
        "put", 
        (PyCFunction) DataStoreClient_put, 
        METH_VARARGS,
        "Perform a get operation on the current value into and then store the specified value. "
    }, 
    {   
        "get_meta", 
        (PyCFunction) DataStoreClient_get_meta, 
        METH_VARARGS,
        "Retrive a metadata of a corresponding key. If the key does not exist an exception will be thrown."
    }, 
    
    {   
        "put_meta", 
        (PyCFunction) DataStoreClient_put_meta, 
        METH_VARARGS,
        "Set the metadata value of a corresponding key."
    }, 
    
    {   
        "timestamp", 
        (PyCFunction) DataStoreClient_timestamp, 
        METH_VARARGS,
        "Get the timestamp of a key. If the key doesn't exit it returns -1."
    }, 

    {   
        "last_timestamp", 
        (PyCFunction) DataStoreClient_last_timestamp, 
        METH_VARARGS,
        "Get the latest timestamp value."
    }, 

    {   
        "first_timestamp", 
        (PyCFunction) DataStoreClient_first_timestamp, 
        METH_VARARGS,
        "Set the oldest timtstamp value."
    }, 

    {   
        "shutdown_server", 
        (PyCFunction) DataStoreClient_shutdown_server, 
        METH_VARARGS,
        "shutdown the remote server."
    }, 

    {NULL, NULL, 0, NULL}
};


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

typedef struct {
    PyObject_HEAD
    Py_ssize_t seq_index, enum_index;
    PyObject *sequence;
} DataStoreResultsState;

static PyObject *DataStoreResults_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static void DataStoreResults_dealloc(DataStoreResultsState *rgstate);
static PyObject *DataStoreResults_next(DataStoreResultsState *rgstate);

PyTypeObject DataStoreResultsType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "DataStoreResults",             /* tp_name */
    sizeof(DataStoreResultsState),            /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)DataStoreResults_dealloc,     /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    (iternextfunc)DataStoreResults_next,      /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    PyType_GenericAlloc,            /* tp_alloc */
    DataStoreResults_new,           /* tp_new */
};

#endif