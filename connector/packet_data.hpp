#pragma once

#include <stdio.h>
#include <iostream>
// Never try something fancy like adding virtual methods or inheritence to these structs as it adds an hidden field to the struct
// making byte copying unreliable
struct PacketType {
	uint32_t type = 0;

	static constexpr auto size() {
		return sizeof(struct PacketType);
	}

	PacketType(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}

	std::string string_representation() {
		return "Packet type: " + std::to_string(type);
	}

	enum Type {
		ReadyPacket = 0,
		FramePacket = 1,
		RemoteClientTracksPacket = 2,
		ControlPacket = 3,
		TrackStatusPacket = 4,

		TilePacket = 11,
		AudioPacket = 21,

		CapturerIntrinsics = 51,
		CamInfoPacket = 61,
		DisconnectPacket = 99
		//NumTilePacket = 5
	};
};

struct PacketFrameHeader {
	uint32_t track_id = 0; // 4
	uint32_t client_id = 0; // 8
	uint32_t frame_number = 0; // 12
	uint32_t frame_length = 0; // 16
	uint32_t frame_offset = 0; // 20
	uint32_t packet_length = 0; // 24


	static constexpr auto size() {
		return sizeof(struct PacketFrameHeader);
	}

	PacketFrameHeader(uint32_t track_id, uint32_t client_id, uint32_t frame_number,
		uint32_t frame_length, uint32_t frame_offset, uint32_t packet_length) {
		this->track_id = track_id;
		this->client_id = client_id;
		this->frame_number = frame_number;
		this->frame_length = frame_length;
		this->frame_offset = frame_offset;
		this->packet_length = packet_length;
	}

	PacketFrameHeader(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}

	std::string string_representation() {
		return "[VIDEO] Client ID: " +
			std::to_string(client_id) +
			", frame number: " +
			std::to_string(frame_number) +
			", frame length: " +
			std::to_string(frame_length) +
			", frame offset: " +
			std::to_string(frame_offset) +
			", packet length: " +
			std::to_string(packet_length);
	}
protected:
	PacketFrameHeader() {};
};

struct PacketHeader {
	uint32_t client_id = 0;
	uint32_t frame_number = 0;
	uint32_t file_length = 0;
	uint32_t file_offset = 0;
	uint32_t packet_length = 0;
	uint32_t capturer_id = 0;
	uint32_t tile_id = 0;


	static constexpr auto size() {
		return sizeof(struct PacketHeader);
	}

	PacketHeader(uint32_t client_id, uint32_t frame_number,
		uint32_t file_length, uint32_t file_offset, uint32_t packet_length, uint32_t capturer_id, uint32_t tile_id) {
		this->client_id = client_id;
		this->frame_number = frame_number;
		this->file_length = file_length;
		this->file_offset = file_offset;
		this->packet_length = packet_length;
		this->capturer_id = capturer_id;
		this->tile_id = tile_id;
	}

	PacketHeader(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}

	std::string string_representation() {
		return "[VIDEO] Client ID: " +
			std::to_string(client_id) +
			", frame number: " +
			std::to_string(frame_number) +
			", file length: " +
			std::to_string(file_length) +
			", file offset: " +
			std::to_string(file_offset) +
			", packet length: " +
			std::to_string(packet_length);
	}
protected:
	PacketHeader() {};
};

struct AudioPacketHeader {
	uint32_t client_id = 0;
	uint32_t frame_number = 0;
	uint32_t file_length = 0;
	uint32_t file_offset = 0;
	uint32_t packet_length = 0;


	static constexpr auto size() {
		return sizeof(struct AudioPacketHeader);
	}

	AudioPacketHeader(uint32_t client_id, uint32_t frame_number,
		uint32_t file_length, uint32_t file_offset, uint32_t packet_length) {
		this->client_id = client_id;
		this->frame_number = frame_number;
		this->file_length = file_length;
		this->file_offset = file_offset;
		this->packet_length = packet_length;
	}

	AudioPacketHeader(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}

	std::string string_representation() {
		return "[AUDIO] Client ID: " +
			std::to_string(client_id) +
			", frame number: " +
			std::to_string(frame_number) +
			", file length: " +
			std::to_string(file_length) +
			", file offset: " +
			std::to_string(file_offset) +
			", packet length: " +
			std::to_string(packet_length);
	}
protected:
	AudioPacketHeader() {};
};

struct TrackStatusChangedHeader {
	uint32_t client_id = 0;
	uint32_t last_frame_nr = 0;
	uint32_t capturer_id = 0;
	uint32_t tile_nr = 0;
	bool is_video = 0;
	bool is_added = 0;

	static constexpr auto size() {
		return sizeof(struct TrackStatusChangedHeader);
	}

	TrackStatusChangedHeader(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}
};

struct CameraIntrinsicsHeader {
	uint32_t client_id = 0;
	uint32_t capturer_id = 0;
	uint32_t capturer_type = 0;

	static constexpr auto size() {
		return sizeof(struct CameraIntrinsicsHeader);
	}

	CameraIntrinsicsHeader(char** buf, size_t& avail) {
		std::memcpy(data(), *buf, size());
		*buf += size();
		avail -= size();
	}

	char* data() {
		return reinterpret_cast<char*>(this);
	}
};
