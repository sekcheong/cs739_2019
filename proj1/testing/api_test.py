#! /usr/bin/env python3
import sys
import kvs

def on_init(server_list):
	print("on_init()")
	for s in server_list:
		print ("  ", s)
	return 1

def on_put(key, value):
	print("on_put()")
	print("  key   : ",key)
	print("  value : ", value)
	return "old_value"

def on_get(key):
	print("on_get()")
	print("  key   : ",key)
	return "meow"

def on_shutdown():
	print("on_shutdown()")
	return 1

def main():
	kvs.init_handler(on_init)
	name = input("Press Enter to exit...")  
	print()
	return

if __name__ == "__main__":
    main()