#pragma once
#include <stdio.h>
#include <iostream>
#include <cstring>

struct CapturerIntrinsics {
    unsigned int  width = 0;
    unsigned int  height = 0;
    unsigned int  model = 0;
    float         ppx = 0.0f;
    float         ppy = 0.0f;
    float         fx = 0.0f;
    float         fy = 0.0f;
    float         coeffs[5]{};

	static constexpr auto size() {
		return 12*4;
	}

	CapturerIntrinsics(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}
};
