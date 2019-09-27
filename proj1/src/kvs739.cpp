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
    		return NULL;
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

    if ((argc!=1) || !PyArg_ParseTuple(args, "s", &key)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
    }

	char value[2048] = {0};
	if (kv739_get(key, value)==1) {
		return Py_BuildValue("s", value);
	}
	else {
		PyErr_SetString(PyExc_RuntimeError, "put value failed");		
		return NULL;
	}

}


//Perform a get operation on the current value into old_value and then store the specified value. 
//Should return 0 on success if there is an old value, 1 on success if there was no old value, and -1 on failure. 
//The old_value parameter must be at least one byte larger than the maximum value size. 
static PyObject* kvs_put(PyObject *self, PyObject *args) { 
	char* key;
    char* value;
	auto argc = PyTuple_Size(args);   

    if ((argc!=2) || !PyArg_ParseTuple(args, "ss", &key, &value)) {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    	return NULL;
    }

	char oldval[MAX_VALUE_SIZE] = {0};
	
	if (kv739_put(key, value, oldval)==1) {			
		return Py_BuildValue("s", oldval);
	}
	else {
		PyErr_SetString(PyExc_RuntimeError, "put value failed");
		return NULL;
	}
}


static PyObject *DataStore_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	DEBUG_PRINT("DataStore_new() [begin]");
    DataStore *self;

    self = (DataStore *)type->tp_alloc(type, 0);
    
    if (self == NULL) {      
        PyErr_SetString(PyExc_RuntimeError, "Error allocating DataStore object");
        return NULL;
    }

    //try to create the camera either by index or info 
    try {
    	char* filename;
		auto argc = PyTuple_Size(args);   	
    	if ((argc==1) && PyArg_ParseTuple(args, "s", &filename)) {
        	self->store_p = new data_store(filename);
    	}
    	else {
    		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    		return NULL;
    	}
    }
    catch (exception &ex) {        
        PyErr_SetString(PyExc_RuntimeError, "Error creating data store.");
        return NULL;
    }

    DEBUG_PRINT("DataStore_new() [end]");
    return (PyObject *)self;
}


static int DataStore_init(DataStore *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static void DataStore_dealloc(DataStore* self) {
 	DEBUG_PRINT("DataStore_dealloc() [begin]");
    if (self->store_p!=nullptr) {
        try {
            delete self->store_p;       
            self->store_p = nullptr;
        }
        catch (exception &ex) {
            PyErr_SetString(PyExc_RuntimeError, "Error deallocating datastore.");
            return;
        }
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
    DEBUG_PRINT("DataStore_dealloc() [end]");    
}


static PyObject* DataStore_get(DataStore *self, PyObject *args) {
	char* key;
   
	auto argc = PyTuple_Size(args);   

    if ((argc!=1)  || !PyArg_ParseTuple(args, "s", &key)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
    }

	char buffer[MAX_VALUE_SIZE] = {0};
	int64_t ts = 0;
	int len = MAX_VALUE_SIZE-1;
	data_store *ds = self->store_p;

	try {
		if (ds->get(key, buffer, &len,  &ts) ==1) {
			buffer[len]=0;
			return Py_BuildValue("(sL)", buffer, ts);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "key doesn't exist");
			return NULL;
		}
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
	}

    Py_RETURN_NONE;
}


static PyObject* DataStore_put_private(DataStore *self, char *key, char *value) {
	char oldval[MAX_VALUE_SIZE] = {0};
	int64_t ts = 0;
	int len = MAX_VALUE_SIZE-1;

	data_store *ds = self->store_p;

	try {
		if (ds->put(key, value, strlen(value), oldval, &len, &ts) ==1) {		
			if (ts>0) {
				oldval[len] = 0;
			}
			else {
				oldval[0] = 0;
			}
			return Py_BuildValue("(sL)", oldval, ts);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "put value failed");
			return NULL;
		}
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStore_put(DataStore *self, PyObject *args) {
	char buffer[128];
	char* key;
	char* value;
	int i;
	long l;
	int64_t l64;
	float f;
	double d;
	char c;

	auto argc = PyTuple_Size(args);   

	if (argc!=2) {
		PyErr_SetString(PyExc_RuntimeError, "insufficient number of parameters"); 
		return NULL;
	}

	if (PyArg_ParseTuple(args, "ss", &key, &value)) {
		return DataStore_put_private(self, key, value);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "si", &key, &i)) {
		sprintf(buffer, "%d", i);
		return DataStore_put_private(self, key, buffer);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "sl", &key, &l)) {
		sprintf(buffer, "%ld", (long) l);
		return DataStore_put_private(self, key, buffer);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "sL", &key, &l64)) {
		sprintf(buffer, "%" PRId64, l64);
		return DataStore_put_private(self, key, buffer);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "sf", &key, &f)) {
		sprintf(buffer, "%f", f);
		return DataStore_put_private(self, key, buffer);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "sd", &key, &d)) {
		sprintf(buffer, "%f", d);
		return DataStore_put_private(self, key, buffer);
	}

	PyErr_Clear();
	if (PyArg_ParseTuple(args, "sc", &key, &c)) {
		sprintf(buffer, "%c", c);
		return DataStore_put_private(self, key, buffer);
	}
	else {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}
}


static PyObject* DataStore_get_meta(DataStore *self, PyObject *args) {
	char* key;
	char value[MAX_VALUE_SIZE];
	int len = MAX_VALUE_SIZE - 1;

	if (PyTuple_Size(args)!=1|| !PyArg_ParseTuple(args, "s", &key, &value)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		data_store *ds = self->store_p;
		if (ds->get_meta(key, value, &len)==1) {		
			return Py_BuildValue("s", value);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "key does not exit");
			return NULL;
		}
	}
	catch (exception &ex)  {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}

	return NULL;
}


static PyObject* DataStore_put_meta(DataStore *self, PyObject *args) {
	char* key;
	char* value;

	if (PyTuple_Size(args)!=2|| !PyArg_ParseTuple(args, "ss", &key, &value)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		data_store *ds = self->store_p;
		if (ds->put_meta(key, value)==1) {		
			Py_RETURN_NONE;
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "put value failed!");
			return NULL;
		}
	}
	catch (exception &ex)  {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}

}


static PyObject* DataStore_last_timestamp(DataStore *self, PyObject *args) {
	return Py_BuildValue("L", 0);
}


static PyObject* DataStore_first_timestamp(DataStore *self, PyObject *args) {
	return Py_BuildValue("L", 0);
}


static PyObject *DataStoreServer_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	DEBUG_PRINT("DataStoreServer_new() [begin]");
    // DataStoreServer *self;

    // self = (DataStoreServer *)type->tp_alloc(type, 0);
    // if (self == NULL) {      
    //     PyErr_SetString(PyExc_RuntimeError, "Error allocating DataStoreServer object");
    //     return NULL;
    // }

    // //try to create the camera either by index or info 
    // try {
    // 	char* host;
    // 	int port;
    // 	if (PyArg_ParseTuple(args, "si", &host, &port)) {
    //     	self->store_p = new data_store(filename);
    // 	}
    // 	else {
    // 		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    // 	}
    // }
    // catch (exception &ex) {        
    //     PyErr_SetString(PyExc_RuntimeError, "Error creating data store.");
    //     return NULL;
    // }

    DEBUG_PRINT("DataStoreServer_new() [end]");
    Py_RETURN_NONE;
    //return (PyObject *)self;
}


static int DataStoreServer_init(DataStoreServer *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static void DataStoreServer_dealloc(DataStoreServer* self) {


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
    PyObject* module = PyModule_Create(&kvsmodule);

    if (!module) return NULL;
    if (PyType_Ready(&DataStoreType) < 0) return NULL;
    Py_INCREF(&DataStoreType);
    PyModule_AddObject(module, "DataStore", (PyObject *)&DataStoreType);

    return module;
}