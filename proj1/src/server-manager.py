#! /usr/bin/env python3

import datetime
import json
import subprocess
import sys


"""Usage: {} <script-to-run> <start-port:7390> <end-port:7400>

Call the specified script with the list of project server locations.
""" % sys.args[0]


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
servers = range(start, end)
servers_json = json.dumps(servers)
now = datetime.now()
now_json = json.dumps(now)


with open(pids, "w") as pidfile:
    pidfile.write("pid port")

with open(pids, "wa") as pidfile:
    for s in servers:
        proc = subprocess.Popen(["python3", "basic-server.py",
                                 s, now_json, servers_json])

        pidfile.writelines([proc.pid + " " + s])

subprocess.run([script,
                json.dumps(["localhost:" + s for s in servers])])
