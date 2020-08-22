#ifndef DISCORD_GATEWAY_OPCODES
#define DISCORD_GATEWAY_OPCODES

#include <system_error>


namespace opcode {

	enum gateway {
		
		dispatch = 0,

		heartbeat,
		
		identify,
		
		presence_update,
		
		voice_state_update,
		
		resume = 6,
		
		reconnect,
		
		request_guild_members,
		
		invalid_session,
		
		hello,
		
		heartbeat_ack
	};

	enum gateway_close {
		
		unknown_error = 4000,
		
		unknown_opcode,
		
		decode_error,
		
		not_authenticated,
		
		authentication_failed,
		
		already_authenticated,
		
		invalid_seq,
		
		rate_limited,
		
		session_timed_out,
		
		invalid_shard,
		
		sharding_required,
		
		invalid_api_version,
		
		invalid_intent,
		
		disallowed_intent
	};

	/*
	enum voice {
		
		identify = 0,
		
		select_protocol,
		
		ready,
		
		heartbeat,
		
		session_description,
		
		speaking,
		
		heartbeat_ack,
		
		resume,
		
		hello,
		
		resumed,
		
		client_disconnect
	};
	*/

	class gateway_opcode : public std::error_category {

	public:

		gateway_opcode() {}
		
		const char* name() const noexcept {
			return "gateway_opcode";
		}

		std::string message(int e) const {
			switch (e) {
			case gateway::dispatch:
				return "Dispatch";
			case gateway::heartbeat:
				return "Heartbeat";
			case gateway::identify:
				return "Identify";
			case gateway::presence_update:
				return "Presence Update";
			case gateway::voice_state_update:
				return "Voice State Update";
			case gateway::resume:
				return "Resume";
			case gateway::reconnect:
				return "Reconnect";
			case gateway::request_guild_members:
				return "Request Guild Members";
			case gateway::invalid_session:
				return "Invalid Session";
			case gateway::hello:
				return "Hello";
			case gateway::heartbeat_ack:
				return "Heartbeat ACK";
			default:
				return "Unknown";
			}
		}

	};

	class gateway_close_opcode : public std::error_category {

	public:

		gateway_close_opcode() {}
	
		const char* name() const noexcept {
			return "gateway_close_opcode";
		}

		std::string message(int e) const {
			switch (e) {
			case gateway_close::unknown_error:
				return "Unknown error";
			case gateway_close::unknown_opcode:
				return "Unknown opcode";
			case gateway_close::decode_error:
				return "Decode error";
			case gateway_close::not_authenticated:
				return "Not authenticated";
			case gateway_close::authentication_failed:
				return "Authentication failed";
			case gateway_close::already_authenticated:
				return "Already authenticated";
			case gateway_close::invalid_seq:
				return "Invalid sequence";
			case gateway_close::rate_limited:
				return "Rate limited";
			case gateway_close::session_timed_out:
				return "Session timed out";
			case gateway_close::invalid_shard:
				return "Invalid shard";
			case gateway_close::sharding_required:
				return "Sharding required";
			case gateway_close::invalid_api_version:
				return "Invalid API version";
			case gateway_close::invalid_intent:
				return "Invalid intent";
			case gateway_close::disallowed_intent:
				return "Disallowed intent";
			default:
				return "Unknown";
		}
		}

	};

	static opcode::gateway_opcode go;

	static opcode::gateway_close_opcode gco;

	inline std::error_code make_error_code(opcode::gateway e) {
		return std::error_code(static_cast<int>(e), go);
	}

	inline std::error_code make_error_code(opcode::gateway_close e) {
		return std::error_code(static_cast<int>(e), gco);
	}
}


namespace std {

	template<> struct is_error_code_enum<opcode::gateway> : public std::true_type {};

	template<> struct is_error_code_enum<opcode::gateway_close> : public std::true_type {};
}

#endif
