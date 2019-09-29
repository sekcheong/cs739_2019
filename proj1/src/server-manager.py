#! /usr/bin/env python3

"""Calls the specified test script with the list of project server locations.

Usage:

    $ server-manager.py <script-to-run> <start-port:7390> <end-port:7400>

This program calls the test script like so:

    $ test1.py '["localhost:7390", "localhost:7391"]'

To parse the list of servers, the test script should call:

    import json, kvlib, sys
    servers = json.loads(sys.argv[1])

The test script can then process the test as normal:

    kvlib.init(servers)

    ret_val = kvlib.put("key", 1, old_val)
    ret_val = kvlib.get("key", old_val)

    server4 = servers[4]
    kvlib.kill(server4)

    ret_val = kvlib.put("key", 2, old_val)
    kvlib.start(server4)

    ret_val = kvlib.get("key", old_val)
    kvlib.shutdown()

The test script must hold a reference to any servers it wants to later
restart.  The library discards unreachable servers during a run.

To restart testing from a clean slate, you'll also need to delete any
stale .sqlite files, which are tied to the host's port.

"""

from datetime import datetime
import json
import pdb
import subprocess
import sys

# FIXME remove
sys.argv.append("test1.py")


if len(sys.argv) < 2:
    print(__doc__)
    exit(1)


try:
    start = sys.argv[2]
except IndexError:
    start = 7390

try:
    end = sys.argv[3]
except IndexError:
    end = 7400


script = sys.argv[1]
pids = "sek-nick.pids"
servers = [x for x in range(start, end)]
servers_json = json.dumps(servers)
now = datetime.strftime(datetime.now(), "%s.%f")
pdb.set_trace()

now_json = json.dumps(now)


with open(pids, "w") as pidfile:
    pidfile.write("# pid host:port")

    for s in servers:
        s = str(s)
        proc = subprocess.Popen(["python3", "basic-server.py",
                                 s, now_json, servers_json])

        pidfile.write("{} {}\n".format(proc.pid, s))

# FIXME uncomment
# subprocess.run([script,
#                 json.dumps(["localhost:" + s for s in servers])])
