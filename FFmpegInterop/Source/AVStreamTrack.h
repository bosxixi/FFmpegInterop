#pragma once
#include <queue>
#include <mutex>

#include "MediaSampleProvider.h"
#include "UncompressedAudioSampleProvider.h"

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Core;
using namespace FFmpegInterop;
using namespace Windows::Media::MediaProperties;

namespace FFmpegInterop
{
	ref class MediaSampleProvider;
	ref class UncompressedAudioSampleProvider;
	class AVStreamTrack sealed
	{
	public:

		AVStreamTrack(int number);
		int GetValue();
		int GetAudioStreamIndex();
		MediaSampleProvider^ GetMediaSampleProvider();
		AudioStreamDescriptor^ GetAudioStreamDescriptor();
		AVCodecContext* GetAVCodecContext();
		AVCodec* GetAVAudioCodec();
		bool CreateTracks(int value, AVFormatContext* paramavFormatCtx, AVCodec* avAudioCodec, bool forceAudioDecode, FFmpegReader^ m_pReader);

	private:
		HRESULT AVStreamTrack::CreateAudioStreamDescriptor(bool forceAudioDecode);
		HRESULT AVStreamTrack::ConvertCodecName(const char* codecName, String^ *outputCodecName);
		int _value;
		int audioStreamIndex;
		AVCodecContext* avAudioCodecCtx;
		std::shared_ptr<AVFormatContext*> avFormatCtx_ptr;
		AudioStreamDescriptor^ audioStreamDescriptor;
		MediaSampleProvider^ audioSampleProvider;
		String^ audioCodecName;
		AVCodec* m_avAudioCodec;

		std::shared_ptr<FFmpegReader^> m_pReader_ptr;
	};
}