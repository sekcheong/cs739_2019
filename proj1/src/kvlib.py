import json
from random import choice
import socket as s
import sys
import subprocess

servers = list()


def init(svr_list):
    servers = [x.split(":") for x in svr_list]
    # TODO add get return values
    # TODO add put return values
    # TODO implement shutdown

def shutdown():
    for s in servers:
        subprocess.run("kill ")

def kill(server):
    """Make server ``(host, port)`` unconnectable."""

    servers.remove(server)

def raise(server):
    """Make server ``(host, port)`` available."""

    servers.append(server)

def connected():
    while not connected and servers:
        s = random.choice(servers)

        try:
            connected = sock.connect(s)
        except ConnectionRefusedError:
            servers.remove(s)

def put(k, v):
    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected():
        sock.send(client_put(k, v))
        sock.close()

def get(k):
    sock = s.socket(s.AF_INET, s.SOCK_STREAM)

    if connected():
        msg_out = client_get(k)
        messlen = sock.send(msg_out)

        sock.close()

def client_put(k, v):
    return json.loads([0, 0, k, v, 0] )

def client_get(k):
    return json.loads([0, 0, k, 0, 0] )
