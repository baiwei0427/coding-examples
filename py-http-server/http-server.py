import sys
import socket
import threading
import gzip
import cStringIO

# Web folder
web_folder = ''

def gzip_compress(buf):
	zbuf = cStringIO.StringIO()
	zfile = gzip.GzipFile(mode = 'wb',  fileobj = zbuf, compresslevel = 9)
	zfile.write(buf)
	zfile.close()
	return zbuf.getvalue()

def path_to_type(path):
	# currently, we support 5 types of files: HTML, CSS, JPG, PDF and PPTX
	type = 'text/html'
	if path.endswith('.css'):
        	type = 'text/css'
   	elif path.endswith('.jpg'):
        	type = 'image/jpeg'
    	elif path.endswith('.pdf'):
        	type = 'application/pdf'
    	elif path.endswith('.pptx'):
        	type = 'application/vnd.openxmlformats-officedocument.presentationml.presentation'

    	return type

def http_header(status, type, length, enable_gzip, enable_chunked):
    	header = 'HTTP/1.1 %s\r\nContent-Type: %s\r\nConnection: close\r\n' % (status, type)

    	if enable_gzip:
        	header = header + 'Content-Encoding: gzip\r\n'

    	if enable_chunked:
        	header = header + 'Transfer-Encoding: chunked\r\n'
    	else:
        	header = header + 'Content-Length: %d\r\n' % (length)

	header = header + '\r\n'
	return header

def error_message(path):
	return '''<html>
	<head><title>404 Not Found</title></head>
	<body><h1>Not Found</h1>
	<p>The requested URL %s was not found on this server.</p>
	</body></html>''' % path

class ServerThread(threading.Thread):
	def __init__(self, client_socket, addr):
		self.socket = client_socket
		self.addr = addr
		threading.Thread.__init__(self)

	def run(self):
		# whether the client supports gzip content-encoding
		enable_gzip = False
		# whether the client supports chunked transfer encoding
		enable_chunked = False

        	request = ''
        	while True:
            		data = self.socket.recv(1024)
            		if data:
                		request = request + data
                		if data.endswith('\r\n\r\n'):
                    			break
            		else:
                		break

		request_lines = request.split('\r\n')
        	if len(request_lines) == 0:
            		self.socket.close()
            		return

        	# get requested file path
        	path = ''
        	if request_lines[0].startswith('GET '):
            		lines = request_lines[0].split(' ')
            		if (len(lines) >= 2):
				path = lines[1]

        	# HTTP 1.1 should support chunked transfer encoding
        	if "HTTP/1.1" in request_lines[0]:
            		enable_chunked = True

        	for line in request_lines:
            		if line.startswith('Accept-Encoding: ') and 'gzip' in line:
                		enable_gzip = True
                		break

        	# No requested file for GET
		if len(path) == 0:
			self.socket.close()
			return

        	try:
            		# if the requested path is not specified
            		if path == '/':
				path = '/index.html'

            		# get content-type based on the requested file
            		type = path_to_type(path)

            		# find the requested file from the Web folder
            		global web_folder
            		f = open(web_folder + path, 'rb')
            		body = f.read()
            		f.close()

            		header = http_header('200 OK', type, len(body), enable_gzip, enable_chunked)

            		if enable_gzip:
				body = gzip_compress(body)

            		if enable_chunked:
                		self.socket.sendall(header)
                		# chunk size
                		chunk_size = 1024
                		i = 0
                		length = len(body)
                		# send the body chunk by chunk
                		while i < length:
                    			if i + chunk_size <= length:
                        			chunk = body[i : i + chunk_size]
                    			else:
                        			chunk = body[i :]
                    			i = i + chunk_size
                    			self.socket.sendall('%x\r\n' % len(chunk))
                    			self.socket.sendall(chunk)
                    			self.socket.sendall("\r\n")
                		# end of the chunk
                		self.socket.sendall('%x\r\n\r\n' % 0)
            		else:
                		self.socket.sendall(header + body)

		except:
			body = error_message(path)
            		if enable_gzip:
                		body = gzip_compress(body)

            		type = 'text/html'
            		# For 404 page, there is no need to apply chunked transfer encoding
            		header = http_header('404 Not Found', type, len(body), enable_gzip, False)
            		self.socket.sendall(header + body)

		self.socket.close()


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: python %s [port] [web folder]" % sys.argv[0]
        sys.exit(1)

    # create a socket object using TCP (SOCK_STREAM) as the transport protocol
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # IP address of the server
    server_ip = "0.0.0.0"
    # port number
    server_port = int(sys.argv[1])
    # Web folder: global variable
    web_folder = sys.argv[2].rstrip('/')

    # bind socket to local address
    s.bind((server_ip, server_port))
    # queue up to 5 requests
    s.listen(5)

    print 'Start HTTP server on %s:%d\nWeb folder: %s' % (server_ip, server_port, web_folder)

    while True:
        # establish a connection
        client_socket, addr = s.accept()
        t = ServerThread(client_socket, addr)
        t.start()
