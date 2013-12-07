#pragma once

namespace Mntone { namespace Rtmp {

	enum type_id_type: uint8
	{
		tid_set_chunk_size = 1,
		tid_abort_message = 2,
		tid_acknowledgement = 3,
		tid_user_control_message = 4,
		tid_window_acknowledgement_size = 5,
		tid_set_peer_bandwidth = 6,
		tid_audio_message = 8,
		tid_video_message = 9,
		tidData__message_amf3 = 15,	// 0x0f
		tid_shared_object_message_amf3 = 16, // 0x10
		tid_command_message_amf3 = 17, // 0x11
		tidData__message_amf0 = 18, // 0x12
		tid_shared_object_message_amf0 = 19, // 0x13
		tid_command_message_amf0 = 20, // 0x14
		tid_aggregate_message = 22, // 0x16
	};

} }