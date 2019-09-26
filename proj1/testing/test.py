import kvs

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