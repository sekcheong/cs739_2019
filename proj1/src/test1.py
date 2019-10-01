#! /usr/bin/env python3

import json
import kvlib
import pdb
import sys

old_val = 0

def main(servers):
    kvlib.init(servers)

    status, val = kvlib.put("a", 12)
    print("put a: {} ({})".format(val, status))

    status, val = kvlib.get("a")
    print("get a: {} ({})".format(val, status))

    kvlib.shutdown()

if __name__ == "__main__":
    try:
        main(json.loads(sys.argv[1]))
    finally:
        kvlib.shutdown()
