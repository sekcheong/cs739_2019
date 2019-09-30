#! /usr/bin/env python3

import json
import kvlib
import pdb
import sys

old_val = 0

def main(servers):
    kvlib.init(servers)

    print(kvlib.put("a", 1, old_val))
    print(old_val)
    print(kvlib.get("a", old_val))
    print(old_val)

    pdb.set_trace()

    kvlib.shutdown()

if __name__ == "__main__":
    try:
        main(json.loads(sys.argv[1]))
    finally:
        kvlib.shutdown()
