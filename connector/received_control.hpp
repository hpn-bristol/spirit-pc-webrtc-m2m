#pragma once

#include <cstdint>
#include <cstring>
#include <queue>

class ReceivedControl {

public:
	ReceivedControl(char* b, uint32_t _packet_len) {
		data.resize(_packet_len);
		std::memcpy(data.data(), b, _packet_len);
	}

	~ReceivedControl() {}

	char* get_data() {
		return data.data();
	}

	size_t get_data_length() const {
		return data.size();
	}

	const std::vector<char>& get_data_v() const {
		return data;
	}

private:
	std::vector<char> data;
};
