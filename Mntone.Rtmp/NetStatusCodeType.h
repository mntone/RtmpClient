#pragma once

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	[Platform::Metadata::Flags]
	public enum class NetStatusCodeType: uint32
	{
		// # NetConnection (0x1)
		NetConnection = 0x10000000,

		// ## Connect (0x001)
		NetConnection_Connect = 0x10010000,
		NetConnection_Connect_Success = 0x10010001,
		NetConnection_Connect_Closed = 0x10010002,
		NetConnection_Connect_Failed = 0x10010004,
		NetConnection_Connect_Rejected = 0x10010008,
		NetConnection_Connect_InvalidApp = 0x10010010,
		NetConnection_ConnectApp_Shutdown = 0x10010020,
		NetConnection_Connect_Other = 0x10018000,

		// ## Call (0x002)
		NetConnection_Call = 0x10020000,
		NetConnection_Call_Failed = 0x10020001,
		NetConnection_Call_Prohibited = 0x10020002,
		NetConnection_Call_BadVersion = 0x10020004,
		NetConnection_Call_Other = 0x10028000,

		// ## Other (0x800)
		NetConnection_Other = 0x18000000,

		// # NetStream (0x2)
		NetStream = 0x20000000,

		// ## Play (0x001)
		NetStream_Play = 0x20010000,
		NetStream_Play_Start = 0x20010001,
		NetStream_Play_Stop = 0x20010002,
		NetStream_Play_Reset = 0x20010004,
		NetStream_Play_PublishNotify = 0x20010008,
		NetStream_Play_UnpublishNotify = 0x20010010,
		NetStream_Play_Transition = 0x20010020,
		NetStream_Play_Switch = 0x20010040,
		NetStream_Play_Complete = 0x20010080,
		NetStream_Play_TransitionComplete = 0x20010100,
		NetStream_Play_InsufficientBandwidth = 0x20010200,
		NetStream_Play_Failed = 0x20010400,
		NetStream_Playstream_NotFound = 0x20010800,
		NetStream_Play_FileStructureInvalid = 0x20011000,
		NetStream_Play_NoSupportedTrackFound = 0x20012000,
		NetStream_Play_Other = 0x20018000,

		// ## Pause (0x002)
		NetStream_Pause = 0x20020000,
		NetStream_Pause_Notify = 0x20020001,
		NetStream_Pause_Other = 0x20028000,

		// ## Unpause (0x004)
		NetStream_Unpause = 0x20040000,
		NetStream_Unpause_Notify = 0x20040001,
		NetStream_Unpause_Other = 0x20048000,

		// ## Seek (0x008)
		NetStream_Seek = 0x20080000,
		NetStream_Seek_Notify = 0x20080001,
		NetStream_Seek_Failed = 0x20080002,
		NetStream_Seek_InvalidTime = 0x20080004,
		NetStream_Seek_Other = 0x20088000,

		// ## Publish (0x010)
		NetStream_Publish = 0x20100000,
		NetStream_Publish_Start = 0x20100001,
		NetStream_Publish_Idle = 0x20100002,
		NetStream_Publish_BadName = 0x20100004,
		NetStream_Publish_Other = 0x20108000,

		// ## Unpublish (0x020)
		NetStream_Unpublish = 0x20200000,
		NetStream_Unpublish_Success = 0x20200001,
		NetStream_Unpublish_Other = 0x20208000,

		// ## Record (0x040)
		NetStream_Record = 0x20400000,
		NetStream_Record_Start = 0x20400001,
		NetStream_Record_Stop = 0x20400002,
		NetStream_Record_NoAccess = 0x20400004,
		NetStream_Record_Failed = 0x20400008,
		NetStream_Record_DiskQuotaExceeded = 0x20400010,
		NetStream_Record_Other = 0x20408000,

		// ## Buffer (0x080)
		NetStream_Buffer = 0x20800000,
		NetStream_Buffer_Empty = 0x20800001,
		NetStream_Buffer_Full = 0x20800002,
		NetStream_Buffer_Flush = 0x20800004,
		NetStream_Buffer_Other = 0x20808000,

		// ## MulticastStream (0x100)
		NetStream_MulticastStream = 0x21000000,
		NetStream_MulticastStream_Reset = 0x21000001,
		NetStream_MulticastStream_Other = 0x21008000,

		// ## Failed (0x200)
		NetStream_Failed = 0x22000000,

		// ## Other (0x800)
		NetStream_Other = 0x28000000,

		// # SharedObject (0x4)
		SharedObject = 0x40000000,
		SharedObject_Flush = 0x40010000,
		SharedObject_Flush_Success = 0x40010001,
		SharedObject_Flush_Failed = 0x40010002,
		SharedObject_Flush_Other = 0x40018000,
		SharedObject_BadPersistence = 0x40020000,
		SharedObjectUri_Mismatch = 0x40040000,
		SharedObject_Other = 0x48000000,

		// # Other (0x8)
		Other = 0x80000000,

		// # Level Mask
		Level1Mask = 0xf0000000,
		Level2Mask = 0xffff0000,
	};

} }