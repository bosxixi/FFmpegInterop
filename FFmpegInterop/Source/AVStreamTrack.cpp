#include "pch.h"
#include "AVStreamTrack.h"

namespace FFmpegInterop
{
	AVStreamTrack::AVStreamTrack(int number)
		:
		//avDict(nullptr)
		//		, avIOCtx(nullptr)
		//		, avFormatCtx(nullptr)
		avAudioCodecCtx(nullptr)
		//, avVideoCodecCtx(nullptr)
		, audioStreamIndex(AVERROR_STREAM_NOT_FOUND)
		//, videoStreamIndex(AVERROR_STREAM_NOT_FOUND)
		//, thumbnailStreamIndex(AVERROR_STREAM_NOT_FOUND)
		//, fileStreamData(nullptr)
		//, fileStreamBuffer(nullptr)
		, audioStreamDescriptor(nullptr)
		, audioSampleProvider(nullptr)
		, audioCodecName(nullptr)
		, m_avAudioCodec(nullptr)
		, _value(number)
	{

	}

	int AVStreamTrack::GetValue() { return _value; }
	int AVStreamTrack::GetAudioStreamIndex() { return audioStreamIndex; }
	AVCodec* AVStreamTrack::GetAVAudioCodec() { return m_avAudioCodec; }
	MediaSampleProvider^ AVStreamTrack::GetMediaSampleProvider() { return audioSampleProvider; }
	AudioStreamDescriptor^ AVStreamTrack::GetAudioStreamDescriptor() { return audioStreamDescriptor; }
	AVCodecContext* AVStreamTrack::GetAVCodecContext() { return avAudioCodecCtx; }
	bool AVStreamTrack::CreateTracks(int value, AVFormatContext* paramavFormatCtx, AVCodec* avAudioCodec, bool forceAudioDecode, bool audioPassthrough, FFmpegReader^ m_pReader)
	{
		audioStreamIndex = value;
		avFormatCtx_ptr = std::make_shared<AVFormatContext*>(paramavFormatCtx);
		m_avAudioCodec = avAudioCodec;
		m_pReader_ptr = std::make_shared<FFmpegReader^>(m_pReader);

		avAudioCodecCtx = avcodec_alloc_context3(m_avAudioCodec);
		if (!avAudioCodecCtx)
		{
			avformat_close_input(&*avFormatCtx_ptr);
			return false;
		}

		if (avcodec_parameters_to_context(avAudioCodecCtx, (*avFormatCtx_ptr)->streams[audioStreamIndex]->codecpar) < 0)
		{
			avformat_close_input(&*avFormatCtx_ptr);
			avcodec_free_context(&avAudioCodecCtx);
			return false;
		}

		if (avcodec_open2(avAudioCodecCtx, avAudioCodec, NULL) < 0)
		{
			avAudioCodecCtx = nullptr;
			return false;
		}

		auto hr = AVStreamTrack::CreateAudioStreamDescriptor(forceAudioDecode, audioPassthrough);

		if (SUCCEEDED(hr))
		{
			hr = audioSampleProvider->AllocateResources();
		}
		if (SUCCEEDED(hr))
		{
			// Convert audio codec name for property
			hr = ConvertCodecName(m_avAudioCodec->name, &audioCodecName);
		}

		avAudioCodecCtx = nullptr;
		audioSampleProvider = nullptr;
		m_pReader_ptr = nullptr;
		return true;
	}

	HRESULT AVStreamTrack::CreateAudioStreamDescriptor(bool forceAudioDecode, bool audioPassthrough)
	{
		if (audioPassthrough)
		{
			audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreatePcm(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, 16));
			audioSampleProvider = ref new MediaSampleProvider(*m_pReader_ptr, *avFormatCtx_ptr, avAudioCodecCtx);
			audioSampleProvider->SetCurrentStreamIndex(audioStreamIndex);
			return (audioStreamDescriptor != nullptr && audioSampleProvider != nullptr) ? S_OK : E_OUTOFMEMORY;
		}
		/*if (avAudioCodecCtx->codec_id == AV_CODEC_ID_AAC && !forceAudioDecode)
		{
			if (avAudioCodecCtx->extradata_size == 0)
			{
				audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreateAacAdts(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate));
			}
			else
			{
				audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreateAac(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate));
			}
			audioSampleProvider = ref new MediaSampleProvider(*m_pReader_ptr, *avFormatCtx_ptr, avAudioCodecCtx);
		}
		else*/ if (avAudioCodecCtx->codec_id == AV_CODEC_ID_MP3 && !forceAudioDecode)
		{
			audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreateMp3(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate));
			audioSampleProvider = ref new MediaSampleProvider(*m_pReader_ptr, *avFormatCtx_ptr, avAudioCodecCtx);
		}
		else
		{
			// We always convert to 16-bit audio so set the size here
			audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreatePcm(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, 16));
			audioSampleProvider = ref new UncompressedAudioSampleProvider(*m_pReader_ptr, *avFormatCtx_ptr, avAudioCodecCtx);
		}
		audioSampleProvider->SetCurrentStreamIndex(audioStreamIndex);
		return (audioStreamDescriptor != nullptr && audioSampleProvider != nullptr) ? S_OK : E_OUTOFMEMORY;
	}

	HRESULT AVStreamTrack::ConvertCodecName(const char* codecName, String^ *outputCodecName)
	{
		HRESULT hr = S_OK;

		// Convert codec name from const char* to Platform::String
		auto codecNameChars = codecName;
		size_t newsize = strlen(codecNameChars) + 1;
		wchar_t * wcstring = new(std::nothrow) wchar_t[newsize];
		if (wcstring == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}

		if (SUCCEEDED(hr))
		{
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, wcstring, newsize, codecNameChars, _TRUNCATE);
			*outputCodecName = ref new Platform::String(wcstring);
			delete[] wcstring;
		}

		return hr;
	}
}