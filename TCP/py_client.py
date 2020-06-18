from socket import *
import struct
try:
    import cPickle as pickle
except:
    import pickle

HOST='localhost'
PORT=2727
BUFSIZE=1024
ADDR=(HOST, PORT)

tcpCliSock=socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect(ADDR)

class Test(object):

    def __init__(self, n, a, w):
        self.name=n
        self.age=a
        self.wife=w

while True:
    data = raw_input(">:")
    if not data:
        break
    data = Test("ht", 26, "wife")
    #tcpCliSock.send(pickle.dumps(data))
    tcpCliSock.send(struct.pack("2si4s",data.name, data.age, data.wife))
    data=tcpCliSock.recv(BUFSIZE)
    if not data:
        break
    print data
    print type(data)

tcpCliSock.close()
