import kvs

#create a server
s = kvs.DataStoreServer("localhost",500123,"data.db")
s.serve()


#create a client
c = kvs.DataStoreClient("localhost",500123)

#insert a key
c.put("ABCD","A")

#put value with timestamp
c.put("ECE", "x", 856790740033 )

#get the value timestamp
v, ts = c.get("ABCD")


c.put_meta("ECE","foo")

print(c.get_meta("ECE"))

#get the timestamp on "ECE"
ts = c.timestamp("ECE")
print(ts);

print(c.first_timestamp())

print(c.last_timestamp())
