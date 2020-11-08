#include <fstream>
#include <string>
#include <assert.h>
#include <misc/zlib.h>
#include "../test/rest/rest.h"
#include "../test/threading/thread_test.cpp"

using json = nlohmann::json;

/*
#define CHUNK 256000

int def(ifstream& in, ofstream& out, int level) {
	int ret, flush;
	unsigned have;
	z_stream stream;
	unsigned char inBuf[CHUNK];
	unsigned char outBuf[CHUNK];

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	ret = deflateInit(&stream, level);
	if (ret != Z_OK)
		return ret;

	do {
		
		in.read((char*) inBuf, CHUNK);
		
		stream.avail_in = in.gcount();
		std::cout << "Read bytes: " << stream.avail_in << "\n";

		flush = in.eof() ? Z_FINISH : Z_NO_FLUSH;
		stream.next_in = inBuf;

		do {
			stream.avail_out = CHUNK;
			stream.next_out = outBuf;
			deflate(&stream, flush);
			have = CHUNK - stream.avail_out;
			out.write((char*) outBuf, have);
			if (!out.good()) {
				deflateEnd(&stream);
				return Z_ERRNO;
			}
			
		} while (stream.avail_out == 0);

	} while (flush != Z_FINISH);

	
	return 1;
}

int inf(ifstream &in, ofstream &out) {
	int ret;
	unsigned have;
	z_stream stream;
	unsigned char inBuf[CHUNK];
	unsigned char outBuf[CHUNK];

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	ret = inflateInit(&stream);
	
	do {
		in.read((char*)inBuf, CHUNK);
		
		stream.avail_in = in.gcount();
		if (stream.avail_in == 0) break;
		std::cout << "Read bytes: " << stream.avail_in << "\n";
		stream.next_in = inBuf;

		do {
			stream.avail_out = CHUNK;
			stream.next_out = outBuf;
			ret = inflate(&stream, Z_NO_FLUSH);
			have = CHUNK - stream.avail_out;
			out.write((char*) outBuf, have);
		} while (stream.avail_out == 0);

	} while (ret != Z_STREAM_END);

	inflateEnd(&stream);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
*/

int main() {
	
	rest::rest r;
	r.open();
	while (true) {
		std::string msg;
		std::getline(std::cin, msg);
		nlohmann::json d;
		if (msg.rfind("-p", 0) == 0 || msg.rfind("-play", 0) == 0) {
			r.send("POST", "/api/channels/737294540670500935/messages", 
				{{"Authorization", "Bot NzM3MTY5OTg5MDk3ODgxNzEz.Xx5dDQ.b3HzsVCoyUG84COEAEpp09O3Jj4"}},
				{{"content", msg}}
			);
		}
		else {
			r.send("POST", "/api/channels/748138432571637809/messages", 
				{{"Authorization", "Bot NzM3MTY5OTg5MDk3ODgxNzEz.Xx5dDQ.b3HzsVCoyUG84COEAEpp09O3Jj4"}},
				{{"content", msg}}
			);
		}
		//std::cout << d.dump(2);
	}


	/*
	discord_bot bot("discord.com", "NzM3MTY5OTg5MDk3ODgxNzEz.Xx5dDQ.b3HzsVCoyUG84COEAEpp09O3Jj4");
	
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

	*/

	return 0;
}