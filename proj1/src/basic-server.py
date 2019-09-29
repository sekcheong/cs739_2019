#! /usr/bin/env python3

"""A simple example of the server sync protocol.

k: key
v: value
t: timestamp
"""


from collections import defaultdict
from datetime import datetime
from pprint import pprint


@unique
class Action(Enum):
    INSERT = 0
    UPDATE = 1
    GET = 2


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
        self.kvs = dict() # kvs[key] = [(v: value, t: timestamp), (v:v, t:t), ...]
        self.server_updates = defaultdict(int) # s[srv] = [(srv, timestamp), (s, t), ...]
        self.live = 1

        for port in allServers:
            self.server_updates[str(port)] = start

        # TODO add with_sqlite here for backend.
        # with sqlite as db:
        #     db = self.server_updates

    def __repr__(self):
        return ("{}: {} {}\n    {}".format(
            self.id, "1" if self.live else "0",
            [(k, self.kvs[k]) for k in self.kvs],
            "\n    ".join([
                "{}: {}.{}".format(k,
                                   self.server_updates[k].second,
                                   self.server_updates[k].microsecond)
                for k in self.server_updates])))

    def insert(self, key, value, time=None):
        if not time:
            time = datetime.now()
        self.kvs[key] = KvEntry(key, value, time)
        self.server_updates[self.id] = time
        self.sync_bidi(key, value, time)

        # sync: kvs, server_updates

    def get(k):
        return kvs[k].v

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
        for k in replay:
            if k not in self.kvs or self.kvs[k].t <= k.t:
                self.kvs[k.k] = k

        try:
            self.server_updates[server.id] = max([x.t for x in replay])
        except ValueError:
            # nothing to replay, so the server's update time is already caught up.
            pass

        # oh shoot, the remote sent us back updates that were older than we are right now
        # that means we were out of date, so roll back our clock.
        self.server_updates[self.id] = (min([x.t for x in replay] +
                                            [self.server_updates[self.id]]))

    def send_kv(self, server, k, v, t):
        """Open connection to remote servers, exchange messages."""

        # open connection
        sock = s.socket(s.AF_INET, s.SOCK_STREAM)
        try:
            sock.connect(("localhost", server.id))
        except ConnectionRefusedError:
            # server down
            return []

        # send message
        msg_out = json.dumps([Action.UPDATE, self.id, k, v, t])
        messlen, received = sock.send(msg_out), 1

        # check for obvious errors
        if messlen != len(msg_out):
            raise RuntimError("Failed to send complete message")

        # reveice reply
        while received:
            data = sock.recv(32)
            msgIn += data
            received = len(data)

        # clean up connection
        sock.close()

        return json.loads(message_in)

    def serve(self):
        sock = s.socket(AF_INET, SOCK_STREAM)
        sock.bind(('', int(self.id)))
        sock.listen(10)
        while 1:    # Run until cancelled
            newsock, client_addr = sock.accept()
            handleClient(newsock)


    def handle_client(self, sock):
        """Handle boxing and unboxing the remote request."""

        # receive the data
        received = 1
        while received:
            msg_in += data
            data = sock.revc(32)
            received = len(data)

        request = json.loads(msg_in)

        if (request[0] == Action.UPDATE):
            sock.sendall(json.dumps(self.receive(*request[1:])))
        if (request[0] == Action.GET):
            sock.sendall(json.dumps(self.get(request[1])))

        sock.close()

        # since insert requires no reply, close socket first
        if (request[0] == Action.INSERT):
            self.insert(*request[2:3])

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

        return replay


def main(myport, start, server_ports):
    KvServer.servers = [KvServer(port, server_ports, start) for port in server_ports]

    try:
        me = [s for s in KvServer.servers if s.id == myport][0]
    except IndexError:
        raise RuntimeError(
            "Server port {} must be included in list of available servers: {}".format(
                port, server_ports))

    me.serve()

if __name__ == "__main__":
    import sys

    myport = sys.args[1]
    start = sys.args[2]
    server_ports = sys.args[3]

    main(myport, start, server_ports)
