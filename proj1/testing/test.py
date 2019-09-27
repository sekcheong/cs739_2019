import kvs



print("Test init, put, get:")

servers = [
	"mango:500123",
	"coco:500124"
] 

kvs.init(servers)

value = kvs.get("cat")
print("value:", value)

ov = kvs.put("dog","bad")
print("old value:", ov)

kvs.shutdown()


print("Test DataStore")

ds = kvs.DataStore("test.db");
v = ds.put("foo","100")
print(v);
v = ds.put("foo","101")
print(v);
v = ds.put("bar","1000")
print(v);
v = ds.get("foo")
print(v);