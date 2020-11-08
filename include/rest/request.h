
#ifndef REST_STRUCTS
#define REST_STRUCTS

#include <string>
#include <misc/json.hpp>
#include <sstream>
#include <iostream>

#define HTTP std::string("HTTP/1.1\r\n")

enum value {
	GET,
	PUT,
	POST,
	DELETE
};

inline std::string get_method_name(value m) {
	switch (m) {
	case GET:
		return "GET";
	case PUT:
		return "PUT";
	case POST:
		return "POST";
	case DELETE:
		return "DELETE";
	default:
		return "Unknown";
	}
}

struct method {

protected:
	value		m_method;
	std::string m_route;
	std::string m_header;
	std::string m_host;

public:
	method(value m): m_method(m) {}

	void route(std::string r) { m_route = r; }

	void host(std::string h) { m_host = h.insert(0, "Host: "); }

	void header(std::string h) { m_header.append(h); }

	void reset_header() { m_header.clear(); }

	value get_method() const { return m_method; }

	virtual void content_type(std::string) { return; }

	virtual std::string dump() {
		std::string tmp = get_method_name(m_method) + " " + m_route + " " + HTTP + m_host + "\r\n" + m_header + "\r\n\r\n";
		return tmp;
	}

};

inline struct get : public virtual method {

	get(value m) : method(m)  {}

} REST_GET(GET);


inline struct post : public virtual method {

public:

	post(value m) : method(m) {}

} REST_POST(POST);

inline struct put : public virtual method {

	put(value m) : method(m) {}

} REST_PUT(PUT);

inline std::string handle_request(
	method&			   method, 
	const std::string& host, 
	const std::string& route, 
	const std::string& headers, 
	const std::string& data) {
	
	std::string request;

	method.reset_header();
	method.route(route);
	method.host(host);
	method.header(headers);

	request = method.dump() + data;
	
	return request;
}

inline nlohmann::json handle_response( 
	const std::string& res) {
	std::cout << "Raw response:\n" << res;
	std::stringstream s(res);
	std::string l;

	nlohmann::json response;

	while (getline(s, l)) {
		if (strcmp(l.c_str(), "\r") == 0) continue;
		if (l.rfind("x-ratelimit") == 0) {
			std::string_view view(l);
			int i = l.find(":");
			std::string key = l.substr(0, i);
			std::string value = l.substr(i + 2, l.size() - i - 3);
			response[key] = value;
		}
		else if (l.at(0) == '{') {
			try {
				response["data"] = nlohmann::json::parse(l);
			}
			catch (nlohmann::json::parse_error) {
				response["data"] = l;
			}
		}
	}

	return response;
}

#endif
