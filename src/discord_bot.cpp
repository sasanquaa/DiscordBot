#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <discord_bot.h>


discord_bot::discord_bot(std::string h, std::string t) :
	m_host(h),
	m_token(t),
	m_ws_route("/?v=6&encoding=json&compress=zlib-stream"),
	m_rest(new rest()),
	m_on_open(nullptr),
	m_on_message(nullptr),
	m_on_close(nullptr),
	m_initialized(false),
	m_heartbeat(opcode::gateway::heartbeat),
	m_heartbeat_interval(0),
	m_sequence(0),
	m_rest_route("/api")
{
	m_rest->open();
	m_gateway = m_rest->send(REST_GET, m_rest_route + "/gateway/bot", "Connection: keep-alive\nAuthorization: Bot " + m_token)["data"];
	if (!m_gateway.count("url")) {
		m_client->get_alog().write(logger::alevel::app, NAME + " Failed to request gateway...");
		exit(EXIT_FAILURE);
	}
	
	std::error_code ec;
	m_client = new dclient_type();
	m_client->set_open_handler(&on_open_forwarder);
	m_client->set_message_handler(std::bind(&on_message_forwarder, m_client, std::placeholders::_1, std::placeholders::_2));
	m_client->set_close_handler(&on_close_forwarder);
	m_client->set_secure(true);
	m_client->set_user_agent("Abby/1");

	m_on_message = std::bind(&discord_bot::on_hello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	m_connection = m_client->get_connection(m_gateway["url"].get<std::string>() + m_ws_route, ec);
	m_hdl = m_connection->get_handle();
	hdl_map[m_hdl.lock().get()] = this;
}

discord_bot::~discord_bot() {
	delete m_rest;
	delete m_client;
}

//Public methods

discord_bot& discord_bot::listen() {

	m_client->connect(m_connection);
	m_client->resume_reading(m_hdl);

	m_client->get_alog().write(logger::alevel::app, NAME + " Sending IDENTIFY payload");

	payload p = event_payload::identify;
	p.set_data_key<std::string>("token", m_token);
	p.set_data_key<int>("intents", intent::GUILD_MESSAGES);
	p.set_data_key<int>(std::vector<std::string>({ "presence", "game", "created_at" }), std::chrono::seconds(std::time(0)).count());

	std::string identify = p.get_gateway_payload();
	m_client->send(m_hdl, identify, frame::opcode::text);
	m_client->resume_reading(m_hdl);

	while (true) {
		auto interval = std::chrono::duration_cast<milliseconds>(h_clock::now() - m_timepoint);
		if (interval >= m_heartbeat_interval) send_heartbeat();
		m_client->resume_reading(m_hdl);
	}
	return *this;
}

discord_bot& discord_bot::on_open(on_open_handler func) {
	m_on_open = func;
	return *this;
}

discord_bot& discord_bot::on_message(on_message_handler func) {
	if (!m_initialized) m_on_message_orig = func;
	else m_on_message = func;
	return *this;
}

discord_bot& discord_bot::on_close(on_close_handler func) {
	m_on_close = func;
	return *this;
}

discord_bot& discord_bot::set_bot_status(int, std::string) {
	return *this;
}


	//p.set_data_key<std::u16string>("content", std::u16string(msg.begin(), msg.end()));

discord_bot& discord_bot::create_message(std::string msg, std::string channel_id) {
	payload p = event_payload::message;
	p.set_data_key<std::string>("content", msg);

	payload h = event_payload::message_headers;
	h.set_data_key<std::string>("Authorization", "Bot " + m_token);
	h.set_data_key<int>("Content-Length", p.get_data().dump().size());

	std::string route = m_rest_route + "/channels/" + channel_id + "/messages";

	nlohmann::json response = m_rest->send(REST_POST, route, h.get_data(), p.get_data());
	return *this;
}

discord_bot& discord_bot::log(std::string msg) {
	m_client->get_alog().write(logger::alevel::app, msg);
	return *this;
}

const std::string& discord_bot::get_token() const {
	return m_token;
}

//Protected methods

void discord_bot::on_hello(int e, std::string e_name, const nlohmann::json& data) {
	m_heartbeat_interval = milliseconds(data["heartbeat_interval"].get<unsigned int>());
	m_on_message = std::bind(&discord_bot::on_identify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_timepoint = h_clock::now();
}

void discord_bot::on_identify(int e, std::string e_name, const nlohmann::json& data) {
	m_initialized = true;
	m_session_id = data["session_id"];
	m_on_message = m_on_message_orig;
}

//Private methods

void discord_bot::send_heartbeat() {
	m_heartbeat.set_sequence(nullptr);
	if (m_sequence) { m_heartbeat.set_sequence(m_sequence); }
	m_client->send(m_hdl, m_heartbeat.get_gateway_payload(), frame::opcode::text);
	m_timepoint = h_clock::now();
}

void discord_bot::on_open_internal(dconnection_hdl hdl) {
	if (m_on_open != nullptr) m_on_open();
}

void discord_bot::on_message_internal(dclient c, dconnection_hdl hdl, msg_ptr msg) {
	const std::string& raw = msg->get_payload();

	nlohmann::json j = nlohmann::json::parse(raw);
	m_sequence = j["s"].is_null() ? m_sequence : j["s"].get<int>();

	int op = j["op"].get<int>();
	std::string op_name = j["t"].is_null() ? "" : j["t"].get<std::string>();
	nlohmann::json data = j["d"];

	switch(op) {
	case opcode::gateway::heartbeat: 
		send_heartbeat();
		break;
	default:
		break;
	}

	if (m_on_message != nullptr) m_on_message(op, op_name, data);
}

void discord_bot::on_close_internal(dconnection_hdl hdl) {
	if (m_on_close != nullptr) m_on_close();
}

void discord_bot::on_open_forwarder(dconnection_hdl hdl) {
	discord_bot* bot = bot_look_up(hdl);
	if (bot == nullptr) return;
	bot->on_open_internal(hdl);
}

void discord_bot::on_message_forwarder(dclient c, dconnection_hdl hdl, msg_ptr msg) {
	discord_bot* bot = bot_look_up(hdl);
	if (bot == nullptr) return;
	bot->on_message_internal(c, hdl, msg);
}

void discord_bot::on_close_forwarder(dconnection_hdl hdl) {
	discord_bot* bot = bot_look_up(hdl);
	if (bot == nullptr) return;
	bot->on_close_internal(hdl);
}
