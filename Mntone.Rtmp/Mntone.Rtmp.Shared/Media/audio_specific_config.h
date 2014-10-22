#pragma once
#include "aac_sampling_frequency.h"

namespace mntone { namespace rtmp { namespace media {

	class audio_specific_config final
	{
	public:
		audio_specific_config( const uint8* data, const size_t size );

		void load( const uint8* data, const size_t size );

	private:
		int32 get_sampling_frequency( aac_sampling_frequency value ) const;

	public:
		int8 object_type() const noexcept { return object_type_; }
		int32 sampling_frequency() const noexcept { return sampling_frequency_; }
		int8 channel_configuration() const noexcept { return channel_configuration_; }

		int8 extension_object_type() const noexcept { return extension_object_type_; }
		bool is_sbr() const noexcept { return is_sbr_; }
		bool is_ps() const noexcept { return is_ps_; }
		int32 extension_sampling_frequency() const noexcept { return extension_sampling_frequency_; }
		int8 extension_channel_configuration() const noexcept { return extension_channel_configuration_; }

	private:
		int8 object_type_;
		int32 sampling_frequency_;
		int8 channel_configuration_;

		int8 extension_object_type_;
		bool is_sbr_, is_ps_;
		int32 extension_sampling_frequency_;
		int8 extension_channel_configuration_;
	};

} } }