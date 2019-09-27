#include "kvs739.h"
#include <vector>
//Provide a null-terminated array of server names (similarly to argv[]). 
//Each server name has the format "host:port" and initialize the client code. 
//Returns 0 on success and -1 on failure. 
static PyObject* kvs_init(PyObject *self, PyObject *args) {	

	auto argc = PyTuple_Size(args);
	PyObject *obj;
	if ((argc==1) && (PyArg_ParseTuple(args, "O", &obj))) {
		PyObject *iter = PyObject_GetIter(obj);
		if (!iter) {
    		PyErr_SetString(PyExc_RuntimeError, "Must be a list of string"); 
		}
	
		std::vector<char *> vals;
		while (true) {
			PyObject *next = PyIter_Next(iter);
			if (!next) {
				// nothing left in the iterator
				break;
			}
			if (!PyUnicode_Check(next)) {
	    		PyErr_SetString(PyExc_RuntimeError, "Unexpected data type"); 
			}
			const char *str = PyUnicode_AsUTF8(next);
			vals.push_back((char *) str);					
		}		
		vals.push_back(0);
		kv739_init(vals.data());
	}
	else {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
	}
	Py_RETURN_NONE;
}


//Shutdown the connection to a server and free state.
//After calling this, client code should be able to call kv739_init() again to the same or a different server. 
static PyObject* kvs_shutdown(PyObject *self, PyObject *args) {
	kv739_shutdown();
	Py_RETURN_NONE;
}


//Retrieve the value corresponding to the key. 
//If the key is present, it should return 0 and store the value in the provided string. 
//The string must be at least 1 byte larger than the maximum allowed value. If the key is not present, it should return 1. 
//If there is a failure, it should return -1. 
static PyObject* kvs_get(PyObject *self, PyObject *args) {
	char* key;
	auto argc = PyTuple_Size(args);   	

    if ((argc==1) && PyArg_ParseTuple(args, "s", &key)) {
    	char value[2048] = {0};
		if (kv739_get(key, value)==1) {
			return Py_BuildValue("s", value);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "put value failed");		
		}
    }
    else {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    }

	Py_RETURN_NONE;
}


//Perform a get operation on the current value into old_value and then store the specified value. 
//Should return 0 on success if there is an old value, 1 on success if there was no old value, and -1 on failure. 
//The old_value parameter must be at least one byte larger than the maximum value size. 
static PyObject* kvs_put(PyObject *self, PyObject *args) { 
	char* key;
    char* value;
	auto argc = PyTuple_Size(args);   

    if ((argc==2) && PyArg_ParseTuple(args, "ss", &key, &value)) {
    	char oldval[2048] = {0};
		if (kv739_put(key, value, oldval)==1) {			
			return Py_BuildValue("s", oldval);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "put value failed");
		}
    }	
    else {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    }

	Py_RETURN_NONE;
}



static PyObject *DataStore_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	Py_RETURN_NONE;
}


static int DataStore_init(DataStore *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static void DataStore_dealloc(DataStore* self) {

}


static PyObject *DataStoreClient_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	Py_RETURN_NONE;
}


static int DataStoreClient_init(DataStoreClient *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static void DataStoreClient_dealloc(DataStoreClient* self) {

}


static struct PyModuleDef kvsmodule = {
    PyModuleDef_HEAD_INIT,
    "kvs",
    "A distributed key value store for CS739",
    -1,
    module_methods, NULL, NULL, NULL, NULL
};


PyMODINIT_FUNC PyInit_kvs(void) {
    PyObject* m;
    m = PyModule_Create(&kvsmodule);
    return m;
}