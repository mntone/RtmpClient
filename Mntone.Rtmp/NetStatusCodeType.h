#pragma once

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	[Platform::Metadata::Flags]
	public enum class NetStatusCodeType: uint32
	{
		// # NetConnection (0x1)
		NetConnection = 0x10000000,

		// ## Connect (0x001)
		NetConnectionConnect = 0x10010000,
		NetConnectionConnectSuccess = 0x10010001,
		NetConnectionConnectClosed = 0x10010002,
		NetConnectionConnectFailed = 0x10010004,
		NetConnectionConnectRejected = 0x10010008,
		NetConnectionConnectInvalidApp = 0x10010010,
		NetConnectionConnectAppShutdown = 0x10010020,
		NetConnectionConnectOther = 0x10018000,

		// ## Call (0x002)
		NetConnectionCall = 0x10020000,
		NetConnectionCallFailed = 0x10020001,
		NetConnectionCallProhibited = 0x10020002,
		NetConnectionCallBadVersion = 0x10020004,
		NetConnectionCallOther = 0x10028000,

		// ## Other (0x800)
		NetConnectionOther = 0x18000000,

		// # NetStream (0x2)
		NetStream = 0x20000000,

		// ## Play (0x001)
		NetStreamPlay = 0x20010000,
		NetStreamPlayStart = 0x20010001,
		NetStreamPlayStop = 0x20010002,
		NetStreamPlayReset = 0x20010004,
		NetStreamPlayPublishNotify = 0x20010008,
		NetStreamPlayUnpublishNotify = 0x20010010,
		NetStreamPlayTransition = 0x20010020,
		NetStreamPlaySwitch = 0x20010040,
		NetStreamPlayComplete = 0x20010080,
		NetStreamPlayTransitionComplete = 0x20010100,
		NetStreamPlayInsufficientBandwidth = 0x20010200,
		NetStreamPlayFailed = 0x20010400,
		NetStreamPlayStreamNotFound = 0x20010800,
		NetStreamPlayFileStructureInvalid = 0x20011000,
		NetStreamPlayNoSupportedTrackFound = 0x20012000,
		NetStreamPlayOther = 0x20018000,

		// ## Pause (0x002)
		NetStreamPause = 0x20020000,
		NetStreamPauseNotify = 0x20020001,
		NetStreamPauseOther = 0x20028000,

		// ## Unpause (0x004)
		NetStreamUnpause = 0x20040000,
		NetStreamUnpauseNotify = 0x20040001,
		NetStreamUnpauseOther = 0x20048000,

		// ## Seek (0x008)
		NetStreamSeek = 0x20080000,
		NetStreamSeekNotify = 0x20080001,
		NetStreamSeekFailed = 0x20080002,
		NetStreamSeekInvalidTime = 0x20080004,
		NetStreamSeekOther = 0x20088000,

		// ## Publish (0x010)
		NetStreamPublish = 0x20100000,
		NetStreamPublishStart = 0x20100001,
		NetStreamPublishIdle = 0x20100002,
		NetStreamPublishBadName = 0x20100004,
		NetStreamPublishOther = 0x20108000,

		// ## Unpublish (0x020)
		NetStreamUnpublish = 0x20200000,
		NetStreamUnpublishSuccess = 0x20200001,
		NetStreamUnpublishOther = 0x20208000,

		// ## Record (0x040)
		NetStreamRecord = 0x20400000,
		NetStreamRecordStart = 0x20400001,
		NetStreamRecordStop = 0x20400002,
		NetStreamRecordNoAccess = 0x20400004,
		NetStreamRecordFailed = 0x20400008,
		NetStreamRecordDiskQuotaExceeded = 0x20400010,
		NetStreamRecordOther = 0x20408000,

		// ## Buffer (0x080)
		NetStreamBuffer = 0x20800000,
		NetStreamBufferEmpty = 0x20800001,
		NetStreamBufferFull = 0x20800002,
		NetStreamBufferFlush = 0x20800004,
		NetStreamBufferOther = 0x20808000,

		// ## MulticastStream (0x100)
		NetStreamMulticastStream = 0x21000000,
		NetStreamMulticastStreamReset = 0x21000001,
		NetStreamMulticastStreamOther = 0x21008000,

		// ## Failed (0x200)
		NetStreamFailed = 0x22000000,

		// ## Other (0x800)
		NetStreamOther = 0x28000000,

		// # SharedObject (0x4)
		SharedObject = 0x40000000,
		SharedObjectFlush = 0x40010000,
		SharedObjectFlushSuccess = 0x40010001,
		SharedObjectFlushFailed = 0x40010002,
		SharedObjectFlushOther = 0x40018000,
		SharedObjectBadPersistence = 0x40020000,
		SharedObjectUriMismatch = 0x40040000,
		SharedObjectOther = 0x48000000,

		// # Other (0x8)
		Other = 0x80000000,

		// # Level Mask
		Level1Mask = 0xf0000000,
		Level2Mask = 0xffff0000,
	};

} }