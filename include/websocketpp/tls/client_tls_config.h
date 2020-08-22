
#ifndef CLIENT_TLS_CONFIG
#define CLIENT_TLS_CONFIG

// Non-Policy common stuff
#include <websocketpp/common/platforms.hpp>

// Concurrency
#include <thread>
#include <mutex>

// Transport
#include <websocketpp/tls/tls_endpoint.h>

// HTTP
#include <websocketpp/http/request.h>
#include <websocketpp/http/response.h>

// Messages
#include <websocketpp/message_buffer/message.hpp>
#include <websocketpp/message_buffer/alloc.hpp>

// Loggers
#include <websocketpp/logger.cpp>

// RNG
#include <random>

// Extensions
#include <websocketpp/extensions/permessage_deflate/disabled.hpp>

/*
* This is a wrapper class for std::mutex
*/
class basic_concurrency {
public:
    typedef std::mutex mutex_type;
    typedef std::lock_guard<mutex_type> scoped_lock_type;
};

/*
* This is a wrapper class for std::random using std::mutex concurrency
*/
template<typename int_type, typename concurrency>
class random_integer {
public:
	typedef typename concurrency::mutex_type mutex_type;
	typedef typename concurrency::scoped_lock_type scoped_lock_type;

	random_integer() {}

	int_type operator()() {
		scoped_lock_type guard(m_lock);
		return m_dis(m_rng);
	}
private:
	std::random_device m_rng;
	std::uniform_int_distribution<int_type> m_dis;
	mutex_type m_lock;
};

struct client_tls_config{

	typedef client_tls_config type;

	typedef basic_concurrency concurrency_type;

    typedef http::parser::request request_type;
    typedef http::parser::response response_type;

    // Message Policies
    typedef message_buffer::message<message_buffer::alloc::con_msg_manager> message_type;
    typedef message_buffer::alloc::con_msg_manager<message_type> con_msg_manager_type;
    typedef message_buffer::alloc::endpoint_msg_manager<con_msg_manager_type> endpoint_msg_manager_type;

    /// Logging policies
    typedef logger::basic<concurrency_type, logger::alevel> alog_type; 
    typedef logger::basic<concurrency_type, logger::elevel> elog_type;

	typedef random_integer<uint32_t, concurrency_type> rng_type;

	static bool const enable_multithreading = true;

	struct transport_config {
		typedef type::concurrency_type concurrency_type;
		typedef type::alog_type alog_type;
		typedef type::elog_type elog_type;
		typedef type::request_type request_type;
		typedef type::response_type response_type;

		static const short port = 443;

		static bool const enable_multithreading = true;

        static const long timeout_socket_pre_init = 0;

        /// Length of time to wait before a proxy handshake is aborted
        static const long timeout_proxy = 0;

        /// Length of time to wait for socket post-initialization
        /**
         * Exactly what this includes depends on the socket policy in use.
         * Often this means the TLS handshake
         */
		static const long timeout_socket_post_init = 0;

        /// Length of time to wait for dns resolution
        static const long timeout_dns_resolve = 0;

        /// Length of time to wait for TCP connect
        static const long timeout_connect = 0;

        /// Length of time to wait for socket shutdown
        static const long timeout_socket_shutdown = 0;
		
        /// Length of time to wait for reading from socket in ms
		static const long timeout_read = 5000;
	};

	typedef tls::endpoint<transport_config> transport_type;


	typedef class connection_base{} connection_base;
	typedef class endpoint_base{} endpoint_base;

	static const long timeout_open_handshake = 0;

	static const long timeout_close_handshake = 0;

	static const long timeout_pong = 0;


	static const int client_version = 13;

    static const logger::level elog_level = logger::elevel::all;

	static const logger::level alog_level =
		logger::alevel::all;
	//	logger::alevel::debug_handshake |
	//	logger::alevel::control |
	//	logger::alevel::fail;

    static const size_t connection_read_buffer_size = 16384;

    static const bool drop_on_protocol_error = false;

    static const bool silent_close = false;

    static const size_t max_message_size = 32000000;

    static const size_t max_http_body_size = 32000000;

    static const bool enable_extensions = true;

    struct permessage_deflate_config {
        typedef client_tls_config::request_type request_type;

        static const bool allow_disabling_context_takeover = true;

        static const uint8_t minimum_outgoing_window_bits = 8;
    };

    typedef extensions::permessage_deflate::disabled
        <permessage_deflate_config> permessage_deflate_type;

};

#endif
