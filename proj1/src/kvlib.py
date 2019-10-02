#! /usr/bin/env python3

"""The CS 739 key-value Python library.

The init, shutdown, put, and get functions are callable from the C interface.
All other functions are internal to the library itself.

"""
from basic_server import Action
import codecs
import json
import random
import socket as s
import subprocess

SERVERS = list()
PIDS = "sek-nick.pids"
NULL = bytes(1)

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

    for server in SERVERS:
        kill(server, 0)
        subprocess.run(["kill", server_pid(server)])

def server_pid(server):
    """Return this server's pid and port from pidfile."""

    global PIDS
    pid, port = 0, 1

    with open(PIDS) as pidfile:
        for line in pidfile:
            line = line.strip().split()

            try:
                port_num = int(line[port])
            except ValueError:
                continue

            if server[1] == port_num:
                return line[pid]

    return 0

def kill(server, forget = 1):
    """Make server ``(host, port)`` unconnectable."""

    global SERVERS
    sock = connect(server)

    if sock:
        sock.send(client_msg(Action.SHUTDOWN))
        sock.shutdown(s.SHUT_RDWR)
        sock.close()

    if forget:
        SERVERS.remove(server)

def start(server):
    """Make server ``(host, port)`` available."""

    global SERVERS
    SERVERS.append(server)

def put(k, v):
    """Insert k,v into the keystore, set old_val to prevous value."""

    try:
        k = str(k)
        v = str(v)
    except ValueError:
        return -1

    if bad_input(k, v):
        return -1

    status, old_val = None, None
    sock = connect()

    if sock:
        sock.send(client_msg(Action.INSERT, k, v))
        status, old_val = receive(sock)

    return status, old_val

def get(k):
    """Return key's current value from the datastore."""

    try:
        k = str(k)
    except ValueError:
        return -1

    status, val = None, None
    sock = connect()

    if sock:
        sock.send(client_msg(Action.GET, k))
        status, val = receive(sock)

        #sock.shutdown(s.SHUT_RDWR)
        sock.close()

    return status, val

def connect(target=None):
    """Connect to the specified server or a random one over the socket.

    Returns the socket if connected.

    """
    global SERVERS
    sock = None

    while not sock and (SERVERS or target):
        server = target or random.choice(SERVERS)
        sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        # sock.settimeout(20) # FIXME remove

        try:
            sock.connect(server)
        except ConnectionRefusedError:
            SERVERS.remove(server)
            #sock.shutdown(s.SHUT_RDWR)
            sock.close()
            sock = None

    return sock

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

    msg_in = bytes()
    data = bytes(1)

    while NULL not in msg_in and data:
        data = sock.recv(2**16)
        msg_in += data

    return "" if not msg_in else Action.unwrap(msg_in)

def client_msg(msg_type, k=None, v=None):
    """Prepare a message of the specified type."""

    return Action.wrap({
        Action.INSERT:    [0, 0, k, v, 0],
        Action.GET:       [2, 0, k, 0, 0],
        Action.SHUTDOWN:  [3, 0, 0, 0, 0],
    }[msg_type])
