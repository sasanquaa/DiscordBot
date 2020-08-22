#pragma once
#ifndef RATE_LIMIT
#define RATE_LIMIT

#include <unordered_map>
#include <unordered_set>
#include <queue>

class rate_limit {

public:
	rate_limit();

	~rate_limit();

	void add_to_bucket(std::string, std::string);

	bool is_exceed_limit();

private:

	std::unordered_map<std::string, std::unordered_set<std::string>> m_rate_bucket;
	
};

#endif
