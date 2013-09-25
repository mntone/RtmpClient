#pragma once

namespace Mntone { namespace Rtmp {

	struct avc_decoder_configuration_record
	{
		unsigned length_size_minus_one : 2;
		unsigned : 6;
		uint8 avc_level_indication;
		uint8 profile_compatibility;
		uint8 avc_profile_indication;
		uint8 configuration_version;
	};

	// uint8 reserved: 3;
	// uint8 numOfSeqeuenceParameterSets: 5;
	//   uint16 sequenceParameterSetLength;
	//   uint8 seqeuenceParameterSetNalUnit[];
	// uint8 numOfPictureParameterSets;
	//   uint16 pictureParameterSetLength;
	//   uint8 pictureParameterSetNalUnit[];

} }