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
import subprocess
import sys

def main(script, start, end):
    """Start servers in start:end range, run test script."""

    pids = "sek-nick.pids"
    now = float(datetime.strftime(datetime.now(), "%s.%f"))
    servers = [x for x in range(start, end)]

    with open(pids, "w") as pidfile:
        pidfile.write("# pid host:port\n")

        for svr in servers:
            svr = str(svr)
            proc = subprocess.Popen(
                ["python3", "basic-server.py",
                 svr, json.dumps(now), json.dumps(servers)])

            pidfile.write("{} {}\n".format(proc.pid, svr))

    subprocess.run(
        [script, json.dumps(["localhost:" + str(svr) for svr in servers])])

if "__main__" in __name__:
    # fixme remove
    if len(sys.argv) == 1:
        sys.argv.append("./test1.py")

    if len(sys.argv) < 2:
        print(__doc__)
        exit(1)

    SCRIPT = sys.argv[1]

    try:
        START = sys.argv[2]
    except IndexError:
        START = 7390

    try:
        END = sys.argv[3]
    except IndexError:
        END = 7400

    main(SCRIPT, START, END)