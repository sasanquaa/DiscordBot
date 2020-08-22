REST

+ Class Rest (wrapper for HTTP/REST with OpenSSL) with open(), close(), send(), init(), handshake(), queueBuffer(), readBuffer(), wait()
	1. init() initialize SSL and set non-blocking I/O for socket
	2. open() finds the host with given name and assign to sockaddr_in. Then call connect() and select() to create connection to the host and call handshake()
	3. handshake() perform SSL_do_handshake and handle SSL errors due to underlying non-blocking BIO (socket)
	4. close() close connection to the host
	5. send() takes in an appropriate HTTP/REST method and send it to the host, receive back data and return it as a string
	6. wait() shorthand method to handle SSL errors


