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

    def __init__(self, k = None, v = None, t = None):
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

    def __init__(self, me, allServers, initInst):
        self.id = str(me)
        self.kvs = dict() # kvs[key] = [(v: value, t: timestamp), (v:v, t:t), ...]
        self.serverUpdateTimes = defaultdict(int) # s[srv] = [(srv, timestamp), (s, t), ...]
        self.live = 1

        for serverPort in allServers:
            self.serverUpdateTimes[str(serverPort)] = initInst

    def __repr__(self):
        return ("{}: {} {}\n    {}".format(
            self.id, "1" if self.live else "0",
            [(k, self.kvs[k]) for k in self.kvs],
            "\n    ".join([
                "{}: {}.{}".format(k,
                                   self.serverUpdateTimes[k].second,
                                   self.serverUpdateTimes[k].microsecond)
             for k in self.serverUpdateTimes])))

    def insert(self, key, value, time = None):
        if not time:
            time = datetime.now()
        self.kvs[key] = KvEntry(key, value, time)
        self.serverUpdateTimes[self.id] = time
        self.syncBidi(key, value, time)

    def syncBidi(self, k, v, t):
        for server in KvServer.servers:
            if server.live and server.id != self.id:
                self.update(server, server.receive(self.id, k, v, t))

    def receive(self, sender, k, v, t):
        """A remote server sent us an update.

        Find the oldest key we think they have and send them keys that are newer."""

        # update our key if necessary
        if (k not in self.kvs or self.kvs[k].t < t):
            self.kvs[k] = KvEntry(k, v, t)

        # we need to play the updates forward from the oldest remote server time.
        playback = min(t, self.serverUpdateTimes[sender])

        # these are the keys newer than the remote server
        replay = [self.kvs[k] for k in self.kvs if self.kvs[k].t > playback]

        # the remote server is now as updated as the most recent update we sent
        # it, or it sent us, whichever is newer
        self.serverUpdateTimes[sender] = max([x.t for x in replay] + [t])

        return replay

    def update(self, server, replay):
        """This server received zero or more update keys in reply.

        Play those updates forward without syncing them.  The remote server is
        as updated as the most recent key they sent us.

        """
        for k in replay:
            if k not in self.kvs or self.kvs[k].t <= k.t:
                self.kvs[k.k] = k

        try:
            self.serverUpdateTimes[server.id] = max([x.t for x in replay])
        except ValueError:
            # nothing to replay, so the server's update time is already caught up.
            pass

        # oh shoot, the remote sent us back updates that were older than we are right now
        # that means we were out of date, so roll back our clock.
        # TODO: verify servers *can* advance in time and be considered caught up.
        self.serverUpdateTimes[self.id] = (min([x.t for x in replay] +
                                               [self.serverUpdateTimes[self.id]]))

def test():

    # set up system
    initInst = datetime.now()
    server_ports = list(range(7390, 7393))
    KvServer.servers = [KvServer(port, server_ports, initInst) for port in server_ports]

    # name our servers
    alice = KvServer.servers[0]
    bob = KvServer.servers[1]
    calvin = KvServer.servers[2]

    # 1. insert a value.

    alice.insert("a", 1)

    print("\n----")
    pprint(KvServer.servers)

    # 2. kill off a server and insert more values

    bob.live = 0
    calvin.insert("a", 2)
    calvin.insert("b", 3)

    print("----")
    pprint(KvServer.servers)

    # 3. revivify the server and watch it update.

    bob.live = 1
    alice.insert("d", 5)

    print("----")
    pprint(KvServer.servers)

    # 4. insert a value into the previously dead server.

    bob.insert("c", 10)
    print("----")
    pprint(KvServer.servers)

    # on my laptop, we can insert about a billion keys a second.
    # that seems really fast.
    import timeit
    print(timeit.timeit(
        lambda: map(bob.insert, range(0, 1000000), range(1000000, 2000000)),
        number=1000000))

    # TODO heartbeat: every second, fire a sync from one server to all the others

if __name__ == "__main__":
    test()
