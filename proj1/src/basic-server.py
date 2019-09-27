#! /usr/bin/env python3

"""A simple example of the server sync protocol.

k: key
v: value
t: timestamp
"""


import string
from collections import defaultdict
from datetime import datetime
from pprint import pprint


class KvEntry:

    def __init__(self, v = None, t = None):
        self.v = v
        self.t = datetime.min

    def __repr__(self):
        return "{} @ {}.{}".format(self.v, self.t.second, self.t.microsecond)


class KvServer:

    servers = list()

    def __init__(self, me, allServers, initInst):
        self.me = str(me)
        self.kvs = defaultdict(KvEntry) # kvs[key] = [(v: value, t: timestamp), (v:v, t:t), ...]
        self.serverUpdateTimes = defaultdict(int) # s[srv] = [(srv, timestamp), (s, t), ...]
        self.live = 1

        for serverPort in allServers:
            self.serverUpdateTimes[str(serverPort)] = initInst

    def __str__(self):
        return "port {} knows {}".format(self.me, self.serverUpdateTimes.keys())

    def __repr__(self):
        return ("{}: {} {}\n    {}".format(
            self.me, "1" if self.live else "0",
            ["{}: {}".format(k, self.kvs[k]) for k in self.kvs],
            "\n    ".join([
                "{}: {}.{}".format(k,
                                   self.serverUpdateTimes[k].second,
                                   self.serverUpdateTimes[k].microsecond)
             for k in self.serverUpdateTimes])))

    def insert(self, key, value, time = None):
        if not time:
            time = datetime.now()
        self.kvs[key] = KvEntry(value, time)
        self.serverUpdateTimes[self.me] = time
        self.syncBidi(key, value, time)

    def syncBidi(self, k, v, t):
        for server in KvServer.servers:
            if server.live:
                self.update(server, server.receive(self.me, k, v, t))

    def receive(self, sender, k, v, t):
        """A remote server sent us an update.

        Find the oldest key we think they have and send them keys that are newer."""

        # update our key if necessary
        if (self.kvs[k].t < t):
            self.kvs[k].v = v
            self.kvs[k].t = t

        # we need to play the updates forward from the oldest remote server time.
        playback = t if t < self.serverUpdateTimes[sender] else self.serverUpdateTimes[sender]

        #import pdb; pdb.set_trace()
        # these are the keys newer than the remote server
        replay = [self.kvs[k] for k in self.kvs if self.kvs[k].t > t]

        # the remote server is now as updated as the most recent update we sent
        # it, or it sent us, whichever is newer
        self.serverUpdateTimes[sender] = max([t for replay.t in replay] + [t])

        return replay

    def update(self, server, replay):
        """This server received zero or more update keys in reply.

        Play those updates forward without syncing them.  The remote server is
        as updated as the most recent key they sent us.

        """
        for k, update in replay:
            if self.kvs[k].t <= update.t:
                self.kvs[k] = update

        try:
            self.serverUpdateTimes[server] = max([t for replay.t in replay])
        except ValueError:
            # nothing to replay, so the server's update time is already caught up.
            pass

        # oh shoot, the remote sent us back updates that were older than we are right now
        # that means we were out of date, so roll back our clock.
        # TODO: verify servers *can* advance in time and be considered caught up.
        self.serverUpdateTimes[self.me] = (min([t for replay.t in replay] +
                                          [self.serverUpdateTimes[self.me]]))

initInst = datetime.now()
server_ports = list(range(7390, 7393))
KvServer.servers = [KvServer(port, server_ports, initInst) for port in server_ports]



alice = KvServer.servers[0]
bob = KvServer.servers[1]
calvin = KvServer.servers[2]

alice.insert("a", 1)

print("\n----")
pprint(KvServer.servers)

bob.live = 0

calvin.insert("a", 2)
calvin.insert("b", 3)

print("----")
pprint(KvServer.servers)

bob.live = 1
bob.insert("c", 10)

print("----")
pprint(KvServer.servers)

alice.insert("d", 5)
print("----")
pprint(KvServer.servers)

# TODO heartbeat: every second, fire a sync from one server to all the others
