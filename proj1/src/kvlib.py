#! /usr/bin/env python3

"""The CS 739 key-value Python library.

The init, shutdown, put, and get functions are callable from the C interface.
All other functions are internal to the library itself.

"""
import codecs
import json
import pdb
import random
import socket as s
import subprocess

SERVERS = list()
PIDS = "sek-nick.pids"

def init(svr_list):
    """Set active server list."""

    global SERVERS

    for server in svr_list:
        server = server.split(":")
        server[1] = int(server[1])
        SERVERS.append(tuple(server))

def shutdown():
    """Gracefully kill all servers."""

    global SERVERS
    shutdowns = SERVERS

    for server in shutdowns:
        kill(server)
        subprocess.run(["kill", server_pid(server)])

def server_pid(server):
    """Return this server's pid and port from pidfile."""

    global PIDS
    pid, port = 0, 1

    with open(PIDS) as pidfile:
        for line in pidfile:
            line = line.strip().split()

            if server == line[port]:
                return line[pid]

    return (0, 0)

def kill(server):
    """Make server ``(host, port)`` unconnectable."""

    global SERVERS
    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected(sock, server):
        sock.send(client_shutdown())
        sock.shutdown()
        sock.close()

    SERVERS.remove(server)

def start(server):
    """Make server ``(host, port)`` available."""

    global SERVERS
    SERVERS.append(server)

def connected(sock, target=None):
    """Connect to the specified server or a random one over the socket."""

    global SERVERS
    connected = 0

    while not connected and (SERVERS or target):
        server = target or random.choice(SERVERS)

        try:
            sock.connect(server)
        except ConnectionRefusedError:
            SERVERS.remove(server)
            sock.close()
        else:
            connected = 1

def put(k, v, old_val=None):
    """Insert k,v into the keystore, set old_val to prevous value."""

    try:
        k = str(k)
        v = str(v)
    except ValueError:
        return -1

    if bad_input(k, v):
        return -1

    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    print("client_put(k, v): " + client_put(k, v))
    print("loads(client_put(k, v)): " + str(json.loads(client_put(k, v))))

    status = None
    if connected(sock):
        sock.send(client_put(k, v))
        status, old_val = receive(sock)

    return status

def get(k, val=None):
    """Return key's current value from the datastore."""

    try:
        k = str(k)
    except ValueError:
        return -1

    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    status = None
    if connected(sock):
        sock.send(client_get(k))
        status, val = receive(sock)

        sock.close()

    return status

def bad_input(k, v):
    """Returns true if input is bad."""

    return bad_key(k) or bad_value(v)

def bad_key(k):
    """Returns true if input is bad."""

    return len(k) > 128 or bad_contents(k)

def bad_value(v):
    """Returns true if input is bad."""

    return len(v) > 2048 or bad_contents(v)

def bad_contents(astr):
    """Returns true if input is bad."""

    return (invalid_encoding(astr) or
            "[" in astr or
            "]" in astr)

def invalid_encoding(astr):
    """Return true if string is invalid:

    - The value wasn't printable ascii, or,
    - We could uu-decode the value.

    """
    try:
        abytes = bytes(astr, "ascii")
    except UnicodeEncodeError:
        return 1

    if True in [(x > 126) or (x < 32) for x in abytes]:
        return 1

    try:
        codecs.decode(abytes, "uu")
    except ValueError:
        return 0

    return 1

# message format is a json-wrapped list:
#   0. action (see basic-server.py::Action)
#   1. originating server, always 0 for server manager
#   2. key
#   3. value
#   4. timestamp

def receive(sock):
    """Return the value received from the remote server."""

    received, msg_in = 1, ""

    while received:
        data = sock.recv(32)
        msg_in += data
        received = len(data)

    return json.loads(msg_in)

def client_put(k, v):
    """Prepare a put or insert message."""

    return json.dumps([0, 0, k, v, 0])

def client_get(k):
    """Prepare a get message."""

    return json.dumps([2, 0, k, 0, 0])

def client_shutdown():
    """Prepare a shutdown message."""

    return json.dumps([3, 0, 0, 0, 0])
