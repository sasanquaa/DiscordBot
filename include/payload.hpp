/*
 * SASANQUA, 10-August-2020
 */

#ifndef DISCORD_PAYLOAD
#define DISCORD_PAYLOAD

#include <opcode.hpp>
#include <misc/json.hpp>
#include <stack>
#include <vector>

typedef std::error_code payload_type;

/*
From Discord:
"Resource fields that may contain a null value have types that are prefixed with a question mark. 
Resource fields that are optional have names that are suffixed with a question mark."
*/

/*
 * This defines the required fields in a specific payload type. 
 * Specifically the field "d" or "data" of a Discord payload for a certain opcode requires 
 * some field to be defined. These constant JSON are used to check the validity (existence) of such field(s) (key(s))
 * by using the validate() method.
 */

static const nlohmann::json IDENTIFY = 
{ 
	{"token", {}},
	{"properties", {"$os", "$browser", "$device"} } 
};

static const nlohmann::json RESUME =
{
	{"token", {}},
	{"session_id", {}},
	{"seq", {}}
};

static const nlohmann::json HEARTBEAT =
{};


static const nlohmann::json REQUEST_GUILDS_MEMBERS =
{
	{"guild_id", {}},
	{"limit", {}}
};

static const nlohmann::json UPDATE_VOICE_STATE =
{
	{"guild_id", {}},
	{"channel_id", {}},
	{"self_mute", {}},
	{"self_deaf", {}}

};

static const nlohmann::json UPDATE_STATUS =
{
	{"since", {}},
	{"game", {}},
	{"status", {}},
	{"afk", {}}
};


//Use for both Gateway and Rest API

class payload {

public:

	payload() : 
		m_data({}) 
	{}

	payload(payload_type t) :
		m_type(t),
		m_data({}),
		m_s(),
		m_t(),
		m_valid(false)
	{}
	
	template<class T> T get_data_key(const std::string& key) const {
		if (m_data.count(key)) return static_cast<T>(m_data[key]);
		return NULL;
	}
	
	template<class T> T set_data_key(const std::string& key, const T& value) {
		m_data[key] = value;
		return value;
	}

	template<class T> T set_data_key(const std::vector<std::string>& keys, const T& value) {
		for (auto i : m_data.items()) {
			if (i.key() == keys[0]) {
				nlohmann::json& j = i.value();
				for (int k = 1; k < keys.size() - 1 && j.count(keys[k]); k++) j = j[keys[k]];
				if(j.count(keys[keys.size() - 1])) j[keys[keys.size() - 1]] = value;
			}
		}
		return value;
	}

	void set_sequence(const nlohmann::json& s) {
		m_s = s;
	}

	void set_name(const nlohmann::json& t) {
		m_t = t;
	}

	const std::string get_gateway_payload(int indent = 0) {
		nlohmann::json p;
		p["op"] = m_type.value();
		p["d"] = m_data;
		p["s"] = m_s;
		p["t"] = m_t;
		return p.dump(indent);
	}

	const std::string get_payload(int indent = 0) {
		return m_data.dump(indent);
	}

	const nlohmann::json& get_data() { return m_data; }

	const bool& is_valid() {
		return m_valid;
	}

	void validate() {
		nlohmann::json j;

		switch (m_type.value()) {
		case opcode::identify:
			j = IDENTIFY;
			break;
		case opcode::resume:
			j = RESUME;
			break;
		case opcode::heartbeat:
			j = HEARTBEAT;
			break;
		case opcode::request_guild_members:
			j = REQUEST_GUILDS_MEMBERS;
			break;
		case opcode::voice_state_update:
			j = UPDATE_VOICE_STATE;
			break;
		case opcode::presence_update:
			j = UPDATE_STATUS;
			break;
		default:
			j = nullptr;
			break;
		}

		std::stack<nlohmann::json::iterator> q;
		std::stack<int> levels;
		std::vector<std::string> parents;

		for (nlohmann::json::iterator it = j.begin(); it != j.end(); it++) q.push(it);

		while (!q.empty()) {
			nlohmann::json::iterator it = q.top();
			nlohmann::json parent_json = m_data;
			q.pop();
			for (std::string s : parents) {parent_json = parent_json[s];}
			if (!levels.empty()) {
				levels.top() -= 1;
				if (levels.top() == 0) {
					levels.pop();
					parents.pop_back();
				}
			}
			std::string p = it.key();
			if (!parent_json.count(p)) {
				m_valid = false;
				return;
			}
			if (!it.value().is_null()) {
				if (it.value().is_array() || it.value().is_string()) {
					for (std::string i : it.value()) {
						if (!parent_json[p].count(i)) {
							m_valid = false;
							return;
						}	
					}
				}
				else if (it.value().is_object()) {
					parents.push_back(p);
					levels.push(it.value().size());
					for (nlohmann::json::iterator itx = it.value().begin(); itx != it.value().end(); itx++) {q.push(itx);}
				}
			}
		}
		m_valid = true;
	}

private:

	payload_type	m_type;
	nlohmann::json	m_data;
	nlohmann::json	m_s;
	nlohmann::json	m_t;
	bool			m_valid;

};


#endif 