#! /usr/bin/env python3

import kvs


servers = [
    "mango:500123",
    "coco:500124"
]

passes = 0
total = 0


def pretty_test(intro, result):
    global passes, total

    print("Testing {}... {}: {}".format(
        intro,
        result,
        "Pass." if result else "Fail!"))

    total += 1
    passes += result and 1

def test_roundtrip_value():
    kvs.init(servers)
    kvs.put("dog","bad")
    kvs.put("dog","good")
    x = kvs.get("dog")
    kvs.shutdown()

    return x == "good"

def test_roundtrip_datastore():
    ds = kvs.DataStore("test.db");
    ds.put("foo","100")
    ds.put("foo","101")
    ds.put("bar","1000")

    return (ds.get("foo")[0] == "101")


if "__main__" in __name__:
    print()
    pretty_test("server roundtrip with update: dog is good",
                test_roundtrip_value())

    pretty_test("datastore roundtrip: foo is 101",
                test_roundtrip_datastore())

    print("{}/{} tests passed. {}".format(passes, total,
                                          ":)" if passes == total else ""))
