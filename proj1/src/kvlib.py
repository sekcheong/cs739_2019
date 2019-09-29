#! /usr/bin/env python3

import codecs
import json
from random import choice
import socket as s
import sys
import subprocess

servers = list()
pids = "sek-nick.pids"

def init(svr_list):
    servers = [x.split(":") for x in svr_list]
    # TODO add get return values
    # TODO add put return values

def shutdown():
    """Save off and kill all servers."""

    shutdowns = servers
    for server in shutdowns:
        kill(server)
        subprocess.run(["kill", server_pid(server)])

def server_pid(server):
    """Return this server's pid and port from pidfile."""

    pid, port = 0, 1

    with open(pids) as pidfile:
        for line in pidfile:
            line = line.strip().split()

            if (server == line[port]):
                return line[pid]

def kill(server):
    """Make server ``(host, port)`` unconnectable."""

    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected(sock, server):
        sock.send(client_shutdown())
        sock.close()

    servers.remove(server)

def start(server):
    """Make server ``(host, port)`` available."""

    servers.append(server)

def connected(sock, server=None):
    """Coonnect to the specified server or a random one over the socket."""

    connected = 0
    while not connected and (servers or server):
        server = server or random.choice(servers)

        try:
            connected = sock.connect(server)
        except ConnectionRefusedError:
            servers.remove(server)

def put(k, v, old_val):
    """Insert k,v into the keystore, set old_val to prevous value."""

    if bad_input(k, v):
        return -1

    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected(sock):
        sock.send(client_put(k, v))
        status, old_val = receive(sock)
        sock.close()

    return status

def get(k, val):
    """Return key's current value from the datastore."""

    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected(sock):
        sock.send(client_get(k))
        status, val = receive(sock)

        sock.close()

    return status

def bad_input(k, v):
    return bad_key(k) or bad_value(v)

def bad_key(k):
    return len(k) > 128 or bad_contents(k)

def bad_value(v):
    return len(v) > 2048 or bad_contents(v)

def bad_contents(astr):
    return (invalid_encoding(v) or
            "[" in v or
            "]" in v)

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
    received, msg_in = 1, ""
    while received:
        data = sock.recv(32)
        msg_in += data
        received = len(data)

    return json.loads(msg_in)

def client_put(k, v):
    return json.loads([0, 0, k, v, 0] )

def client_get(k):
    return json.loads([2, 0, k, 0, 0] )

def client_shutdown():
    return json.loads([3, 0, 0, 0, 0] )
