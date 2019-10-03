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
import socket as s
import sys
from withsqlite import sqlite_db


NULL = bytes(1)

@unique
class Action(Enum):
    """The list of available server actions."""

    INSERT = 0
    UPDATE = 1
    GET = 2
    SHUTDOWN = 3

    @staticmethod
    def wrap(msg):
        return bytes(json.dumps(msg), encoding="ascii") + NULL

    @staticmethod
    def unwrap(msg):
        return json.loads(str(msg[:-1], encoding="ascii"))

class KvEntry:
    """A single key-value entry."""

    def __init__(self, k=None, v=None, t=None):
        if not k:
            k = ""
        if not t:
            t = datetime.min

        self.k = k
        self.v = v
        self.t = t

    def __repr__(self):
        return "['{}', '{}', {}]".format(self.k, self.v, self.t)

    def asplode(self):
        return [self.k, self.v, self.t]


class KvServer:
    """The key-value server.

    The server sync protocol is as follows:

    1. Each server keeps track of each other server's last updated time.
    2. When a server receives a new or updated key from the client, it saves
       that update with the current time into the local database.  This is
       automatically the newest value for the data because the project assumes
       only a single client is running at one time.
    3. After saving the value, the server broadcasts it to every other server.
    4. Each other server receives that key, value, and timestamp, storing it if
       the key is new or the timstamp is newer.
    5. The remote server reviews both timestamps it has for the originating
       server (the last update time and current key's time).  If this server has
       any updates newer than the oldest of those times, it'll reply with those
       k,v,t triplets.  It then stores the latest timestamp as the originating
       server's current timestamp.
    6. The originating server then saves the keys it received that are new or
       newer, updates its clock for the remote server to the latest key it
       received, and backs up its own clock to the earliest update it didn't
       have.
    7. All servers then wait for a new connection.

    """
    servers = list()

    def __init__(self, me, allServers, start):
        self.id = str(me)
        self.kvs = dict() # kvs = [k1: KvE1, k2: KvE2, ...]
        self.server_updates = defaultdict(int) # su = [srv: timestamp, s: t, ...]
        self.live = 1
        self.sock = 0 # configured in self.serve()

        self.db = sqlite_db(self.id, autocommit = False)
        self.db.begin()

        try:
            self.kvs = { x: KvEntry(*y) for x, y in self.db["kvs"].items() }
        except KeyError:
            self.kvs = dict()

        for port in allServers:
            try:
                self.server_updates[str(port)] = float(self.db["servers"][str(port)])
            except KeyError:
                self.server_updates[str(port)] = float(start)


    def __repr__(self):
        return ("{}: {} {}\n    {}".format(
            self.id, "1" if self.live else "0",
            [(k, self.kvs[k]) for k in self.kvs],
            "\n    ".join([
                "{}: {}".format(k, self.server_updates[k])
                for k in self.server_updates])))

    def shutdown(self):
        """Save database, close socket, mark self as ded so we stop serving."""

        self.save_db()
        self.db.save()
        self.sock.shutdown(s.SHUT_RDWR)
        self.sock.close()
        self.live = 0

    def save_db(self):
        """Save state to db."""

        self.db["servers"] = self.server_updates
        self.db["kvs"] = { x: y.asplode() for x, y in self.kvs.items() }

    def insert(self, key, value, time=None):
        """Insert new value into the kv-store."""

        status = 0
        old_value = None

        if not time: # handles both None and 0 times (from kvlib)
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
            value = self.kvs[k].v
            status = 0
        except KeyError:
            value = ""
            status = 1

        return status, value

    def sync_bidi(self, k, v, t):
        """Sync new key to other servers, update self with their new values."""

        for server in self.server_updates.keys():
            if server != self.id:
                self.update(server, self.send_kv(server, k, v, t))

    def update(self, server, replay):
        """This server received zero or more update keys in reply.

        Play those updates forward without syncing them.  The remote server is
        as updated as the most recent key they sent us.

        """
        newer_keys = list()

        for kve in replay:
            kve = KvEntry(*kve)
            if kve not in self.kvs or self.kvs[kve.k].t <= kve.t:
                self.kvs[kve.k] = kve
                newer_keys.append(kve)

        try:
            self.server_updates[server] = max([x[2] for x in replay])
        except ValueError:
            # nothing to replay, so the server's update time is already caught up.
            pass

        # Oh shoot, the remote sent us back updates that were older than we are
        # right now.  That means we were out of date, so roll back our clock.
        self.server_updates[self.id] = (min([x.t for x in newer_keys] +
                                            [self.server_updates[self.id]]))

    def send_kv(self, server, k, v, t):
        """Open connection to remote servers, exchange messages.

        Returns list of updates to apply.

        """
        # open connection
        sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        try:
            sock.connect(("localhost", int(server)))
            sock.settimeout(20) # FIXME remove
        except ConnectionRefusedError:
            # server down
            return []

        # send message
        msg_out = Action.wrap([Action.UPDATE.value, self.id, k, v, t])
        messlen = sock.send(msg_out)

        # check for obvious errors
        if messlen != len(msg_out):
            return []

        # reveice reply
        msg_in, data = bytes(), bytes(1)
        while NULL not in msg_in and data:
            data = sock.recv(2**16)
            msg_in += data

        # clean up connection
        sock.shutdown(s.SHUT_RDWR)
        sock.close()

        return Action.unwrap(msg_in)

    def serve(self):
        """Serve until killed."""

        self.sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        self.sock.bind(('', int(self.id)))
        self.sock.listen(10)
        self.sock.settimeout(10)
        while self.live:
            try:
                newsock = self.sock.accept()[0]
            except s.timeout:
                self.shutdown()
            else:
                self.handle_client(newsock)

    def handle_client(self, sock):
        """Handle boxing and unboxing of the remote request."""

        value = ""
        status = 0

        # receive the data
        received, msg_in = 1, bytes()
        while NULL not in msg_in:
            data = sock.recv(2**16)
            msg_in += data

        if msg_in:
            request = Action.unwrap(msg_in)
            xmit = lambda x: sock.sendall(Action.wrap(x))
            action = Action(request[0])

            if action == Action.GET:
                xmit(self.get(request[2]))
            elif action == Action.INSERT:
                xmit(self.insert(request[2], request[3])),
            elif action == Action.UPDATE:
                xmit(self.receive(*request[1:])),
            elif action == Action.SHUTDOWN:
                self.shutdown()
            else:
                raise RuntimeError("Unrecognized action: {}".format(request[0]))

    def receive(self, sender, k, v, t):
        """A remote server sent us an update.

        Find the oldest key we think they have and send them keys that are newer."""

        # update our key if necessary
        if (k not in self.kvs or self.kvs[k].t < t):
            self.kvs[k] = KvEntry(k, v, t)

        # we need to play the updates forward from the oldest remote server time.
        playback = min(t, self.server_updates[sender])

        # these are the kves newer than the remote server
        replay = [self.kvs[k].asplode() for k in self.kvs if self.kvs[k].t > playback]

        # the remote server is now as updated as the most recent update we sent
        # it, or it sent us, whichever is newer
        self.server_updates[sender] = max([x[2] for x in replay] + [t])

        self.save_db()

        return replay


def main(myport, start, server_ports):
    """Load server list and start serving."""

    if int(myport) not in server_ports:
        raise RuntimeError(
            "Server port {} must be included in list of available servers: {}".format(
                myport, server_ports))

    myserver = KvServer(myport, server_ports, start)
    myserver.serve()

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], json.loads(sys.argv[3]))
