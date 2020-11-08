#ifndef DISCORD_EVENTS
#define DISCORD_EVENTS

#include <payload.hpp>

namespace intent {

	inline const int GUILDS						= 1;
	
	inline const int GUILD_MEMBERS				= 1 << 1;
	
	inline const int GUILD_BANS					= 1 << 2;
	
	inline const int GUILD_EMOJIS				= 1 << 3;
	
	inline const int GUILD_INTERGRATIONS		= 1 << 4;
	
	inline const int GUILD_WEBHOOKS				= 1 << 5;
	
	inline const int GUILD_INVITES				= 1 << 6;
	
	inline const int GUILD_VOICE_STATES			= 1 << 7;
	
	inline const int GUILD_PRESENCES			= 1 << 8;
	
	inline const int GUILD_MESSAGES				= 1 << 9;
	
	inline const int GUILD_MESSAGE_REACTIONS	= 1 << 10;
	
	inline const int GUILD_MESSAGE_TYPING		= 1 << 11;

	inline const int DIRECT_MESSAGES			= 1 << 12;

	inline const int DIRECT_MESSAGE_REACTIONS	= 1 << 13;
	
	inline const int DIRECT_MESSAGE_TYPING		= 1 << 14;

}

namespace event {

	enum type {

		 HELLO							,

		 READY							,

		 RESUMED						,

		 RECONNECT						,

		 INVALID_SESSION				,

		 CHANNEL_CREATE					,

		 CHANNEL_UPDATE					,

		 CHANNEL_DELETE					,

		 CHANNEL_PINS_UPDATE			,

		 GUILD_CREATE					,

		 GUILD_UPDATE					,

		 GUILD_DELETE					,

		 GUILD_BAN_ADD					,

		 GUILD_BAN_REMOVE				,

		 GUILD_EMOJIS_UPDATE			,

		 GUILD_INTEGRATIONS_UPDATE		,

		 GUILD_MEMBER_ADD				,

		 GUILD_MEMBER_REMOVE			,

		 GUILD_MEMBER_UPDATE			,

		 GUILD_MEMBERS_CHUNK			,

		 GUILD_ROLE_CREATE				,

		 GUILD_ROLE_UPDATE				,

		 GUILD_ROLE_DELETE				,

		 INVITE_CREATE					,

		 INVITE_DELETE					,

		 MESSAGE_CREATE					,

		 MESSAGE_UPDATE					,

		 MESSAGE_DELETE					,

		 MESSAGE_DELETE_BULK			,

		 MESSAGE_REACTION_ADD			,

		 MESSAGE_REACTION_REMOVE		,

		 MESSAGE_REACTION_REMOVE_ALL	,

		 MESSAGE_REACTION_REMOVE_EMOJI	,

		 PRESENCE_UPDATE				,

		 TYPING_START					,

		 USER_UPDATE					,

		 VOICE_STATE_UPDATE				,

		 VOICE_SERVER_UPDATE			,

		 WEBHOOKS_UPDATE				
	};
	

}


namespace event_payload {

	//For Gateway

	inline payload identify(opcode::gateway::identify);

	inline payload presence(opcode::gateway::presence_update);

	//For Rest API

	inline payload message;
	inline payload message_headers;
	

}

//Payload initialization

namespace {

	using namespace std;
	using namespace nlohmann;
	using namespace event_payload;

	struct _identify {
		_identify() {
			identify.set_data_key<string>("token", "");
			identify.set_data_key<bool>("compress", true);
			identify.set_data_key<bool>("guild_subscriptions", false);
			identify.set_data_key<int>("intents", 0);
			identify.set_data_key<json>("properties", 
			json(
			{ 
				{"$os", "window"}, 
				{"$browser", "Abby"}, 
				{"$device", "Abby"} 
			}));
			identify.set_data_key<json>("presence", json({ 
				{"since", {}},
				{"game", {{"name", "with SASANQUA <3"}, {"type", 0}, {"created_at", 0}} },
				{"status", "online"},
				{"afk", false}
			}));
		}
	} identify_;

	struct _message {
		_message() {

			message_headers.set_data_key<string>("Accept", "*/*");
			message_headers.set_data_key<string>("Authorization", "");
			message_headers.set_data_key<string>("User-Agent", "Abby (https://github.com/sasanquaa, 1)");
			message_headers.set_data_key<string>("Connection", "keep-alive");
			message_headers.set_data_key<string>("Content-Type", "application/json");
			message_headers.set_data_key<int>("Content-Length", 0);
		
			message.set_data_key<string>("content", "");
		}
	} message_;

	struct _presence {
		_presence() {
			presence.set_data_key<json>("user",
			json({
				{"id", ""},
				{"username", ""},
				{"discriminator", ""}
			}));
			presence.set_data_key<json>("game", 
			json({ 
				{"name", "with SASANQUA <3"},
				{"type", 0},
				{"created_at", 0}
			}));
		}
	} presence_;
}

#endif
