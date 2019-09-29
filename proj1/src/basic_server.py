#! /usr/bin/env python3

"""A simple example of the server sync protocol.

k: key
v: value
t: timestamp
"""


from collections import defaultdict
from datetime import datetime
from enum import Enum, unique
import json
from pprint import pprint
import socket as s
import sys
from withsqlite import sqlite_db


@unique
class Action(Enum):
    INSERT = 0
    UPDATE = 1
    GET = 2
    SHUTDOWN = 3

class KvEntry:
    def __init__(self, k=None, v=None, t=None):
        if not k:
            k = ""
        if not t:
            t = datetime.min

        self.k = k
        self.v = v
        self.t = t

    def __repr__(self):
        return "{} @ {}.{}".format(self.v, self.t.second, self.t.microsecond)


class KvServer:

    servers = list()

    def __init__(self, me, allServers, start):
        self.id = str(me)
        self.kvs = dict() # kvs = [k1: KvE1, k2: KvE2, ...]
        self.server_updates = defaultdict(int) # su = [srv: timestamp, s: t, ...]
        self.live = 1
        self.sock = 0 # configured in self.serve()

        self.db = sqlite_db(self.id)
        self.db.begin()

        try:
            self.kvs = self.db["kvs"]
        except KeyError:
            self.kvs = dict()

        for port in allServers:
            try:
                self.server_updates[str(port)] = self.db["servers"][str(port)]
            except KeyError:
                self.server_updates[str(port)] = start

    def __repr__(self):
        return ("{}: {} {}\n    {}".format(
            self.id, "1" if self.live else "0",
            [(k, self.kvs[k]) for k in self.kvs],
            "\n    ".join([
                "{}: {}.{}".format(k,
                                   self.server_updates[k].second,
                                   self.server_updates[k].microsecond)
                for k in self.server_updates])))

    def shutdown(self):
        """Save database, close socket, mark self as ded so we stop serving."""

        self.save_db()
        self.sock.close()
        self.live = 0

    def save_db(self):
        """Save state to db."""

        self.db["servers"] = self.server_updates
        self.db["kvs"] = self.kvs
        self.db.save()

    def insert(self, key, value, time=None):
        status = 0
        old_value = None

        if not time:
            time = float(datetime.strftime(datetime.now(), "%s.%f"))

        try:
            old_value = self.kvs[key].v
            status = 0
        except KeyError:
            old_value = None
            status = 1

        self.kvs[key] = KvEntry(key, value, time)
        self.server_updates[self.id] = time
        self.save_db() # save before contacting any number of remote servers

        self.sync_bidi(key, value, time)
        self.save_db()

        return status, old_value

    def get(self, k):
        """Return the key's value and whether it exists: (exists?, value)"""

        try:
            value = kvs[k].v
            status = 0
        except KeyError:
            value = ""
            status = 1

        return status, value

    def sync_bidi(self, k, v, t):
        """Sync new key to other servers, update self with their new values."""

        for server in KvServer.servers:
            if server.live and server.id != self.id:
                self.update(server, send_kv(server, k, v, t))

    def update(self, server, replay):
        """This server received zero or more update keys in reply.

        Play those updates forward without syncing them.  The remote server is
        as updated as the most recent key they sent us.

        """
        for kve in replay:
            if kve not in self.kvs or self.kvs[kve.k].t <= kve.t:
                self.kvs[kve.k] = kve

        try:
            self.server_updates[server.id] = max([x.t for x in replay])
        except ValueError:
            # nothing to replay, so the server's update time is already caught up.
            pass

        # Oh shoot, the remote sent us back updates that were older than we are
        # right now.  That means we were out of date, so roll back our clock.
        self.server_updates[self.id] = (min([x.t for x in replay] +
                                            [self.server_updates[self.id]]))

    def send_kv(self, server, k, v, t):
        """Open connection to remote servers, exchange messages.

        Returns list of updates to apply.

        """
        # open connection
        sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        try:
            sock.connect(("localhost", server.id))
        except ConnectionRefusedError:
            # server down
            return []

        # send message
        msg_out = json.dumps([Action.UPDATE, self.id, k, v, t])
        messlen = sock.send(msg_out)

        # check for obvious errors
        if messlen != len(msg_out):
            return []

        # reveice reply
        received, msg_in = 1, ""
        while received:
            data = sock.recv(32)
            msg_in += data
            received = len(data)

        # clean up connection
        sock.close()

        return json.loads(msg_in)

    def serve(self):
        """Serve until killed."""

        self.sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        self.sock.bind(('', int(self.id)))
        self.sock.listen(10)
        while self.live:
            newsock, client_addr = self.sock.accept()
            handle_client(newsock)

    def handle_client(self, sock):
        """Handle boxing and unboxing of the remote request."""

        value = ""
        status = 0

        # receive the data
        received, msg_in = 1, ""
        while received:
            msg_in += data
            data = sock.recv(32)
            received = len(data)

        request = json.loads(msg_in)
        xmit = lambda x: sock.sendall(json.dumps(x))

        if (request[0] == Action.GET):
            status, value = self.get(request[1])
            xmit((status, value))

        if (request[0] == Action.INSERT):
            status, value = self.insert(*request[2:3])
            xmit((status, value))

        if (request[0] == Action.UPDATE):
            xmit(self.receive(*request[1:]))

        if (request[0] == Action.SHUTDOWN):
            # shutdown before closing socket so manager doesn't kill us before
            # we've saved data.
            self.shutdown()

        sock.close()

    def receive(self, sender, k, v, t):
        """A remote server sent us an update.

        Find the oldest key we think they have and send them keys that are newer."""

        # update our key if necessary
        if (k not in self.kvs or self.kvs[k].t < t):
            self.kvs[k] = KvEntry(k, v, t)

        # we need to play the updates forward from the oldest remote server time.
        playback = min(t, self.server_updates[sender])

        # these are the keys newer than the remote server
        replay = [self.kvs[k] for k in self.kvs if self.kvs[k].t > playback]

        # the remote server is now as updated as the most recent update we sent
        # it, or it sent us, whichever is newer
        self.server_updates[sender] = max([x.t for x in replay] + [t])

        self.save_db()

        return replay


def main(myport, start, server_ports):
    KvServer.servers = [KvServer(port, server_ports, start) for port in server_ports]

    try:
        me = [s for s in KvServer.servers if s.id == myport][0]
    except IndexError:
        raise RuntimeError(
            "Server port {} must be included in list of available servers: {}".format(
                myport, server_ports))

    me.serve()

if __name__ == "__main__":
    myport = sys.argv[1]
    start = sys.argv[2]
    server_ports = json.loads(sys.argv[3])

    main(myport, start, server_ports)
