import xmlrpclib
import sys

if len(sys.argv) != 4:
    print 'Usage: %s [IP address] [port] [file]' % sys.argv[0]
    sys.exit()
    
ip = sys.argv[1]
port = sys.argv[2]
filename = sys.argv[3]

proxy = xmlrpclib.ServerProxy("http://" + ip + ":" + port)
with open(filename, "rb") as handle:
    proxy.sendfile(filename, xmlrpclib.Binary(handle.read()))
