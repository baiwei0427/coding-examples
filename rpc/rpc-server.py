import xmlrpclib
import sys
from SimpleXMLRPCServer import SimpleXMLRPCServer

def sendfile(filename, content):
    print 'receive %s' % filename
    with open(filename, "wb") as handle:
        handle.write(content.data)


if len(sys.argv) != 2:
    print 'Usage: %s [port]' % sys.argv[0]
    sys.exit()

server = SimpleXMLRPCServer(("0.0.0.0", int(sys.argv[1])), allow_none=True)
server.register_function(sendfile, 'sendfile')
server.serve_forever()
