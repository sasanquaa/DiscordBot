#ifndef DISCORD_BOT_IMPL
#define DISCORD_BOT_IMPL

#include <rest/rest.h>
#include <websocketpp/client.cpp>
#include <websocketpp/tls/client_tls_config.h>
#include <events.h>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <chrono>
#include <rate_limit.h>
#include <misc/zlib.h>

//Using macros and typedef

using h_clock =															std::chrono::high_resolution_clock;

using time_point =														std::chrono::high_resolution_clock::time_point;

using milliseconds =													std::chrono::milliseconds;

typedef client<client_tls_config>										dclient_type;

typedef client<client_tls_config>*										dclient;

typedef std::shared_ptr<connection<client_tls_config>>					dconnection;

typedef connection_hdl													dconnection_hdl;

typedef client_tls_config::message_type::ptr							msg_ptr;

typedef std::function<void()>											on_open_handler;

typedef std::function<void(int, std::string, const nlohmann::json&)>	on_message_handler;

typedef std::function<void()>											on_close_handler;

class discord_bot {

public:
	/**
	*/
	discord_bot(std::string, std::string);

	/**
	*/
	~discord_bot();

	/**
	Called once when the bot successfully connects to the gateway.

	@param on_open_handler A callback method upon gateway connection.
	*/
	discord_bot& on_open(on_open_handler);

	/**
	Called everytime someone sends a message to the server.

	@param on_message_handler A callback method for handling received messages.
	*/
	discord_bot& on_message(on_message_handler);

	/**
	Called once when the bot closes the connection to the gateway.

	@param on_close_handler A callback method to handle closing.
	*/
	discord_bot& on_close(on_close_handler);

	/**
	*/
	discord_bot& set_bot_status(int, std::string);

	/**
	Create a message msg and send it to channel_id

	@param msg		  A message to send.
	@param channel_id A guild channel's id that the bot is in to send message to.
	*/
	discord_bot& create_message(std::string msg, std::string channel_id);

	/**
	*/
	discord_bot& dispatch_event(event::type);

	/**
	*/
	discord_bot& listen();

	/*
	*/
	discord_bot& log(std::string);

	/*
	*/
	const std::string& get_token() const;

protected:
	/*
	*/
	void on_hello(int, std::string, const nlohmann::json&);

	/*
	*/
	void on_identify(int, std::string, const nlohmann::json&);

private:

	/**
	*/
	void on_open_internal(dconnection_hdl);

	/**
	*/
	void on_message_internal(dclient, dconnection_hdl, msg_ptr);

	/**
	*/
	void on_close_internal(dconnection_hdl);

	/*
	*/
	void send_heartbeat();

	/*
	*/
	static void on_open_forwarder(dconnection_hdl);

	/*
	*/
	static void on_message_forwarder(dclient, dconnection_hdl, msg_ptr);

	/*
	*/
	static void on_close_forwarder(dconnection_hdl);


	std::string			m_host;
	std::string			m_token;
	std::string			m_ws_route;
	std::string			m_session_id;
	std::string			m_rest_route;

	nlohmann::json		m_gateway;
	rest*				m_rest;
	rate_limit*			m_ratelimit;
	dclient				m_client;
	dconnection			m_connection;
	dconnection_hdl		m_hdl;

	payload				m_heartbeat;
	milliseconds		m_heartbeat_interval;
	time_point			m_timepoint;
	int					m_sequence;

	on_open_handler		m_on_open;
	on_message_handler	m_on_message;
	on_message_handler	m_on_message_orig;
	on_close_handler	m_on_close;

	bool m_initialized;

};

inline std::unordered_map<void*, discord_bot*> hdl_map;

inline discord_bot* bot_look_up(dconnection_hdl hdl) {
	void* raw_hdl = hdl.lock().get();
	if (hdl_map.count(raw_hdl)) {
		return hdl_map.at(raw_hdl);
	}
	return nullptr;
}

#endif
