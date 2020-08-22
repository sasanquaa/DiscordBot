/*
 * Copyright (c) 2014, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WEBSOCKETPP_CONNECTION_H
#define WEBSOCKETPP_CONNECTION_H

#include <websocketpp/tls/connection_base.h>
#include <rest/hsocket.h>
#include <websocketpp/uri.cpp>
#include <websocketpp/logger.cpp>
#include <memory>
#include <websocketpp/common/platforms.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/// Empty timer class to stub out for timer functionality that iostream
/// transport doesn't support
struct timer {
    void cancel() {}
};

/*
* Modified version from the original file of Websocket++. This is modified from 
* websocketpp::transport::iostream::connection to use socket instead of std::ostream.
*/


namespace tls {

template <typename config>
class connection : public std::enable_shared_from_this<connection<config>> {
public:

	typedef std::shared_ptr<connection<config>> ptr;
	typedef std::shared_ptr<timer> timer_ptr;

	typedef typename config::concurrency_type::scoped_lock_type scoped_lock_type;
	typedef typename config::concurrency_type::mutex_type mutex_type;

	typedef typename config::alog_type alog_type;
	typedef typename config::elog_type elog_type;

	explicit connection(bool is_server, const std::shared_ptr<alog_type> & alog, const std::shared_ptr<elog_type> & elog)
		: m_is_server(is_server)
		, m_is_secure(true)
		, m_alog(alog)
		, m_elog(elog)
		, m_remote_endpoint("SauceSearch BOT")
	{
		m_alog->write(logger::alevel::app, NAME + " Init tls_connection constructor");
		m_host = "gateway.discord.gg";
		m_port = config::port;
		m_hsocket = new hsocket_tls();
		socket_init();
	}

	/**
	 * Get a shared pointer to this component.
	 */
	std::shared_ptr<connection<config>> get_shared() {
		return connection<config>::shared_from_this();
	}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	void register_ostream() {}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	void set_uri(uri_ptr ptr) {}

	/** Original read_some()
	 * ----Modified----
	 * Copies bytes from buf into m_buf.
	 * It will return the number of bytes successfully processed. All of the
	 * bytes will be read in one call.
	 * ----------------
	 *
	 * @param buf Char buffer to read into the websocket
	 * @param len Length of buf
	 * @return The number of characters from buf actually read.
	 */
	size_t copy_some(char const * buf, size_t len) {
		scoped_lock_type lock(m_read_mutex);
		return 0;
	}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	size_t read_all() {}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	size_t readsome() {}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	void eof() {}

	/**
	 * ----Modified----
	 * Not required.
	 * ----------------
	 */
	void fatal_error() {}

	/**
	 * ----Modified----
	 * This method allows that external API to flag
	 * whether or not this connection is secure so that users of the WebSocket++
	 * API will get more accurate information.
	 *
	 * This is used to set m_is_secure for socket. Indicates whether it uses TLS or not.
	 * ----------------
	 *
	 * @param value Whether or not this connection is secure.
	 */
	void set_secure(bool value) {
		m_is_secure = value;
	}

	/**
	 * ----Modified----
	 * The `set_secure` method may be used to flag connections
	 * that are secured by an external API.
	 * ----------------
	 *
	 * @return Whether or not the underlying transport is secure
	 */
	bool is_secure() const {
		return m_is_secure;
	}

	/**
	 * Sets the remote endpoint address returned by `get_remote_endpoint`. This
	 * value should be a human readable string that describes the remote
	 * endpoint. Typically an IP address or hostname, perhaps with a port. But
	 * may be something else depending on the nature of the underlying
	 * transport.
	 *
	 * If none is set the default is "socket_connection transport".
	 *
	 * @since 0.3.0-alpha4
	 *
	 * @param value The remote endpoint address to set.
	 */
	void set_remote_endpoint(std::string value) {
		m_remote_endpoint = value;
	}

	/**
	 * ----Modified----
	 * It will return the string "socket_connnection transport" by default. The
	 * `set_remote_endpoint` method may be used by external network code to set
	 * a more accurate value.
	 * ----------------
	 *
	 * This value is used in access and error logs and is available to the end
	 * application for including in user facing interfaces and messages.
	 *
	 * @return A string identifying the address of the remote endpoint
	 */
	std::string get_remote_endpoint() const {
		return m_remote_endpoint;
	}

	/**
	 * @return The handle for this connection.
	 */
	connection_hdl get_handle() const {
		return m_connection_hdl;
	}

	/**
	 * Timers are not implemented in this transport. The timer pointer will
	 * always be empty. The handler will never be called.
	 *
	 * @param duration Length of time to wait in milliseconds
	 * @param callback The function to call back when the timer has expired
	 * @return A handle that can be used to cancel the timer if it is no longer
	 * needed.
	 */
	std::shared_ptr<timer> set_timer(long, timer_handler) {
		return std::shared_ptr<timer>();
	}

	/**
	 * ----Modified----
	 * Set an optional handler for writing input into the socket.
	 * The signature of the handler is
	 * `std::error_code (connection_hdl, char const *, size_t)`.
	 *
	 * See also, set_vector_write_handler, for an optional write handler that
	 * allows more efficient handling of multiple writes at once.
	 * ----------------
	 *
	 * @see set_vector_write_handler
	 *
	 * @param h The handler to call when data is to be written.
	 */
	void set_write_handler(async_write_handler h) {
		m_write_handler = h;
	}

	/**
	 * The vectored write handler is called when the iostream transport receives
	 * multiple chunks of data that need to be written to the appropriate output
	 * location. This handler can be used in conjunction with the write_handler
	 * in place of registering an ostream for output.
	 *
	 * The sequence of buffers represents bytes that should be written
	 * consecutively and it is suggested to group the buffers into as few next
	 * layer packets as possible. Vector write is used to allow implementations
	 * that support it to coalesce writes into a single TCP packet or TLS
	 * segment for improved efficiency.
	 *
	 * This is an optional handler. If it is not defined then multiple calls
	 * will be made to the standard write handler.
	 *
	 * The signature of the handler is
	 * `std::error_code (connection_hdl, std::vector<websocketpp::transport::buffer>
	 * const & bufs)`. The code returned will be reported and logged by the core
	 * stdrary. The `websocketpp::transport::buffer` type is a struct with two
	 * data members. buf (char const *) and len (size_t).
	 *
	 * @since 0.6.0
	 *
	 * @param h The handler to call when vectored data is to be written.
	 */
	void set_vector_write_handler(vector_write_handler h) {
		m_vector_write_handler = h;
	}

	/**
	 * The shutdown handler is called when the iostream transport receives a
	 * notification from the core stdrary that it is finished with all read and
	 * write operations and that the underlying transport can be cleaned up.
	 *
	 * If you are using iostream transport with another socket stdrary, this is
	 * a good time to close/shutdown the socket for this connection.
	 *
	 * The signature of the handler is `std::error_code (connection_hdl)`. The
	 * code returned will be reported and logged by the core stdrary.
	 *
	 * @since 0.5.0
	 *
	 * @param h The handler to call on connection shutdown.
	 */
	void set_shutdown_handler(async_shutdown_handler h) {
		m_shutdown_handler = h;
	}
protected:
	/**
	 * Initialize the connection's transport component.
	 *
	 * @param handler The `init_handler` to call when initialization is done
	 */
	void init(init_handler handler) {
		m_alog->write(logger::alevel::app, NAME + " init()");
		handler(std::error_code());
	}

	/**
	 * Initiates an async_read request for at least num_bytes bytes. The input (from socket)
	 * will be read into buf. A maximum of len bytes will be input. When the
	 * operation is complete, handler will be called with the status and number
	 * of bytes read.
	 *
	 * This method may or may not call handler from within the initial call. The
	 * application should be prepared to accept either.
	 *
	 * The application should never call this method a second time before it has
	 * been called back for the first read. If this is done, the second read
	 * will be called back immediately with a double_read error.
	 *
	 * If num_bytes or len are zero handler will be called back immediately
	 * indicating success.
	 *
	 * ----Modified----
	 * During handshake, the FIRST async_read_at_least will be called to read the handshake HTTP response.
	 * If the number of bytes read from the socket exceed len bytes, refer below. Otherwise, m_handshaking will
	 * be set to true, the handler is called and start processing the handshake response. When the handshake response is valid,
	 * the underlying handler assumes that the last few bytes are frame data and proceed to call this method again which will be
	 * cancelled and m_handshaked is set to true to indicate the handshake process in completed.
	 *
	 * If the number of bytes read from the socket exceed len bytes, len bytes will be
	 * read into buf and the handler will be called with the status and number of len bytes read.
	 * The flag m_want_read will be set to true which indicates this method will be called
	 * again to read the remaining bytes.
	 * ----------------
	 *
	 * @param num_bytes Don't call handler until at least this many bytes have
	 * been read.
	 * @param buf The buffer to read bytes into
	 * @param len The size of buf. At maximum, this many bytes will be read.
	 * @param handler The callback to invoke when the operation is complete or
	 * ends in an error
	 */
	void async_read_at_least(size_t num_bytes, char *buf, size_t len,
		read_handler handler)
	{
		if (num_bytes > len) {
			handler(make_error_code(transport::error::invalid_num_bytes), size_t(0));
			return;
		}

		if (num_bytes == 0 || len == 0) {
			handler(std::error_code(), size_t(0));
			return;
		}

		std::string t = m_hsocket->read_to_string();
		memcpy(buf, t.c_str(), t.size());

		if (t.size() > 0 && t.size() >= num_bytes) handler(std::error_code(), t.size());
	}

	/**
	 * Write len bytes in buf to the output method. Call handler to report
	 * success or failure. handler may or may not be called during async_write,
	 * but it must be safe for this to happen.
	 *
	 * ----Modified----
	 * This will write buf to socket.
	 * ----------------
	 *
	 * @param buf buffer to read bytes from
	 * @param len number of bytes to write
	 * @param handler Callback to invoke with operation status.
	 */
	void async_write(char const * buf, size_t len, write_handler
		handler)
	{
		m_alog->write(logger::alevel::app, NAME + " async_write()");
		m_hsocket->write(buf, len);
		handler(std::error_code());
	}

	/**
	 * Write a sequence of buffers to the output method. Call handler to report
	 * success or failure. handler may or may not be called during async_write,
	 * but it must be safe for this to happen.
	 *
	 * ----Modified----
	 * This will write sequences of buffer (bufs) to socket.
	 * ----------------
	 *
	 * @param bufs vector of buffers to write
	 * @param handler Callback to invoke with operation status.
	 */
	void async_write(std::vector<buffer> const & bufs, write_handler
		handler)
	{
		m_alog->write(logger::alevel::app, NAME + " async_write()");
		std::vector<buffer>::const_iterator it;
		for (it = bufs.begin(); it != bufs.end(); it++) {
			m_hsocket->write((*it).buf, (*it).len);
		}
		handler(std::error_code());
	}

	/**
	 * @param hdl The new handle
	 */
	void set_handle(connection_hdl hdl) {
		m_connection_hdl = hdl;
	}

	/**
	 * Invoke a callback within the transport's event system if it has one. If
	 * it doesn't, the handler will be invoked immediately before this function
	 * returns.
	 *
	 * @param handler The callback to invoke
	 *
	 * @return Whether or not the transport was able to register the handler for
	 * callback.
	 */
	std::error_code dispatch(tls_dispatch_handler handler) {
		handler();
		return std::error_code();
	}

	/**
	 * If a shutdown handler is set, call it and pass through its return error
	 * code. Otherwise assume there is nothing to do and pass through a success
	 * code.
	 *
	 * @param handler The `shutdown_handler` to call back when complete
	 */
	void async_shutdown(shutdown_handler handler) {
		std::error_code ec;
		if (m_shutdown_handler) {
			ec = m_shutdown_handler(m_connection_hdl);
		}
		handler(ec);
	}
private:

	void socket_init() {
		m_hsocket->connect_to(m_host, m_port);
		if (m_hsocket->is_connected()) {
			reset_stream();
			m_s << NAME << " initialized tls_connection" << ", secured connection: " << bool(m_is_secure);
			m_alog->write(logger::alevel::app, m_s.str());
		}
	}

	void reset_stream() {
		m_s.str("");
		m_s.clear();
	}

	// transport resources
	std::string		m_host;
	short			m_port;
	hsocket_tls*	m_hsocket;
	

	// handlers
	connection_hdl			m_connection_hdl;
	async_write_handler		m_write_handler;
	vector_write_handler	m_vector_write_handler;
	async_shutdown_handler  m_shutdown_handler;

	bool const      m_is_server;
	bool            m_is_secure;
	std::shared_ptr<alog_type>     m_alog;
	std::shared_ptr<elog_type>     m_elog;
	std::string     m_remote_endpoint;
	std::stringstream m_s;
	// This lock ensures that only one thread can edit read data for this
	// connection. This is a very coarse lock that is basically locked all the
	// time. The nature of the connection is such that it cannot be
	// parallelized, the locking is here to prevent intra-connection concurrency
	// in order to allow inter-connection concurrency.
	mutex_type      m_read_mutex;
};

}
#endif // WEBSOCKETPP_TRANSPORT_IOSTREAM_CON_HPP

