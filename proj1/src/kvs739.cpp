#include "kvs739.h"
#include <vector>


static rpc_server *rpc_ = 0;
static PyObject *py_init_callback_ = 0;
static PyObject *py_get_callback_ = 0;
static PyObject *py_put_callback_ = 0;
static PyObject *py_shutdown_callback_ = 0;



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

	char value[MAX_VALUE_SIZE] = {0};
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


static PyObject* kvs_timestamp(PyObject *self, PyObject *args) {
	int64_t ts = data_store::os_timestamp();
	return Py_BuildValue("L", ts);
}



static PyObject* kvs_init_handler(PyObject *self, PyObject *args) {
	DEBUG_PRINT("kvs_init_handler() [begin]");
	PyObject *cb;
    
    if (!PyArg_ParseTuple(args, "O", &cb)) {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    	return NULL;
    }

    if (cb == Py_None) {
    	Py_XDECREF(py_init_callback_);
    	py_init_callback_ = 0;
        Py_RETURN_NONE;
    }
    
    if (!PyCallable_Check(cb)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

	Py_XINCREF(cb); 
	py_init_callback_ = cb;
	DEBUG_PRINT("kvs_init_handler() [end]");
	Py_RETURN_NONE;
}


static PyObject* kvs_put_handler(PyObject *self, PyObject *args) {
	DEBUG_PRINT("kvs_put_handler() [begin]");

	PyObject *cb;
    
    if (!PyArg_ParseTuple(args, "O", &cb)) {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    	return NULL;
    }

    if (cb == Py_None) {
    	Py_XDECREF(py_put_callback_);
    	py_put_callback_ = 0;
        Py_RETURN_NONE;
    }
    
    if (!PyCallable_Check(cb)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

	Py_XINCREF(cb); 
	py_put_callback_ = cb;

	DEBUG_PRINT("kvs_put_handler() [end]");
	Py_RETURN_NONE;
}


static PyObject* kvs_get_handler(PyObject *self, PyObject *args) {
	DEBUG_PRINT("kvs_get_handler() [begin]");
	
	PyObject *cb;
    
    if (!PyArg_ParseTuple(args, "O", &cb)) {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    	return NULL;
    }

    if (cb == Py_None) {
    	Py_XDECREF(py_get_callback_);
    	py_get_callback_ = 0;
        Py_RETURN_NONE;
    }
    
    if (!PyCallable_Check(cb)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

	Py_XINCREF(cb); 
	py_get_callback_ = cb;

	DEBUG_PRINT("kvs_get_handler() [end]");
	Py_RETURN_NONE;
}


static PyObject* kvs_shutdown_handler(PyObject *self, PyObject *args) {
	DEBUG_PRINT("kvs_shutdown_handler() [begin]");
	
	PyObject *cb;
    
    if (!PyArg_ParseTuple(args, "O", &cb)) {
    	PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
    	return NULL;
    }

    if (cb == Py_None) {
    	Py_XDECREF(py_shutdown_callback_);
    	py_shutdown_callback_ = 0;
        Py_RETURN_NONE;
    }
    
    if (!PyCallable_Check(cb)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

	Py_XINCREF(cb); 
	py_shutdown_callback_ = cb;
	
	//https://stackoverflow.com/questions/1796510/accessing-a-python-traceback-from-the-c-api
	
	DEBUG_PRINT("kvs_shutdown_handler() [end]");
	Py_RETURN_NONE;
}


static int DataStore_init(DataStore *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static PyObject* make_server_list(char** servers) {
	DEBUG_PRINT("make_server_list() [begin]");

	if (!servers) {
		Py_RETURN_NONE;
	}

	auto* p = PyList_New(0);
    if (p==NULL) {
        PyErr_SetString(PyExc_RuntimeError, "unable to allocate server list object");   
        Py_RETURN_NONE;
    }

	int i = 0;
	while (servers[i]) {
		DEBUG_PRINT("make_server_list() %s", servers[i]);
		auto *v = Py_BuildValue("s", servers[i]);
		PyList_Append(p, v);
		i++;
	}

	DEBUG_PRINT("make_server_list() [end]");
	return p;

}


static int init_callback(char** server_list) { 
	DEBUG_PRINT("init_callback() [begin]");

	if (!py_init_callback_) return -1;
    
    //If you are getting called back from another non-Python created 
    //thread (i.e. a C/C++ thread receiving data on a socket), then 
    //you MUST acquire Python's Global Interpreter Lock (GIL) before 
    //calling any Python API functions.   
	bool error = false;
	auto *sl = make_server_list(server_list);
	Py_XINCREF(sl); 
	// DEBUG_PRINT("init_callback() 1");
    auto gstate = PyGILState_Ensure(); 
    // DEBUG_PRINT("init_callback() 2");
    auto *arglist = Py_BuildValue("(O)", sl); 
    // DEBUG_PRINT("init_callback() 3");
    auto *result = PyObject_CallObject(py_init_callback_, arglist);
    //auto *result = PyObject_CallObject(py_init_callback_, NULL);
   	// DEBUG_PRINT("init_callback() 6");
    
    if (result==NULL) {
        DEBUG_PRINT("init_callback() failed to invoke the callback function");
        PyErr_Clear();
        error = true;
    }

    Py_DECREF(sl);
    // DEBUG_PRINT("init_callback() 4");

    Py_DECREF(arglist);

    // DEBUG_PRINT("init_callback() 5");
    PyGILState_Release(gstate);

    if (error) return -1;

	// DEBUG_PRINT("init_callback() 7");

    int ret = (int) PyLong_AsLong(result);

	DEBUG_PRINT("init_callback() [end]");  

	return ret;
}


static int get_callback(char* key, char* value) { 
	DEBUG_PRINT("get_callback() [begin]");

	if (!py_get_callback_) return -1;
    
    //If you are getting called back from another non-Python created 
    //thread (i.e. a C/C++ thread receiving data on a socket), then 
    //you MUST acquire Python's Global Interpreter Lock (GIL) before 
    //calling any Python API functions.   

	bool error = false;
    auto gstate = PyGILState_Ensure(); 
    auto *arglist = Py_BuildValue("(s)", key); 

    auto *result = PyObject_CallObject(py_get_callback_, arglist);


 	DEBUG_PRINT("get_callback() 5");
    if (result==NULL) {
        DEBUG_PRINT("get_callback() failed to invoke the callback function");
        PyErr_Clear();
        DEBUG_PRINT("get_callback() 5.5");
        error = true;
    }

    DEBUG_PRINT("get_callback() 4");

    Py_DECREF(arglist);
    PyGILState_Release(gstate);

   if (error) return -1;

   	DEBUG_PRINT("get_callback() [end]");

    const char *p = PyUnicode_AsUTF8(result);
    strcpy(value,p);

    //check if key doesn't exit symbol we return 0
    if (strlen(value)==2) {
    	if (value[0]=='[' && value[1]==']') {
    		return 1;
    	}
    }
	return 0;
}


static int put_callback(char* key, char* value, char* old_value) { 
	DEBUG_PRINT("put_callback() [begin]");

	if (!py_put_callback_) return -1;
    
    //If you are getting called back from another non-Python created 
    //thread (i.e. a C/C++ thread receiving data on a socket), then 
    //you MUST acquire Python's Global Interpreter Lock (GIL) before 
    //calling any Python API functions.   
	bool error = false;
    auto gstate = PyGILState_Ensure(); 
    auto *arglist = Py_BuildValue("(ss)", key, value);

    auto *result = PyObject_CallObject(py_put_callback_, arglist);
    
    if (result==NULL) {
        DEBUG_PRINT("put_callback() failed to invoke the callback function");
        PyErr_Clear();
        error = true;
    }

    Py_DECREF(arglist);
    PyGILState_Release(gstate);

    if (error) return -1;

    const char *p = PyUnicode_AsUTF8(result);
    DEBUG_PRINT("put_callback() [end]");  
    
    strcpy(old_value, p);
    if (strlen(old_value)==2) {
    	if (old_value[0]=='[' && old_value[1]==']') {
    		return 1;
    	}
    }

	return 0;
}


static int shutdown_callback() { 
	DEBUG_PRINT("shutdown_callback() [begin]");

	if (!py_shutdown_callback_) return -1;

	bool error = false;

    auto gstate = PyGILState_Ensure(); 

    auto *result = PyObject_CallObject(py_shutdown_callback_, NULL);
    if (result==NULL) {
        DEBUG_PRINT("shutdown_callback() failed to invoke the callback function");
        PyErr_Clear();
        error = true;
    }

    PyGILState_Release(gstate);

    if (error) return -1;
    
    int ret = (int) PyLong_AsLong(result);

	DEBUG_PRINT("shutdown_callback() [end]");  

	return ret;
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
	
	try {
		auto *ds = self->store_p;
		if (ds->get(key, buffer, &len,  &ts)==1) {
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
		return NULL;
	}

    Py_RETURN_NONE;
}


static PyObject* DataStore_put(DataStore *self, PyObject *args) {
	char* key;
	char* value;
	int64_t ts = 0;

	auto argc = PyTuple_Size(args);   

	if (argc<2) {
		PyErr_SetString(PyExc_RuntimeError, "Insufficient number of arguments"); 
		return NULL;
	}


	if (PyArg_ParseTuple(args, "ssL", &key, &value, &ts)) {
	 	//we have key, value, and time stamp
	}
	else {
		PyErr_Clear();
		ts = 0;
		if (!PyArg_ParseTuple(args, "ss", &key, &value)) {
			PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
			return NULL;
		}
	}


	try {
		data_store *ds = self->store_p;
		if (ds->put(key, value, strlen(value), ts)==1) {		
			Py_RETURN_NONE;
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


static PyObject* DataStore_timestamp(DataStore *self, PyObject *args) {
	char* key;

	if (PyTuple_Size(args)==0) {
		int64_t ts = data_store::os_timestamp();
		return Py_BuildValue("L", ts);
	}

	if (PyTuple_Size(args)!=1 || !PyArg_ParseTuple(args, "s", &key)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		data_store *ds = self->store_p;
		int64_t ts = ds->get_timestamp(key);
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStore_last_timestamp(DataStore *self, PyObject *args) {
	try {
		data_store *ds = self->store_p;
		int64_t ts = ds->get_last_timestamp();
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStore_first_timestamp(DataStore *self, PyObject *args) {
	try {
		data_store *ds = self->store_p;
		int64_t ts = ds->get_first_timestamp();
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static int DataStoreServer_init(DataStoreServer *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static PyObject *DataStoreServer_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	DEBUG_PRINT("DataStoreServer_new() [begin]");
    DataStoreServer *self;

    self = (DataStoreServer*)type->tp_alloc(type, 0);
    
    if (self == NULL) {      
        PyErr_SetString(PyExc_RuntimeError, "Error allocating DataStore object");
        return NULL;
    }

    //try to create the camera either by index or info 
    try {
    	char* host;
    	int port;
    	char* dbfile;

		auto argc = PyTuple_Size(args);   	
    	if ((argc!=3) || !PyArg_ParseTuple(args, "sis", &host, &port, &dbfile)) {
    		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters");
    		return NULL;
    	}
    	self->server_p = new dsserver(host, port, dbfile);
    }
    catch (exception &ex) {        
        PyErr_SetString(PyExc_RuntimeError, "Error creating DataStoreServer.");
        return NULL;
    }

    DEBUG_PRINT("DataStoreServer_new() [end]");
    return (PyObject *)self;
    Py_RETURN_NONE;
    //return (PyObject *)self;
}


static void DataStoreServer_dealloc(DataStoreServer* self) {

 	DEBUG_PRINT("DataStoreServer_dealloc() [begin]");
    if (self->server_p!=nullptr) {
        try {
            delete self->server_p;       
            self->server_p = nullptr;
        }
        catch (exception &ex) {
            PyErr_SetString(PyExc_RuntimeError, "Error deallocating datastore.");
            return;
        }
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
    DEBUG_PRINT("DataStoreServer_dealloc() [end]");  
}


static PyObject* DataStoreServer_serve(DataStoreServer *self, PyObject *args) {
	try {
		dsserver *s = self->server_p;
		s->serve();
		Py_RETURN_NONE;
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStoreServer_stop(DataStoreServer *self, PyObject *args) {
	try {
		dsserver *s = self->server_p;
		s->stop();
		Py_RETURN_NONE;
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStoreServer_running(DataStoreServer *self, PyObject *args) {
	try {
		dsserver *s = self->server_p;
		if (s->is_running()) {
			Py_BuildValue("O", Py_True);
		}
		else {
			Py_BuildValue("O", Py_False);
		}
		Py_RETURN_NONE;
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static int DataStoreClient_init(DataStoreClient *self, PyObject *args, PyObject *kwds) {
	return 0;
}


static PyObject *DataStoreClient_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	DEBUG_PRINT("DataStoreClient_new() [begin]");
    DataStoreClient *self;

    self = (DataStoreClient*)type->tp_alloc(type, 0);
    
    if (self == NULL) {      
        PyErr_SetString(PyExc_RuntimeError, "Error allocating DataStore object");
        return NULL;
    }

    //try to create the camera either by index or info 
    try {
    	char* host;
    	int port;
		auto argc = PyTuple_Size(args);   	
    	if ((argc!=2) || !PyArg_ParseTuple(args, "si", &host, &port)) {
    		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters");
    		return NULL;
    	}
    	self->client_p = new dsclient(host, port);
    }
    catch (exception &ex) {        
        PyErr_SetString(PyExc_RuntimeError, "Error creating DataStoreClient.");
        return NULL;
    }

    DEBUG_PRINT("DataStoreClient_new() [end]");
    return (PyObject *)self;
}


static void DataStoreClient_dealloc(DataStoreClient* self) {
 	DEBUG_PRINT("DataStoreClient_dealloc() [begin]");
    if (self->client_p!=nullptr) {
        try {
            delete self->client_p;       
            self->client_p = nullptr;
        }
        catch (exception &ex) {
            PyErr_SetString(PyExc_RuntimeError, "Error deallocating DataStoreClient.");
            return;
        }
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
    DEBUG_PRINT("DataStoreClient_dealloc() [end]");    
}


static PyObject* DataStoreClient_get(DataStoreClient *self, PyObject *args) {
	char* key;

	auto argc = PyTuple_Size(args);   
    if ((argc!=1)  || !PyArg_ParseTuple(args, "s", &key)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
    }

	char buffer[MAX_VALUE_SIZE] = {0};
	int64_t ts = 0;
	int len = MAX_VALUE_SIZE-1;
	
	try {
		auto *c = self->client_p;
		if (c->get(key, buffer, &len,  &ts)) {
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
		return NULL;
	}

    Py_RETURN_NONE;
}


static PyObject* DataStoreClient_put(DataStoreClient *self, PyObject *args) {
	char* key;
	char* value;
	int64_t ts = 0;

	auto argc = PyTuple_Size(args);   

	if (argc<2) {
		PyErr_SetString(PyExc_RuntimeError, "Insufficient number of arguments"); 
		return NULL;
	}


	if (PyArg_ParseTuple(args, "ssL", &key, &value, &ts)) {
	 	//we have key, value, and time stamp
	}
	else {
		PyErr_Clear();
		ts = 0;
		if (!PyArg_ParseTuple(args, "ss", &key, &value)) {
			PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
			return NULL;
		}
	}

	try {
		auto *c = self->client_p;
		c->put(key, value, strlen(value), ts);		
		Py_RETURN_NONE;
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}

}


static PyObject* DataStoreClient_get_meta(DataStoreClient *self, PyObject *args) {
	char* key;
	char value[MAX_VALUE_SIZE];
	int len = MAX_VALUE_SIZE - 1;

	if (PyTuple_Size(args)!=1|| !PyArg_ParseTuple(args, "s", &key, &value)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		if (c->get_meta(key, value, &len)) {
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

	Py_RETURN_NONE;
}


static PyObject* DataStoreClient_put_meta(DataStoreClient *self, PyObject *args) {
	char* key;
	char* value;

	if (PyTuple_Size(args)!=2|| !PyArg_ParseTuple(args, "ss", &key, &value)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		c->put_meta(key, value);
		Py_RETURN_NONE;
	}
	catch (exception &ex)  {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStoreClient_last_timestamp(DataStoreClient *self, PyObject *args) {
	if (PyTuple_Size(args)!=0) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		int64_t ts = c->get_last_timestamp();
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	};
}


static PyObject* DataStoreClient_first_timestamp(DataStoreClient *self, PyObject *args) {
	if (PyTuple_Size(args)!=0) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		int64_t ts = c->get_first_timestamp();
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStoreClient_timestamp(DataStoreClient *self, PyObject *args) {
	char* key;

	if (PyTuple_Size(args)!=1 || !PyArg_ParseTuple(args, "s", &key)) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		int64_t ts = c->get_timestamp(key);
		return Py_BuildValue("L", ts);
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject* DataStoreClient_shutdown_server(DataStoreClient *self, PyObject *args) {
	
	if (PyTuple_Size(args)!=0) {
		PyErr_SetString(PyExc_RuntimeError, "Invalid parameters"); 
		return NULL;
	}

	try {
		auto *c = self->client_p;
		c->shutdown_server();
		Py_RETURN_NONE;		
	}
	catch (exception &ex) {
		PyErr_SetString(PyExc_RuntimeError, ex.what());
		return NULL;
	}
}


static PyObject *DataStoreResults_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {

 	// PyObject *sequence;

  //   if (!PyArg_UnpackTuple(args, "revgen", 1, 1, &sequence))
  //       return NULL;

  //   /* We expect an argument that supports the sequence protocol */
  //   if (!PySequence_Check(sequence)) {
  //       PyErr_SetString(PyExc_TypeError, "revgen() expects a sequence");
  //       return NULL;
  //   }

  //   Py_ssize_t len = PySequence_Length(sequence);
  //   if (len == -1)
  //       return NULL;

  //   /* Create a new RevgenState and initialize its state - pointing to the last
  //    * index in the sequence.
  //   */
    DataStoreResultsState *rgstate = (DataStoreResultsState *)type->tp_alloc(type, 0);
    if (!rgstate) {
        return NULL;
    }


	// cons_cell *cons = PyObject_NEW(cons_cell, &cons_type);
 //    if(cons) {
 //        cons->car = car; Py_INCREF(car);   INCREF when holding a PyObject* 
 //        cons->cdr = cdr; Py_INCREF(cdr);  /* ditto */
 //    }
 //    return cons;
    // Py_INCREF(sequence);
    // rgstate->sequence = sequence;
    // rgstate->seq_index = len - 1;
    // rgstate->enum_index = 0;

    return (PyObject *)rgstate;
	Py_RETURN_NONE;
}


static void DataStoreResults_dealloc(DataStoreResultsState *rgstate) {
	// We need XDECREF here because when the generator is exhausted,
    // rgstate->sequence is cleared with Py_CLEAR which sets it to NULL.
    Py_XDECREF(rgstate->sequence);
    Py_TYPE(rgstate)->tp_free(rgstate);
}


static PyObject *DataStoreResults_next(DataStoreResultsState *rgstate) {
	Py_RETURN_NONE;
}


static struct PyModuleDef kvsmodule = {
    PyModuleDef_HEAD_INIT,
    "kvs",
    "A distributed key value store for CS739",
    -1,
    module_methods, NULL, NULL, NULL, NULL
};


void exit_handler() {
	DEBUG_PRINT("exit_handler() [begin]");
	if (rpc_) {
		delete rpc_;
	}
	DEBUG_PRINT("exit_handler() [end]");
}


PyMODINIT_FUNC PyInit_kvs(void) {
	DEBUG_PRINT("PyInit_kvs() [begin]");
    PyObject* module = PyModule_Create(&kvsmodule);

    if (!module) return NULL;

	if (!rpc_) {
		rpc_ = new rpc_server(RPC_PORT);
		rpc_->serve();
		rpc_->set_init_callback(init_callback);
		rpc_->set_put_callback(put_callback);
		rpc_->set_get_callback(get_callback);
		rpc_->set_shutdown_callback(shutdown_callback);
	}

	Py_AtExit(exit_handler);

    if (PyType_Ready(&DataStoreType) < 0) return NULL;
    Py_INCREF(&DataStoreType);
    PyModule_AddObject(module, "DataStore", (PyObject *)&DataStoreType);

    if (PyType_Ready(&DataStoreServerType) < 0) return NULL;
    Py_INCREF(&DataStoreServerType);
    PyModule_AddObject(module, "DataStoreServer", (PyObject *)&DataStoreServerType);

    if (PyType_Ready(&DataStoreClientType) < 0) return NULL;
    Py_INCREF(&DataStoreClientType);
    PyModule_AddObject(module, "DataStoreClient", (PyObject *)&DataStoreClientType);

    if (PyType_Ready(&DataStoreResultsType) < 0) return NULL;
    Py_INCREF(&DataStoreResultsType);
    PyModule_AddObject(module, "DataStoreResults", (PyObject *)&DataStoreResultsType);

    // Make sure the GIL has been created since we need to acquire it in our
    // callback to safely call into the python application.
    if (! PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    } 
    DEBUG_PRINT("PyInit_kvs() [end]");
    return module;
}
