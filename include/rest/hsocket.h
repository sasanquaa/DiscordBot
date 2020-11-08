/*
 * SASANQUA 09-August-2020
 */

#ifndef HSOCKET
#define HSOCKET

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <openssl/ssl.h>
#include <iostream>

//Winsocket and OpenSSL initialization
static struct WSINIT {
	WSINIT() {
#ifndef WS_INIT
#define WS_INIT
		WSAData wsaData;
		WORD dllVer = MAKEWORD(2, 1);
		WSAStartup(dllVer, &wsaData);
		OPENSSL_init_ssl(0, NULL);
#endif
	}
};
static WSINIT init;

static const SSL_METHOD *DEFAULT_METHOD = TLSv1_2_method();

//Macros

#define DEFAULT_BUFFER_SIZE 4096

#define TIMEOUT 5000L

#define INTERNET AF_INET

#define TCP SOCK_STREAM

#define UDP SOCK_DGRAM

#define IP_TCP IPPROTO_TCP

#define IP_UDP IPPROTO_UDP

#define NON_BLOCKING FIONBIO

typedef SOCKET hsocket;

typedef u_long mode;

typedef ULONG ip;

//Wrappers

inline hsocket create_tcp_socket() { 
	return socket(INTERNET, TCP, IP_TCP); 
}

inline hsocket create_udp_socket() {
	return socket(INTERNET, UDP, IP_UDP);
}

inline void set_non_blocking(hsocket hs, mode* m) {
	ioctlsocket(hs, NON_BLOCKING, m);
}

inline void wait(fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) {
	select(0, read, write, except, timeout);
}

inline void wait_read(hsocket hs, const timeval* timeout) {
	struct fd_set fds;
	FD_ZERO(&fds);
	FD_SET(hs, &fds);
	wait(&fds, nullptr, nullptr, timeout);
}

inline void wait_write(hsocket hs, const timeval* timeout) {
	struct fd_set fds;
	FD_ZERO(&fds);
	FD_SET(hs, &fds);
	wait(nullptr, &fds, nullptr, timeout);
}

inline void wait_read_write(hsocket hs, const timeval* timeout) {
	struct fd_set fds;
	FD_ZERO(&fds);
	FD_SET(hs, &fds);
	wait(&fds, &fds, nullptr, timeout);
}

/*
 * Class hsocket_tls initiates SSL and socket with non-blocking I/O, TLS 1.2 by default.
 * It handles asynchronous writing and reading for the socket with default timeout time is 5 seconds.
 * It is required to call connect_to (to initialize connection, handshaking, etc...) before 
 * being able to do writing/reading.
 */
class hsocket_tls {
public:

	hsocket_tls() : 
		m_buffer(new char[0]), 
		m_len(0),
		m_connected(false),
		m_reading(false),
		m_timeout(new timeval()) {}

	~hsocket_tls() {
		delete m_timeout;
		delete[] m_buffer;

		closesocket(m_socket);

		SSL_free(m_ssl);
		SSL_CTX_free(m_ssl_ctx);
	}

	/**
	 * This method try to connect to the given host and port (in client mode).
	 * If there is already an established connection, by m_connected, it will return.
	 * Otherwise, m_connected is only set to true 
	 * if the host is valid, connection and TLS handshake is successful.
	 *
	 * @param host The host to create connection to.
	 * @param port The port of the host.
	 */
	void connect_to(std::string host, short port) {
		if (m_connected) return;
		m_host = host;
		m_port = port;

		ip ip_addr = get_host_ip(m_host, std::to_string(port));
		if (ip_addr == 0) return;

		struct sockaddr_in sin;
		sin.sin_family = INTERNET;
		sin.sin_port = htons(m_port);
		sin.sin_addr.S_un.S_addr = ip_addr;

		m_socket = create_tcp_socket();
		m_timeout->tv_sec = TIMEOUT / 1000;
		m_timeout->tv_usec = (TIMEOUT * 1000) % 1000000;
		set_non_blocking(m_socket, &m_mode);

		m_ssl_ctx = SSL_CTX_new(DEFAULT_METHOD);
		m_ssl = SSL_new(m_ssl_ctx);
		SSL_set_fd(m_ssl, m_socket);
		SSL_set_connect_state(m_ssl);

		connect(m_socket, (const sockaddr*) &sin, sizeof(sin));
		wait_read_write(m_socket, m_timeout);

		int e = SSL_get_error(m_ssl, SSL_do_handshake(m_ssl));

		while (e) {
			switch (e) {
			case SSL_ERROR_WANT_READ:
				wait_read(m_socket, m_timeout);
				break;
			case SSL_ERROR_WANT_WRITE:
				wait_write(m_socket, m_timeout);
				break;
			default:
				break;
			}
			e = SSL_get_error(m_ssl, SSL_do_handshake(m_ssl));
		}

		m_connected = true;
	}

	void disconnect() {
		if (!m_connected) return;
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
		SSL_CTX_free(m_ssl_ctx);
		closesocket(m_socket);
		m_connected = false;
	}
	
	/**
	 * This method writes buf of len bytes to the socket with SSL_write. 
	 * By OpenSSL for non-blocking I/O, it will return with success 
	 * if all of buf of length len has been written. Otherwise, an error is returned, 
	 * call SSL_get_error will return SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE.
	 *
	 * @param buf Char buffer that contains data to write.
	 * @param len Length of char buffer.
	 *
	 * @return Number of bytes written to the socket.
	 */
	size_t write(const char *buf, size_t len) {
		if (!m_connected) return 0;
		wait_write(m_socket, m_timeout);
		int r = SSL_write(m_ssl, buf, len);
		return len;
	}

	/**
	 * This method will try to read from the socket by first calling wait_read
	 * with 5 seconds timeout to wait for the data to arrive. Then call SSL_read
	 * to read data into a temporary buffer and copy it m_buffer.
	 * If the total of bytes t read from the socket is larger than len, 
	 * then read len bytes into buf, m_reading is set to true, m_bytes_remain is set
	 * to hold the number bytes the caller need to recall this method to finish reading all of m_bytes_remain.
	 * 
	 * @param buf Char buffer to read the data into.
	 * @param len Length of char buffer.
	 *
	 * @return Number of bytes read.
	 */
	size_t read(char *buf, size_t len) {
		if (!m_connected || len == 0) return size_t(0);

		if (m_reading) {
			if (m_bytes_remain == 0) {
				m_reading = false;
				return size_t(0);
			}
			int t = read_from_buffer(buf, m_bytes_remain > len ? len : m_bytes_remain, m_cursor - m_bytes_remain);
			m_bytes_remain -= t;
			return t;
		}

		int n, t = 0, cursor = m_cursor;
		char buffer[DEFAULT_BUFFER_SIZE];

		wait_read(m_socket, m_timeout);

		while ((n = SSL_read(m_ssl, buffer, sizeof(buffer))) > 0) {
			copy_to_buffer(buffer, n);
			t += n;
			wait_read(m_socket, m_timeout);
		}

		if (t > len) {
			read_from_buffer(buf, len, cursor);
			m_reading = true;
			m_bytes_remain = t - len;
			return size_t(len);
		}

		read_from_buffer(buf, t, cursor);
		return size_t(t);
	}

	/**
	 * This method try to read response from the host and convert it to a string.
	 *
	 * @return A string representing response from host.
	 */
	std::string read_to_string() {
		std::string res;
		char buffer[DEFAULT_BUFFER_SIZE];
		int n;
		while ((n = read(buffer, sizeof(buffer))) > 0) {
			res.append(buffer, n);
		}
		return res;
	}

	bool is_connected() const {
		return m_connected;
	}

private:
	
	size_t copy_to_buffer(char *buf, size_t len) {
		if (len == 0) return size_t(0);
		m_buffer = (char*)realloc(m_buffer, m_len + len);
		memcpy(m_buffer + m_cursor, buf, len);
		m_len += len;
		m_cursor += len;
		return size_t(len);
	}

	size_t read_from_buffer(char *buf, size_t len, size_t cursor) {
		if (len == 0) return size_t(0);
		memcpy(buf, m_buffer + cursor, len);
		memcpy(m_buffer + cursor, m_buffer + cursor + len, m_len - len - cursor);
		m_buffer = (char*)realloc(m_buffer, m_len - len);
		m_len -= len;
		m_cursor -= len;
		return size_t(len);
	}

	ULONG get_host_ip(std::string h, std::string p) {

		struct addrinfo hints, *results;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = INTERNET;
		hints.ai_socktype = TCP;
		hints.ai_protocol = IP_TCP;
		hints.ai_flags = AI_CANONNAME;

		getaddrinfo(h.c_str(), p.c_str(), &hints, &results);

		for (auto p = results; p != NULL; p = p->ai_next) {
			if (strcmp(p->ai_canonname, h.c_str()) == 0) {
				std::stringstream s;
				sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(p->ai_addr);
				in_addr addr = sin->sin_addr;
				s << std::to_string(addr.S_un.S_un_b.s_b1) << "." 
				  << std::to_string(addr.S_un.S_un_b.s_b2) << "." 
				  << std::to_string(addr.S_un.S_un_b.s_b3) << "." 
				  << std::to_string(addr.S_un.S_un_b.s_b4);
				printf("-----------------------------\n");
				printf("IP Address: %s\nHost name: %s\nSocket type: %d\nSocket protocol: %d\n", s.str().c_str(), p->ai_canonname, p->ai_socktype, p->ai_protocol);
				printf("-----------------------------\n");
				return addr.S_un.S_addr;
			}
		}

		return 0;
	}

	std::string		m_host;
	short			m_port;
	bool			m_connected;
	bool			m_reading;

	char*			m_buffer;
	size_t			m_len;
	size_t			m_cursor;
	size_t			m_bytes_remain;
	
	hsocket			m_socket;
	mode			m_mode = 1;
	SSL*			m_ssl;
	SSL_CTX*		m_ssl_ctx;
	timeval*		m_timeout;
};

#endif