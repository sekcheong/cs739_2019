#! /usr/bin/env python3
import sys
import kvs

def on_init(servers):
	print("on_init()")
	for s in servers:
	 	print ("   ", s)
	return 1

def on_put(key, value):
	print("on_put()")
	print("  key   : ",key)
	print("  value : ", value)
	#returns the old value
	return "wufoo"

def on_get(key):
	print("on_get()")
	print("  key   : ",key)
	#returns the value
	raise NameError("Key doesn't exist")
	return "meow"

def on_shutdown():
	print("on_shutdown()")
	#shutdown server
	return 1


#this is your service entry point which hooks up python code to APIs in 
def main():
	kvs.init_handler(on_init)
	kvs.put_handler(on_put)
	kvs.get_handler(on_get)
	kvs.shutdown_handler(on_shutdown)
	name = input("Press Enter to exit...")  
	print()
	return

if __name__ == "__main__":
    main()