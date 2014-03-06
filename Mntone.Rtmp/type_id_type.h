#pragma once

namespace mntone { namespace rtmp {

	enum class type_id_type: uint8
	{
		set_chunk_size = 1,
		abort_message = 2,
		acknowledgement = 3,
		user_control_message = 4,
		window_acknowledgement_size = 5,
		set_peer_bandwidth = 6,
		audio_message = 8,
		video_message = 9,
		data_message_amf3 = 15,	// 0x0f
		shared_object_message_amf3 = 16, // 0x10
		command_message_amf3 = 17, // 0x11
		data_message_amf0 = 18, // 0x12
		shared_object_message_amf0 = 19, // 0x13
		command_message_amf0 = 20, // 0x14
		aggregate_message = 22, // 0x16
	};

} }