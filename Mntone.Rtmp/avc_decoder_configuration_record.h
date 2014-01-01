#pragma once

namespace mntone { namespace rtmp {

	struct avc_decoder_configuration_record
	{
		uint8 configuration_version;
		uint8 avc_profile_indication;
		uint8 profile_compatibility;
		uint8 avc_level_indication;
		unsigned length_size_minus_one : 2;
		unsigned : 6;
		unsigned numOfSeqeuenceParameterSets : 5;
		unsigned : 3;
	};

	//   uint16 sequenceParameterSetLength;
	//   uint8 seqeuenceParameterSetNalUnit[];
	// uint8 numOfPictureParameterSets;
	//   uint16 pictureParameterSetLength;
	//   uint8 pictureParameterSetNalUnit[];

} }