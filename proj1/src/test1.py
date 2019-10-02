#! /usr/bin/env python3

from datetime import datetime
import json
import kvlib
import pdb
import sys

old_val = 0

def main(servers):
    kvlib.init(servers)

    print("command: value (status)")

    status, val = kvlib.put("a", 3)
    print("put a: {} ({})".format(val, status))

    status, val = kvlib.get("a")
    print("get a: {} ({})".format(val, status))

    # ~666/s on my little laptop.
    # puts will be slower: ~2/s, with autocommit enabled
    # puts: ~50/s without autocommit.
    # this system is optimized for gets, 1200:1
    # performance would benefit greatly by switching to udp
    print("10 puts, 200 gets per timestamp.")
    for i in range(1, 10):

        for j in range(1, 200):
            status, val = kvlib.get("a")

        for j in range(1, 10):
            status, val = kvlib.put("a", 3)

        print(datetime.now())

if __name__ == "__main__":
    try:
        main(json.loads(sys.argv[1]))
    finally:
        kvlib.shutdown()
