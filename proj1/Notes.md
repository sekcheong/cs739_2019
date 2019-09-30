
# Design

These are components we do or may need:

1. Daemon (Sek) :question:
2. IPC (Sek) :white_check_mark:
3. Backend data store (Sek) :white_check_mark:
4. Server-to-server communication protocol (Nick)
5. Load balancer / gateway
6. Monitoring / failure recovery system

## Daemon
Each daemon holds a distributed KV store. Daemon processors are identical except the database file they use for storing data and the TCP port. A daemon can also spawn a number of daemon processors for each port, file combination, say D[port, file]. One of the daemon needs to be the leader that responsible for:

1. Making sure the keys are replicated among the other daemons (this ensures eventual consistancy)
2. Monitor other daemons and replace one if one fails
3. If the master daemon dies, the peer should elect a master daemon take over its job
4. If a request key doesn't exit from a specific daemon, that daemon should be able to ask the master to see who can has the key. The requestor upon receive the value should populate its own store. The requesting daemon need can simply tell the client key doesn't exist while try to find the key from other daemon and update it in a low priority job.
### Need to have ability to monitor daemon across servers?


## IPC
For the time being we use TCP/IP (might not be the most efficient, but simple enough to implement) for our IPC. We use a messaging based communication. The communication consist of a request [message](https://github.com/sekcheong/cs739_2019/blob/master/proj1/src/message.h) and a response message. A client making a request will always get a response. If client doesn't receive response within certain time limit, one can assume the server is crashed or in an error state. The format of the message as following:

    +-------------+
    | heder       |
    +-------------+
    | payload     |
    +-------------+


                             64-bit
                             +-----------------------------------------------+
                             | id                                            |
                             +-----------------------+-----------------------+
                             | command               | flag                  |
                             +-----------------------+-----------------------+
                             | param                                         |
                             +-----------------------+-----------------------+
                             | key size              | value size            |
                             +-----------------------+-----------------------+
                             | value time stamp                              |
                             +-----------------------------------------------+
    The key segment    ----> | key string                                    |
                             |                                               |
    The value segment  ----> |                                               |
                             .                                               .
                             .                    payload                    .
                             .                                               .
                             |                                               |
                             +-----------------------------------------------+

The message is fixed in 4096 byte and it is composed of a header block and a payload block. We could probably remove the payload block for messages that do not involve reading, updating key/value pairs to save some bandwidth but for simplicity we just keep the payload block.

## Backend Data Store
We use SQLite3 as our backend store. We create one DB file per server and there is only one table in the DB data_store (key TEXT PRIMARY KEY, value BLOB, timestamp INTEGER)