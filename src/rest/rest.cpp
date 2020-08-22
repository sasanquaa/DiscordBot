#include <rest/rest.h>
#include <algorithm>

rest::rest(const std::string& host, const short& port) :
	m_host(host),
	m_port(port),
	m_hsocket(new hsocket_tls())
	{}

rest::~rest() {
	delete m_hsocket;
}

void rest::open() { m_hsocket->connect_to(m_host, m_port); }

nlohmann::json rest::send(method& method, const std::string& route, const nlohmann::json& h, const nlohmann::json& data) {
	if (!m_hsocket->is_connected()) return {};
	std::string headers;
	for (auto& i : h.items()) {
		std::string v = i.value().dump();
		v.erase(std::remove(v.begin(), v.end(), '\"'), v.end());
		headers.append(i.key() + ": " + v + "\r\n");
	}
	if(headers.rfind('\n', headers.size() - 1) == headers.size() - 1) headers.erase(headers.end() - 2, headers.end());
	return send(method, route, headers, data.dump());
}

nlohmann::json rest::send(method& method, const std::string& route, const std::string& headers, const std::string& data) {
	std::string request = handle_request(method, m_host, route, headers, data);
	m_hsocket->write(request.c_str(), request.size());
	std::cout << "RestAPI [Request]: \n\n" << request << "\n";
	nlohmann::json response = handle_response(m_hsocket->read_to_string());
	std::cout << "RestAPI [Response]: \n" << response.dump(2) << "\n";
	return response;
}

void rest::close() {
	m_hsocket->disconnect();
}
