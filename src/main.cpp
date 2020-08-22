#include <discord_bot.h>

using json = nlohmann::json;



int main() {

	discord_bot bot("discord.com", "NzM3MTY5OTg5MDk3ODgxNzEz.Xx5dDQ.MIcV0DDdO6Lp9ee-cDZAKyE8SZs");
	
	bot.on_open([&]() {
		bot.log("-----Connected to the Gateway-----");
	
	}).on_message([&](auto e, auto event_name, auto& data ) {

		bot.log("Event code: " + std::to_string(e) + "\n");
	
		bot.log("Event received: " + event_name + "\n");

		bot.log("Data received: \n" + static_cast<nlohmann::json>(data).dump(2));

		if (event_name == "MESSAGE_CREATE") {

			std::string author = data["author"]["username"];
			std::string msg = data["content"];

			if (msg == "!sasanqua") {
				bot.create_message("Love SASANQUA <3", data["channel_id"]);
			}
			else if (msg == "!nhacanime") {
				bot.create_message("Nhac. anime main ba' co' dau` bui` re? rach' giau' nghe` nhu 2 con suc' vat.: y2u.be/dQw4w9WgXcQ", data["channel_id"]);
			}

			bot.log("[" + author + "] send a message: " + msg);
		}

	}).on_close([&]() {
		
	}).listen();
	

	return 0;

}