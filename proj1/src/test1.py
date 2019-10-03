#! /usr/bin/env python3

from datetime import datetime
import json
import kvlib
import pdb
import sys

old_val = 0

def main(servers):
    kvlib.init(servers)

    print("command: (status) value")
    print("get a: ({}) {}".format(*kvlib.get("a")))
    print("put a: ({}) {}".format(*kvlib.put("a", 3)))
    print("get a: ({}) {}".format(*kvlib.get("a")))

    # ~666/s on my little laptop.
    # puts will be slower: ~2/s, with autocommit enabled
    # puts: ~50/s without autocommit.
    # this system is optimized for gets, 1200:1
    # performance would benefit greatly by switching to udp
    epochTime = lambda: float(datetime.strftime(datetime.now(), "%s.%f"))

    gets = 40000
    puts = 2000

    seconds = 60
    minutes = 2 * seconds
    gets = 650 * minutes
    puts = 45 * minutes

    print(gets, "gets btw timestamps.")
    start = epochTime()
    for i in range(0, gets):
            status, val = kvlib.get("a")
    end = epochTime()
    print(gets, "gets took", end - start, "seconds. gets per second", gets / (end - start))

    print(puts, "puts btw timestamps.")
    start = epochTime()
    for i in range(0, puts):
        status, val = kvlib.put("a", 3)
    end = epochTime()
    print(puts, "puts took", end - start, "seconds. puts per second", puts / (end - start))

if __name__ == "__main__":
    try:
        main(json.loads(sys.argv[1]))
    finally:
        kvlib.shutdown()
