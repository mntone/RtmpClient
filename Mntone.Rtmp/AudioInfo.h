#pragma once
#include "sound_info.h"
#include "AudioFormat.h"

namespace Mntone { namespace Rtmp {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AudioInfo sealed
	{
	internal:
		AudioInfo() { }

		void SetInfo( sound_info soundInfo )
		{
			switch( soundInfo.rate )
			{
			case sound_rate::sr_5_5khz: SampleRate_ = 5513; break;
			case sound_rate::sr_11khz: SampleRate_ = 11025; break;
			case sound_rate::sr_22khz: SampleRate_ = 22050; break;
			case sound_rate::sr_44khz: SampleRate_ = 44100; break;
			default: throw ref new Platform::InvalidArgumentException();
			}

			switch( soundInfo.format )
			{
			case sound_format::sf_lpcm:	Format_ = AudioFormat::Lpcm; break;
			case sound_format::sf_adpcm: Format_ = AudioFormat::Adpcm; break;
			case sound_format::sf_mp3: Format_ = AudioFormat::Mp3; break;
			case sound_format::sf_lpcm_little_endian: Format_ = AudioFormat::LpcmLe; break;
			case sound_format::sf_nellymoser16khz_mono:
				Format_ = AudioFormat::Nellymoser;
				SampleRate_ = 16000;
				break;
			case sound_format::sf_nellymoser8khz_mono:
				Format_ = AudioFormat::Nellymoser;
				SampleRate_ = 8000;
				break;
			case sound_format::sf_nellymoser: Format_ = AudioFormat::Nellymoser; break;
			case sound_format::sf_g711_alaw_logarithmic_pcm: Format_ = AudioFormat::G711Alaw; break;
			case sound_format::sf_g711_mulaw_logarithmic_pcm: Format_ = AudioFormat::G711Mulaw; break;
			case sound_format::sf_speex: Format_ = AudioFormat::Speex; break;
			case sound_format::sf_mp38khz:
				Format_ = AudioFormat::Mp3;
				SampleRate_ = 8000;
				break;
			default: throw ref new Platform::InvalidArgumentException();
			}

			ChannelCount_ = soundInfo.type == sound_type::st_stereo ? 2 : 1;
			BitsPerSample_ = soundInfo.size == sound_size::ss_16bit ? 16 : 8;
		}

	public:
		property AudioFormat Format
		{
			AudioFormat get() { return Format_; }
		internal:
			void set( AudioFormat value ) { Format_ = value; }
		}
		property uint32 SampleRate
		{
			uint32 get() { return SampleRate_; }
		internal:
			void set( uint32 value ) { SampleRate_ = value; }
		}
		property uint16 ChannelCount
		{
			uint16 get() { return ChannelCount_; }
		internal:
			void set( uint16 value ) { ChannelCount_ = value; }
		}
		property uint16 Bitrate
		{
			uint16 get() { return Bitrate_; }
		internal:
			void set( uint16 value ) { Bitrate_ = value; }
		}
		property uint16 BitsPerSample
		{
			uint16 get() { return BitsPerSample_; }
		internal:
			void set( uint16 value ) { BitsPerSample_ = value; }
		}

	private:
		AudioFormat Format_;
		uint32 SampleRate_;
		uint16 ChannelCount_, Bitrate_, BitsPerSample_;
	};

} }