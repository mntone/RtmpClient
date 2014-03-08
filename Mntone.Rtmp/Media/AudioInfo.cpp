#include "pch.h"
#include "AudioInfo.h"

using namespace mntone::rtmp::media;
using namespace Mntone::Rtmp::Media;

void AudioInfo::SetInfo( const mntone::rtmp::media::sound_info& soundInfo )
{
	switch( soundInfo.rate )
	{
	case sound_rate::r5_5khz: SampleRate_ = 5513; break;
	case sound_rate::r11khz: SampleRate_ = 11025; break;
	case sound_rate::r22khz: SampleRate_ = 22050; break;
	case sound_rate::r44khz: SampleRate_ = 44100; break;
	default: throw ref new Platform::InvalidArgumentException();
	}

	switch( soundInfo.format )
	{
	case sound_format::linear_pcm:	Format_ = AudioFormat::Lpcm; break;
	case sound_format::adaptive_differential_pcm: Format_ = AudioFormat::Adpcm; break;
	case sound_format::mp3: Format_ = AudioFormat::Mp3; break;
	case sound_format::linear_pcm_little_endian: Format_ = AudioFormat::LpcmLe; break;
	case sound_format::nellymoser_16khz_mono:
		Format_ = AudioFormat::Nellymoser;
		SampleRate_ = 16000;
		break;
	case sound_format::nellymoser_8khz_mono:
		Format_ = AudioFormat::Nellymoser;
		SampleRate_ = 8000;
		break;
	case sound_format::nellymoser: Format_ = AudioFormat::Nellymoser; break;
	case sound_format::g711_alaw_logarithmic_pcm: Format_ = AudioFormat::G711Alaw; break;
	case sound_format::g711_mulaw_logarithmic_pcm: Format_ = AudioFormat::G711Mulaw; break;
	case sound_format::speex: Format_ = AudioFormat::Speex; break;
	case sound_format::mp3_8khz:
		Format_ = AudioFormat::Mp3;
		SampleRate_ = 8000;
		break;
	default: throw ref new Platform::InvalidArgumentException();
	}

	ChannelCount_ = soundInfo.type == sound_type::stereo ? 2 : 1;
	BitsPerSample_ = soundInfo.size == sound_size::s16bit ? 16 : 8;
}