#! /usr/bin/env python3






def client_put(k, v):
    return json.loads([0, 0, k, v, 0] )

def client_get(k):
    return json.loads([0, 0, k, 0, 0] )

servers = json.loads(sys.argv[1])

sock = s.socket(s.AF_INET, s.SOCK_STREAM)
sock.connect(servers[0].split(":"))
msg_out = client_put("a", 1)
messlen, received = sock.send(msg_out), 0
sock.close()

sock = s.socket(s.AF_INET, s.SOCK_STREAM)
sock.connect(servers[-1].split(":"))
msg_out = client_get("b", 1)
messlen, received = sock.send(msg_out), 0
sock.close()
