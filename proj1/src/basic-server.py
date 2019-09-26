#! /usr/bin/env python3

"""A simple example of the server sync protocol.

k: key
v: value
t: timestamp
"""


from collections import defaultdict
from datetime import datetime
from pprint import pprint


class KvEntry:

    def __init__(self, v, t):
        self.v = v
        self.t = t


class KvServer:

    def __init__(self, me, allServers, initInst):
        self.me = str(me)
        self.kvs = defaultdict(KvEntry) # kvs[key] = [(v: value, t: timestamp), (v:v, t:t), ...]
        self.servers = defaultdict(int) # s[srv] = [(srv, timestamp), (s, t), ...]
        self.live = 1

        for serverPort in allServers:
            self.servers[str(serverPort)] = initInst

    def __str__(self):
        return "port {} knows {}".format(self.me, self.servers.keys())

    def __repr__(self):
        return str(self.me) + ":\n" + "\n".join(["    {}: {}:{}.{}".format(
            k, self.servers[k].minute, self.servers[k].second, self.servers[k].microsecond)
            for k in self.servers])

    def insert(self, key, value, time = None):
        if not time:
            time = datetime.now()
        self.kvs[key] = KvEntry(value, time)
        self.servers[self.me] = time
        self.syncBidi(key, value, time)

    def syncBidi(self, k, v, t):
        for server in servers:
            if self.servers[sever].live:
                self.update(server, server.receive(self.me, k, v, t))

    def receive(sender, k, v, t):
        """A remote server sent us an update.

        Find the oldest key we think they have and send them keys that are newer."""

        if (self.kvs[k].t < t):
            self.kvs[t].v = v
            self.kvs[t].t = t

        # we need to play the updates forward from the oldest remote server time.
        playback = t if t > self.servers[sender] else self.servers[sender]

        # these are the keys newer than the remote server
        replay = [k for k in self.kvs if k.t > t]

        # the remote server is now as updated as the most recent update we sent it, or it sent us
        self.servers[sender] = max([t for replay.t in replay] + KvEntry(k, v, t))

        return replay

    def update(self, server, replay):
        """This server received zero or more update keys in reply.

        Play those updates forward without syncing them.  The remote server is
        as updated as the most recent key they sent us.

        """
        for k, update in replay:
            if self.kvs[k].t <= update.t:
                self.kvs[k] = update

        self.servers[server] = max([t for replay.t in replay]) or self.servers[server]

        # oh shoot, the remote sent us back updates that were older than we are right now
        # that means we were out of date, so roll back our clock.
        # TODO: verify servers *can* advance in time and be considered caught up.
        self.servers[me] = min([t for replay.t in replay] + self.servers[self.me])

initInst = datetime.now()
server_ports = list(range(7390, 7400))
servers = [KvServer(port, server_ports, initInst) for port in server_ports]
pprint(servers)


# TODO heartbeat: every second, fire a sync from one server to all the others
