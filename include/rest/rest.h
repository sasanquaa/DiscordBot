#ifndef REST_API
#define REST_API

#include <rest/request.h>
#include <rest/hsocket.h>
#include <iostream>

class rest {

public:
	
	/**
	 * Constructor with default values for host, port, SSL_METHOD.
	 *
	 * @param host Specify host to connect to.
	 * @param port Port number 80 for unsecure, 443 for secured (TLS) connection.
	 * @param method Method for secured connection, TLS v1.2 by default.
	 */
	rest(const std::string& host = "discord.com", const short& port = 443);
	
	/**
	 * Default destructor.
	 */
	~rest();

	/**
	 * Delete default move semantic.
	 */
	rest(const rest&) = delete;

	/**
	 * Find the host specified by m_host, setting appropriate port m_port and 
	 * host address. Then attempt to connect to the host.
	 */
	void open();
	
	/**
	 * Attempt to close the connection.
	 */
	void close();
	
	/**
	 * Send data to route of host (e.g. "example.com/hello" where route = "hello", host = "example.com")
	 * using HTTP method (GET, PUT, POST, DELETE) then wait for the host to response.
	 *
	 * @param method A HTTP method.
	 * @param route A route relative to the host.
	 * @param data Data to send to the host.
	 * @return JSON object representing the response from the host.
	 */
	nlohmann::json send(method& method, const std::string& route, const nlohmann::json& headers = {}, const nlohmann::json& data = {});

	nlohmann::json send(method& method, const std::string& route, const std::string& headers = "", const std::string& data = "");

private:

	std::string	 m_host;
	short		 m_port;
	hsocket_tls* m_hsocket;
	
};


#endif
