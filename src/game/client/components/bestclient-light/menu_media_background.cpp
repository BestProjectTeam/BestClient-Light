/* Copyright © 2026 BestProject Team */
#include "menu_media_background.h"

#include <base/fs.h>
#include <base/math.h>
#include <base/mem.h>
#include <base/str.h>
#include <base/time.h>
#include <base/types.h>
#include <base/vmath.h>

#include <engine/shared/config.h>

#include <game/localization.h>

#include <algorithm>
#include <limits>

#if defined(CONF_VIDEORECORDER)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#endif

namespace
{
	constexpr int MENU_MEDIA_MAX_VIDEO_FRAME_MS = 250;
	constexpr int MENU_MEDIA_DEFAULT_VIDEO_FRAME_MS = 33;
	constexpr int64_t MENU_MEDIA_PTS_UNSET = std::numeric_limits<int64_t>::min();
	constexpr int MENU_VIDEO_OK = 0;
	constexpr int MENU_VIDEO_ERR_OPEN = 1;
	constexpr int MENU_VIDEO_ERR_INFO = 2;
	constexpr int MENU_VIDEO_ERR_STREAM = 3;
	constexpr int MENU_VIDEO_ERR_CODEC = 4;
	constexpr int MENU_VIDEO_ERR_DECODER = 5;
	constexpr int MENU_VIDEO_ERR_DIM = 6;
	constexpr int MENU_VIDEO_ERR_ALLOC = 7;
	constexpr int MENU_VIDEO_ERR_RGBA = 8;
	constexpr int MENU_VIDEO_ERR_SCALE = 9;
	constexpr int MENU_VIDEO_ERR_DECODE = 10;

#if defined(CONF_VIDEORECORDER)
	bool DecodeFirstFrameFromFile(const char *pAbsolutePath, CImageInfo &ImageOut)
	{
		ImageOut.Free();

		if(pAbsolutePath == nullptr || pAbsolutePath[0] == '\0')
			return false;

		AVFormatContext *pFormatCtx = nullptr;
		AVCodecContext *pCodecCtx = nullptr;
		SwsContext *pSwsCtx = nullptr;
		AVPacket *pPacket = nullptr;
		AVFrame *pFrame = nullptr;
		AVFrame *pFrameRgba = nullptr;
		int VideoStream = -1;
		bool Success = false;
		int SrcW = 0;
		int SrcH = 0;
		size_t FrameBytes = 0;

		auto CopyFrame = [&]() -> bool {
			if(!pFrame || !pFrameRgba || !pSwsCtx || SrcW <= 0 || SrcH <= 0 || FrameBytes == 0)
				return false;
			if(av_frame_make_writable(pFrameRgba) < 0)
				return false;
			const int ScaledLines = sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, SrcH, pFrameRgba->data, pFrameRgba->linesize);
			if(ScaledLines <= 0 || pFrameRgba->linesize[0] <= 0 || (size_t)pFrameRgba->linesize[0] < (size_t)SrcW * 4ull)
				return false;

			ImageOut.Free();
			ImageOut.m_Width = SrcW;
			ImageOut.m_Height = SrcH;
			ImageOut.m_Format = CImageInfo::FORMAT_RGBA;
			ImageOut.m_pData = (uint8_t *)malloc(FrameBytes);
			if(!ImageOut.m_pData)
				return false;

			for(int y = 0; y < SrcH; ++y)
			{
				mem_copy(
					ImageOut.m_pData + (size_t)y * (size_t)SrcW * 4ull,
					pFrameRgba->data[0] + (size_t)y * (size_t)pFrameRgba->linesize[0],
					(size_t)SrcW * 4ull);
			}
			return true;
		};

		do
		{
			if(avformat_open_input(&pFormatCtx, pAbsolutePath, nullptr, nullptr) != 0)
				break;
			if(avformat_find_stream_info(pFormatCtx, nullptr) < 0)
				break;

			VideoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
			if(VideoStream < 0)
				break;

			const AVStream *pStream = pFormatCtx->streams[VideoStream];
			const AVCodec *pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
			if(!pCodec)
				break;

			pCodecCtx = avcodec_alloc_context3(pCodec);
			if(!pCodecCtx || avcodec_parameters_to_context(pCodecCtx, pStream->codecpar) < 0 || avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
				break;

			SrcW = pCodecCtx->width;
			SrcH = pCodecCtx->height;
			if(SrcW <= 0 || SrcH <= 0)
				break;
			if((size_t)SrcW > std::numeric_limits<size_t>::max() / ((size_t)SrcH * 4ull))
				break;
			FrameBytes = (size_t)SrcW * (size_t)SrcH * 4ull;
			if(FrameBytes == 0)
				break;

			pSwsCtx = sws_getContext(SrcW, SrcH, pCodecCtx->pix_fmt, SrcW, SrcH, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
			if(!pSwsCtx)
				break;

			pPacket = av_packet_alloc();
			pFrame = av_frame_alloc();
			pFrameRgba = av_frame_alloc();
			if(!pPacket || !pFrame || !pFrameRgba)
				break;

			pFrameRgba->format = AV_PIX_FMT_RGBA;
			pFrameRgba->width = SrcW;
			pFrameRgba->height = SrcH;
			if(av_frame_get_buffer(pFrameRgba, 1) < 0)
				break;

			while(av_read_frame(pFormatCtx, pPacket) >= 0)
			{
				if(pPacket->stream_index == VideoStream)
				{
					if(avcodec_send_packet(pCodecCtx, pPacket) < 0)
					{
						av_packet_unref(pPacket);
						break;
					}
					while(avcodec_receive_frame(pCodecCtx, pFrame) == 0)
					{
						if(CopyFrame())
						{
							Success = true;
							break;
						}
					}
				}
				av_packet_unref(pPacket);
				if(Success)
					break;
			}

			if(!Success && avcodec_send_packet(pCodecCtx, nullptr) >= 0)
			{
				while(avcodec_receive_frame(pCodecCtx, pFrame) == 0)
				{
					if(CopyFrame())
					{
						Success = true;
						break;
					}
				}
			}
		} while(false);

		if(!Success)
			ImageOut.Free();

		if(pFrameRgba)
			av_frame_free(&pFrameRgba);
		if(pFrame)
			av_frame_free(&pFrame);
		if(pPacket)
			av_packet_free(&pPacket);
		if(pSwsCtx)
			sws_freeContext(pSwsCtx);
		if(pCodecCtx)
			avcodec_free_context(&pCodecCtx);
		if(pFormatCtx)
			avformat_close_input(&pFormatCtx);

		return Success;
	}

	struct SFfmpegVideo
	{
		AVFormatContext *m_pFormatCtx = nullptr;
		AVCodecContext *m_pCodecCtx = nullptr;
		AVFrame *m_pFrame = nullptr;
		AVFrame *m_pFrameRgba = nullptr;
		AVPacket *m_pPacket = nullptr;
		SwsContext *m_pSwsCtx = nullptr;
		int m_VideoStream = -1;
		int m_Width = 0;
		int m_Height = 0;
		int64_t m_LastVideoPts = MENU_MEDIA_PTS_UNSET;

		~SFfmpegVideo() { Close(); }

		void Close()
		{
			if(m_pPacket)
				av_packet_free(&m_pPacket);
			if(m_pFrameRgba)
				av_frame_free(&m_pFrameRgba);
			if(m_pFrame)
				av_frame_free(&m_pFrame);
			if(m_pSwsCtx)
			{
				sws_freeContext(m_pSwsCtx);
				m_pSwsCtx = nullptr;
			}
			if(m_pCodecCtx)
				avcodec_free_context(&m_pCodecCtx);
			if(m_pFormatCtx)
				avformat_close_input(&m_pFormatCtx);
			m_pPacket = nullptr;
			m_pFrameRgba = nullptr;
			m_pFrame = nullptr;
			m_pCodecCtx = nullptr;
			m_pFormatCtx = nullptr;
			m_pSwsCtx = nullptr;
			m_VideoStream = -1;
		}

		int Open(const char *pPath)
		{
			Close();
			if(pPath == nullptr || pPath[0] == '\0')
				return MENU_VIDEO_ERR_OPEN;
			if(avformat_open_input(&m_pFormatCtx, pPath, nullptr, nullptr) != 0)
				return MENU_VIDEO_ERR_OPEN;
			if(avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
				return MENU_VIDEO_ERR_INFO;

			m_VideoStream = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
			if(m_VideoStream < 0)
				return MENU_VIDEO_ERR_STREAM;

			const AVStream *pStream = m_pFormatCtx->streams[m_VideoStream];
			const AVCodec *pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
			if(!pCodec)
				return MENU_VIDEO_ERR_CODEC;

			m_pCodecCtx = avcodec_alloc_context3(pCodec);
			if(!m_pCodecCtx || avcodec_parameters_to_context(m_pCodecCtx, pStream->codecpar) < 0 || avcodec_open2(m_pCodecCtx, pCodec, nullptr) < 0)
				return MENU_VIDEO_ERR_DECODER;

			m_Width = m_pCodecCtx->width;
			m_Height = m_pCodecCtx->height;
			if(m_Width <= 0 || m_Height <= 0)
				return MENU_VIDEO_ERR_DIM;
			if((size_t)m_Width > std::numeric_limits<size_t>::max() / ((size_t)m_Height * 4ull))
				return MENU_VIDEO_ERR_DIM;

			m_pFrame = av_frame_alloc();
			m_pFrameRgba = av_frame_alloc();
			m_pPacket = av_packet_alloc();
			if(!m_pFrame || !m_pFrameRgba || !m_pPacket)
				return MENU_VIDEO_ERR_ALLOC;

			m_pFrameRgba->format = AV_PIX_FMT_RGBA;
			m_pFrameRgba->width = m_Width;
			m_pFrameRgba->height = m_Height;
			if(av_frame_get_buffer(m_pFrameRgba, 1) < 0)
				return MENU_VIDEO_ERR_RGBA;

			m_pSwsCtx = sws_getContext(m_Width, m_Height, m_pCodecCtx->pix_fmt, m_Width, m_Height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
			if(!m_pSwsCtx)
				return MENU_VIDEO_ERR_SCALE;
			return MENU_VIDEO_OK;
		}

		bool Decode(CMenuMediaBackground::SDecodedVideoFrame &OutFrame, const std::atomic<bool> &Stop)
		{
			if(!m_pFormatCtx || !m_pCodecCtx || !m_pPacket || !m_pFrame || !m_pFrameRgba || !m_pSwsCtx || m_Width <= 0 || m_Height <= 0)
				return false;

			bool SeekRestart = false;
			while(!Stop.load(std::memory_order_relaxed))
			{
				while(avcodec_receive_frame(m_pCodecCtx, m_pFrame) == 0)
				{
					int DurationMs = MENU_MEDIA_DEFAULT_VIDEO_FRAME_MS;
					int64_t DurationTs = 0;
					if(m_LastVideoPts != MENU_MEDIA_PTS_UNSET && m_pFrame->best_effort_timestamp != AV_NOPTS_VALUE)
						DurationTs = m_pFrame->best_effort_timestamp - m_LastVideoPts;
					if(DurationTs > 0)
					{
						const int64_t Rescaled = av_rescale_q(DurationTs, m_pFormatCtx->streams[m_VideoStream]->time_base, AVRational{1, 1000});
						if(Rescaled > 0)
							DurationMs = (int)Rescaled;
					}
					m_LastVideoPts = m_pFrame->best_effort_timestamp;

					const size_t FrameBytes = (size_t)m_Width * (size_t)m_Height * 4ull;
					OutFrame.m_vData.resize(FrameBytes);
					OutFrame.m_Width = m_Width;
					OutFrame.m_Height = m_Height;
					OutFrame.m_DurationMs = DurationMs;

					if(av_frame_make_writable(m_pFrameRgba) < 0)
						return false;

					sws_scale(m_pSwsCtx, m_pFrame->data, m_pFrame->linesize, 0, m_Height, m_pFrameRgba->data, m_pFrameRgba->linesize);
					for(int y = 0; y < m_Height; ++y)
						mem_copy(OutFrame.m_vData.data() + (size_t)y * (size_t)m_Width * 4ull, m_pFrameRgba->data[0] + (size_t)y * (size_t)m_pFrameRgba->linesize[0], (size_t)m_Width * 4ull);

					return true;
				}

				const int ReadResult = av_read_frame(m_pFormatCtx, m_pPacket);
				if(ReadResult < 0)
				{
					if(SeekRestart || Stop.load(std::memory_order_relaxed))
						return false;
					if(av_seek_frame(m_pFormatCtx, m_VideoStream, 0, AVSEEK_FLAG_BACKWARD) < 0)
						return false;
					avcodec_flush_buffers(m_pCodecCtx);
					m_LastVideoPts = MENU_MEDIA_PTS_UNSET;
					SeekRestart = true;
					continue;
				}

				SeekRestart = false;
				if(m_pPacket->stream_index != m_VideoStream)
				{
					av_packet_unref(m_pPacket);
					continue;
				}

				const int SendResult = avcodec_send_packet(m_pCodecCtx, m_pPacket);
				av_packet_unref(m_pPacket);
				if(SendResult < 0)
					return false;
			}
			return false;
		}
	};
#else
	bool DecodeFirstFrameFromFile(const char *pAbsolutePath, CImageInfo &ImageOut)
	{
		(void)pAbsolutePath;
		ImageOut.Free();
		return false;
	}
#endif

	const char *MenuVideoErrorText(int Code)
	{
		switch(Code)
		{
		case MENU_VIDEO_ERR_OPEN: return BcLocalize("Failed to open video.");
		case MENU_VIDEO_ERR_INFO: return BcLocalize("Failed to read video info.");
		case MENU_VIDEO_ERR_STREAM: return BcLocalize("No video stream found.");
		case MENU_VIDEO_ERR_CODEC: return BcLocalize("Unsupported video codec.");
		case MENU_VIDEO_ERR_DECODER: return BcLocalize("Failed to initialize video decoder.");
		case MENU_VIDEO_ERR_DIM: return BcLocalize("Invalid video dimensions.");
		case MENU_VIDEO_ERR_ALLOC: return BcLocalize("Failed to allocate video frames.");
		case MENU_VIDEO_ERR_RGBA: return BcLocalize("Failed to allocate RGBA frame.");
		case MENU_VIDEO_ERR_SCALE: return BcLocalize("Failed to initialize video scaler.");
		case MENU_VIDEO_ERR_DECODE: return BcLocalize("Failed while decoding video.");
		default: return BcLocalize("Failed while decoding video.");
		}
	}
}

CMenuMediaBackground::~CMenuMediaBackground()
{
	Unload();
}

void CMenuMediaBackground::Init(IGraphics *pGraphics, IStorage *pStorage)
{
	m_pGraphics = pGraphics;
	m_pStorage = pStorage;
	SetStatus(BcLocalize("Disabled."));
}

void CMenuMediaBackground::SetStatus(const char *pText)
{
	m_HasError = false;
	str_copy(m_aStatus, pText, sizeof(m_aStatus));
}

void CMenuMediaBackground::SetError(const char *pText)
{
	m_HasError = true;
	str_copy(m_aStatus, pText, sizeof(m_aStatus));
}

void CMenuMediaBackground::RecycleVideoBuffer(std::vector<uint8_t> &Buffer)
{
	if(Buffer.empty())
		return;
	if(!m_FreeBufferQueue.try_enqueue(std::move(Buffer)))
		Buffer.clear();
}

void CMenuMediaBackground::ReleaseMainVideoResources()
{
	if(m_pGraphics != nullptr)
		m_pGraphics->UnloadTexture(&m_VideoTexture);

	SDecodedVideoFrame Dummy;
	while(m_FrameQueue.try_dequeue(Dummy))
	{
	}
	std::vector<uint8_t> DummyBuf;
	while(m_FreeBufferQueue.try_dequeue(DummyBuf))
	{
	}

	m_Width = 0;
	m_Height = 0;
	m_NextFrameTime = std::chrono::nanoseconds::zero();
}

void CMenuMediaBackground::FinishStoppedDecoder()
{
	if(!m_DecoderThread.joinable())
		return;
	if(!m_DecoderExited.load(std::memory_order_acquire))
		return;

	m_DecoderThread.detach();

	const bool Failed = m_DecoderFailed.load(std::memory_order_relaxed);
	const int ErrorCode = m_DecoderError.load(std::memory_order_relaxed);
	const bool HadFrame = m_IsLoaded;
	const bool Stopping = m_StopPending;
	m_StopPending = false;
	ReleaseMainVideoResources();
	m_IsVideo = false;
	m_IsLoaded = false;

	if(Stopping)
	{
		if(!m_ReloadPending && m_LastConfigEnabled == 0)
		{
			SetStatus(BcLocalize("Disabled."));
			m_LiveWork = false;
		}
	}
	else if(Failed && !m_ReloadPending)
	{
		if(ErrorCode == MENU_VIDEO_ERR_DECODE && !HadFrame)
			SetError(BcLocalize("Failed to decode first video frame."));
		else
			SetError(MenuVideoErrorText(ErrorCode));
		if(m_LastConfigEnabled == 0)
			m_LiveWork = false;
	}
}

void CMenuMediaBackground::Unload()
{
	m_StopRequested.store(true, std::memory_order_relaxed);
	m_ReloadPending = false;
	m_StopPending = false;
	if(m_DecoderThread.joinable())
	{
		if(m_DecoderExited.load(std::memory_order_acquire))
			m_DecoderThread.detach();
		else
			m_DecoderThread.join();
	}

	ReleaseMainVideoResources();
	MediaDecoder::UnloadFrames(m_pGraphics, m_vFrames);

	m_Animated = false;
	m_AnimationStart = 0;
	m_IsVideo = false;
	m_IsLoaded = false;
	m_HasError = false;
	m_LiveWork = false;
	m_aLoadedPath[0] = '\0';
	m_aDecoderPath[0] = '\0';
	m_aPendingPath[0] = '\0';
	m_PendingEnabled = 0;
	m_LastConfigEnabled = -1;
	m_aLastConfigPath[0] = '\0';
	m_LastDecodeStepTick = 0;
	str_copy(m_aStatus, BcLocalize("Disabled."), sizeof(m_aStatus));
}

void CMenuMediaBackground::Shutdown()
{
	Unload();
	m_pGraphics = nullptr;
	m_pStorage = nullptr;
}

bool CMenuMediaBackground::LoadStaticMedia(const char *pPath, int StorageType)
{
	m_IsVideo = false;

	const std::string Ext = MediaDecoder::ExtractExtensionLower(pPath);
	if(Ext == "png")
	{
		CImageInfo Image;
		if(!m_pGraphics->LoadPng(Image, pPath, StorageType))
		{
			void *pPngData = nullptr;
			unsigned PngDataSize = 0;
			if(!m_pStorage->ReadFile(pPath, StorageType, &pPngData, &PngDataSize))
			{
				SetError(BcLocalize("Failed to load PNG."));
				return false;
			}
			const bool Decoded = MediaDecoder::DecodeImageToRgba(m_pGraphics, static_cast<unsigned char *>(pPngData), PngDataSize, pPath, Image);
			free(pPngData);
			if(!Decoded)
			{
				SetError(BcLocalize("Failed to load PNG."));
				return false;
			}
		}

		MediaDecoder::UnloadFrames(m_pGraphics, m_vFrames);
		const int ImageWidth = (int)Image.m_Width;
		const int ImageHeight = (int)Image.m_Height;
		SMediaFrame Frame;
		Frame.m_Texture = m_pGraphics->LoadTextureRawMove(Image, 0, pPath);
		if(!Frame.m_Texture.IsValid())
		{
			Image.Free();
			SetError(BcLocalize("Failed to upload PNG."));
			return false;
		}

		m_vFrames.push_back(Frame);
		m_Animated = false;
		m_Width = ImageWidth;
		m_Height = ImageHeight;
		m_AnimationStart = time_get();
		m_IsLoaded = true;
		SetStatus(BcLocalize("Loaded image."));
		return true;
	}

	void *pData = nullptr;
	unsigned DataSize = 0;
	if(!m_pStorage->ReadFile(pPath, StorageType, &pData, &DataSize))
	{
		SetError(BcLocalize("Failed to read file."));
		return false;
	}

	const bool AnimatedImage = MediaDecoder::IsLikelyAnimatedImageExtension(Ext);
	bool Success = false;
	if(AnimatedImage)
	{
		Success = MediaDecoder::DecodeAnimatedImage(m_pGraphics, static_cast<unsigned char *>(pData), DataSize, pPath, m_vFrames, m_Animated, m_Width, m_Height, m_AnimationStart, 15000);
		if(!Success)
			Success = MediaDecoder::DecodeStaticImage(m_pGraphics, static_cast<unsigned char *>(pData), DataSize, pPath, m_vFrames, m_Animated, m_Width, m_Height, m_AnimationStart);
	}
	else
	{
		Success = MediaDecoder::DecodeStaticImage(m_pGraphics, static_cast<unsigned char *>(pData), DataSize, pPath, m_vFrames, m_Animated, m_Width, m_Height, m_AnimationStart);
		if(!Success)
			Success = MediaDecoder::DecodeAnimatedImage(m_pGraphics, static_cast<unsigned char *>(pData), DataSize, pPath, m_vFrames, m_Animated, m_Width, m_Height, m_AnimationStart, 15000);
	}
	free(pData);

	if(!Success)
	{
		char aAbsolutePath[IO_MAX_PATH_LENGTH];
		m_pStorage->GetCompletePath(StorageType, pPath, aAbsolutePath, sizeof(aAbsolutePath));

		CImageInfo FallbackImage;
		if(DecodeFirstFrameFromFile(aAbsolutePath, FallbackImage))
		{
			MediaDecoder::UnloadFrames(m_pGraphics, m_vFrames);
			const int ImageWidth = (int)FallbackImage.m_Width;
			const int ImageHeight = (int)FallbackImage.m_Height;
			SMediaFrame Frame;
			Frame.m_Texture = m_pGraphics->LoadTextureRawMove(FallbackImage, 0, pPath);
			if(!Frame.m_Texture.IsValid())
			{
				FallbackImage.Free();
				SetError(BcLocalize("Failed to upload image."));
				return false;
			}

			m_vFrames.push_back(Frame);
			m_Animated = false;
			m_Width = ImageWidth;
			m_Height = ImageHeight;
			m_AnimationStart = time_get();
			m_IsLoaded = true;
			SetStatus(BcLocalize("Loaded image."));
			return true;
		}

		SetError(BcLocalize("Failed to decode image."));
		return false;
	}

	m_IsLoaded = true;
	SetStatus(m_Animated ? BcLocalize("Loaded animated image.") : BcLocalize("Loaded image."));
	return true;
}

void CMenuMediaBackground::DecoderThreadMain()
{
#if defined(CONF_VIDEORECORDER)
	{
		SFfmpegVideo Video;
		const int OpenResult = Video.Open(m_aDecoderPath);
		if(OpenResult != MENU_VIDEO_OK)
		{
			m_DecoderError.store(OpenResult, std::memory_order_relaxed);
			m_DecoderFailed.store(true, std::memory_order_relaxed);
		}
		else
		{
			while(!m_StopRequested.load(std::memory_order_relaxed))
			{
				if(m_FrameQueue.size_approx() >= 3)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
					continue;
				}

				SDecodedVideoFrame Frame;
				std::vector<uint8_t> RecycledBuffer;
				if(m_FreeBufferQueue.try_dequeue(RecycledBuffer))
					Frame.m_vData = std::move(RecycledBuffer);

				if(!Video.Decode(Frame, m_StopRequested))
				{
					if(!m_StopRequested.load(std::memory_order_relaxed))
					{
						m_DecoderError.store(MENU_VIDEO_ERR_DECODE, std::memory_order_relaxed);
						m_DecoderFailed.store(true, std::memory_order_relaxed);
					}
					break;
				}

				for(;;)
				{
					if(m_FrameQueue.try_enqueue(std::move(Frame)))
						break;
					if(m_StopRequested.load(std::memory_order_relaxed))
						break;
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				}
			}
		}
	}
	m_DecoderExited.store(true, std::memory_order_release);
#else
	m_DecoderFailed.store(true, std::memory_order_relaxed);
	m_DecoderExited.store(true, std::memory_order_release);
#endif
}

bool CMenuMediaBackground::LoadVideo(const char *pPath, int StorageType)
{
#if !defined(CONF_VIDEORECORDER)
	(void)pPath;
	(void)StorageType;
	m_IsVideo = false;
	SetError(BcLocalize("Video backgrounds are unavailable in this build."));
	return false;
#else
	if(m_pStorage == nullptr || m_pGraphics == nullptr || m_DecoderThread.joinable())
	{
		SetError(BcLocalize("Failed to open video."));
		return false;
	}

	char aPath[IO_MAX_PATH_LENGTH];
	m_pStorage->GetCompletePath(StorageType, pPath, aPath, sizeof(aPath));
	if(aPath[0] == '\0')
	{
		SetError(BcLocalize("Failed to open video."));
		return false;
	}

	str_copy(m_aDecoderPath, aPath, sizeof(m_aDecoderPath));
	m_StopRequested.store(false, std::memory_order_relaxed);
	m_DecoderFailed.store(false, std::memory_order_relaxed);
	m_DecoderError.store(MENU_VIDEO_OK, std::memory_order_relaxed);
	m_DecoderExited.store(false, std::memory_order_relaxed);
	m_IsVideo = true;
	m_IsLoaded = false;
	m_LiveWork = true;
	SetStatus(BcLocalize("Loading video."));
	m_DecoderThread = std::thread([this]() { DecoderThreadMain(); });
	return true;
#endif
}

void CMenuMediaBackground::ApplyLoad(int Enabled, const char *pPath)
{
	ReleaseMainVideoResources();
	MediaDecoder::UnloadFrames(m_pGraphics, m_vFrames);
	m_Animated = false;
	m_AnimationStart = 0;
	m_IsVideo = false;
	m_IsLoaded = false;
	m_aLoadedPath[0] = '\0';
	m_LastDecodeStepTick = 0;
	m_ReloadPending = false;

	if(Enabled == 0)
	{
		SetStatus(BcLocalize("Disabled."));
		m_LiveWork = false;
		return;
	}
	if(pPath == nullptr || pPath[0] == '\0')
	{
		SetError(BcLocalize("No media selected."));
		m_LiveWork = false;
		return;
	}

	m_LiveWork = true;
	const int StorageType = fs_is_relative_path(pPath) ? IStorage::TYPE_SAVE : IStorage::TYPE_ABSOLUTE;
	const std::string Ext = MediaDecoder::ExtractExtensionLower(pPath);
	const bool Success = MediaDecoder::IsLikelyVideoExtension(Ext) ? LoadVideo(pPath, StorageType) : LoadStaticMedia(pPath, StorageType);
	if(Success)
		str_copy(m_aLoadedPath, pPath, sizeof(m_aLoadedPath));
	else if(!m_DecoderThread.joinable())
		m_LiveWork = false;
}

void CMenuMediaBackground::ReloadFromConfig(int Enabled, const char *pPath)
{
	if(pPath == nullptr)
		pPath = "";

	m_LastConfigEnabled = Enabled;
	str_copy(m_aLastConfigPath, pPath, sizeof(m_aLastConfigPath));
	m_PendingEnabled = Enabled;
	str_copy(m_aPendingPath, pPath, sizeof(m_aPendingPath));

	const bool WantLoad = Enabled != 0 && pPath[0] != '\0';
	if(m_StopPending)
	{
		m_ReloadPending = WantLoad;
		m_StopRequested.store(true, std::memory_order_relaxed);
		return;
	}

	const bool ThreadRunning = m_DecoderThread.joinable() && !m_DecoderExited.load(std::memory_order_acquire);
	if(ThreadRunning)
	{
		m_StopRequested.store(true, std::memory_order_relaxed);
		m_StopPending = true;
		m_ReloadPending = WantLoad;
		m_IsLoaded = false;
		m_LiveWork = true;
		MediaDecoder::UnloadFrames(m_pGraphics, m_vFrames);
		m_Animated = false;
		if(m_pGraphics != nullptr && m_VideoTexture.IsValid())
			m_pGraphics->UnloadTexture(&m_VideoTexture);
		if(!WantLoad)
			SetStatus(BcLocalize("Disabled."));
		return;
	}

	FinishStoppedDecoder();
	ApplyLoad(Enabled, pPath);
}

void CMenuMediaBackground::SyncFromConfig(int Enabled, const char *pPath)
{
	if(pPath == nullptr)
		pPath = "";
	if(Enabled == 0)
	{
		if(!m_LiveWork && m_LastConfigEnabled == 0)
			return;
		ReloadFromConfig(0, pPath);
		return;
	}
	if(m_LastConfigEnabled != Enabled || str_comp(m_aLastConfigPath, pPath) != 0)
		ReloadFromConfig(Enabled, pPath);
}

void CMenuMediaBackground::UpdateFromConfig(int Enabled, const char *pPath)
{
	if(Enabled == 0 && !m_LiveWork)
		return;
	SyncFromConfig(Enabled, pPath);
	Update();
}

void CMenuMediaBackground::ReloadFromConfig()
{
	ReloadFromConfig(g_Config.m_BclMenuMediaBackground, g_Config.m_BclMenuMediaBackgroundPath);
}

void CMenuMediaBackground::Update()
{
	FinishStoppedDecoder();
	if(m_ReloadPending && !m_DecoderThread.joinable())
	{
		char aPath[IO_MAX_PATH_LENGTH];
		str_copy(aPath, m_aPendingPath, sizeof(aPath));
		ApplyLoad(m_PendingEnabled, aPath);
	}

	if(!m_IsVideo || m_StopPending)
		return;

	if(!CSubsystemTicker::ShouldRunPeriodic(time_get(), m_LastDecodeStepTick, time_freq() / 120))
		return;

	if(m_DecoderFailed.load(std::memory_order_relaxed) && m_DecoderExited.load(std::memory_order_acquire))
	{
		FinishStoppedDecoder();
		return;
	}

	const auto Now = time_get_nanoseconds();
	if(m_VideoTexture.IsValid() && Now < m_NextFrameTime)
		return;

	SDecodedVideoFrame Frame;
	if(!m_FrameQueue.try_dequeue(Frame))
		return;
	if(m_pGraphics == nullptr || Frame.m_vData.empty() || Frame.m_Width <= 0 || Frame.m_Height <= 0)
	{
		RecycleVideoBuffer(Frame.m_vData);
		return;
	}

	if(!m_VideoTexture.IsValid() || m_Width != Frame.m_Width || m_Height != Frame.m_Height)
	{
		if(m_VideoTexture.IsValid())
			m_pGraphics->UnloadTexture(&m_VideoTexture);

		CImageInfo Image;
		Image.m_Width = Frame.m_Width;
		Image.m_Height = Frame.m_Height;
		Image.m_Format = CImageInfo::FORMAT_RGBA;
		Image.m_pData = Frame.m_vData.data();
		m_VideoTexture = m_pGraphics->LoadTextureRaw(Image, 0, m_aLoadedPath);
		m_Width = Frame.m_Width;
		m_Height = Frame.m_Height;
		if(!m_VideoTexture.IsValid())
		{
			SetError(BcLocalize("Failed to create video texture."));
			m_StopRequested.store(true, std::memory_order_relaxed);
			m_StopPending = true;
			m_ReloadPending = false;
			m_IsLoaded = false;
			m_LiveWork = true;
			RecycleVideoBuffer(Frame.m_vData);
			return;
		}
		SetStatus(BcLocalize("Loaded video."));
	}
	else
	{
		m_pGraphics->UpdateTextureRaw(m_VideoTexture, 0, 0, (size_t)m_Width, (size_t)m_Height, Frame.m_vData.data(), false);
	}

	m_IsLoaded = true;
	m_NextFrameTime = Now + std::chrono::milliseconds(std::clamp(Frame.m_DurationMs, 1, MENU_MEDIA_MAX_VIDEO_FRAME_MS));
	RecycleVideoBuffer(Frame.m_vData);
}

void CMenuMediaBackground::RenderTextureCover(IGraphics::CTextureHandle Texture, int Width, int Height, float TargetCenterX, float TargetCenterY, float TargetWidth, float TargetHeight)
{
	const float TextureAspect = Width > 0 && Height > 0 ? (float)Width / (float)Height : 1.0f;
	const float TargetAspect = TargetHeight > 0.0f ? TargetWidth / TargetHeight : 1.0f;

	float DrawW = TargetWidth;
	float DrawH = TargetHeight;

	if(TextureAspect > TargetAspect)
	{
		DrawH = TargetHeight;
		DrawW = DrawH * TextureAspect;
	}
	else
	{
		DrawW = TargetWidth;
		DrawH = DrawW / std::max(TextureAspect, 0.001f);
	}
	const float DrawX = TargetCenterX - DrawW * 0.5f;
	const float DrawY = TargetCenterY - DrawH * 0.5f;

	m_pGraphics->TextureSet(Texture);
	m_pGraphics->WrapClamp();
	m_pGraphics->QuadsBegin();
	m_pGraphics->QuadsSetSubset(0.0f, 0.0f, 1.0f, 1.0f);
	m_pGraphics->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	const IGraphics::CQuadItem Quad(DrawX, DrawY, DrawW, DrawH);
	m_pGraphics->QuadsDrawTL(&Quad, 1);
	m_pGraphics->QuadsEnd();
	m_pGraphics->WrapNormal();
}

bool CMenuMediaBackground::Render(float ScreenWidth, float ScreenHeight, const SRenderContext *pRenderContext)
{
	if(!m_IsLoaded || m_pGraphics == nullptr)
		return false;

	IGraphics::CTextureHandle Texture;
	if(m_IsVideo)
		Texture = m_VideoTexture;
	else if(!MediaDecoder::GetCurrentFrameTexture(m_vFrames, m_Animated, m_AnimationStart, Texture))
		return false;

	if(!Texture.IsValid())
		return false;

	const CScreenRect PrevScreen = m_pGraphics->GetScreen();
	m_pGraphics->BlendNormal();

	const bool UseWorldOffset = pRenderContext != nullptr &&
				    pRenderContext->m_WorldOffset > 0.0f &&
				    pRenderContext->m_ViewWidth > 0.0f &&
				    pRenderContext->m_ViewHeight > 0.0f;
	if(UseWorldOffset)
	{
		const float WorldOffset = std::clamp(pRenderContext->m_WorldOffset, 0.0f, 1.0f);
		const float ViewWidth = pRenderContext->m_ViewWidth;
		const float ViewHeight = pRenderContext->m_ViewHeight;
		const float MapWidth = std::max(pRenderContext->m_MapWidth, ViewWidth);
		const float MapHeight = std::max(pRenderContext->m_MapHeight, ViewHeight);
		const float MapCenterX = MapWidth * 0.5f;
		const float MapCenterY = MapHeight * 0.5f;

		constexpr float MaxCoverScale = 2.0f;
		const float TargetWidth = ViewWidth * (1.0f + (MaxCoverScale - 1.0f) * WorldOffset);
		const float TargetHeight = ViewHeight * (1.0f + (MaxCoverScale - 1.0f) * WorldOffset);

		const float TextureAspect = m_Width > 0 && m_Height > 0 ? (float)m_Width / (float)m_Height : 1.0f;
		const float TargetAspect = TargetHeight > 0.0f ? TargetWidth / TargetHeight : 1.0f;
		float DrawW = TargetWidth;
		float DrawH = TargetHeight;
		if(TextureAspect > TargetAspect)
		{
			DrawH = TargetHeight;
			DrawW = DrawH * TextureAspect;
		}
		else
		{
			DrawW = TargetWidth;
			DrawH = DrawW / std::max(TextureAspect, 0.001f);
		}

		const float MaxShiftX = std::max(0.0f, (DrawW - ViewWidth) * 0.5f);
		const float MaxShiftY = std::max(0.0f, (DrawH - ViewHeight) * 0.5f);
		const float ScrollRangeX = std::max(0.0f, (MapWidth - ViewWidth) * 0.5f);
		const float ScrollRangeY = std::max(0.0f, (MapHeight - ViewHeight) * 0.5f);
		const float RelX = ScrollRangeX > 0.0f ? std::clamp((pRenderContext->m_CameraCenterX - MapCenterX) / ScrollRangeX, -1.0f, 1.0f) : 0.0f;
		const float RelY = ScrollRangeY > 0.0f ? std::clamp((pRenderContext->m_CameraCenterY - MapCenterY) / ScrollRangeY, -1.0f, 1.0f) : 0.0f;
		const float TargetCenterX = pRenderContext->m_CameraCenterX - RelX * MaxShiftX;
		const float TargetCenterY = pRenderContext->m_CameraCenterY - RelY * MaxShiftY;

		m_pGraphics->MapScreen(CScreenRect(
			vec2(pRenderContext->m_CameraCenterX - ViewWidth * 0.5f, pRenderContext->m_CameraCenterY - ViewHeight * 0.5f),
			vec2(pRenderContext->m_CameraCenterX + ViewWidth * 0.5f, pRenderContext->m_CameraCenterY + ViewHeight * 0.5f)));
		RenderTextureCover(Texture, m_Width, m_Height, TargetCenterX, TargetCenterY, TargetWidth, TargetHeight);
	}
	else
	{
		m_pGraphics->MapScreen(CScreenRect(0.0f, 0.0f, ScreenWidth, ScreenHeight));
		RenderTextureCover(Texture, m_Width, m_Height, ScreenWidth * 0.5f, ScreenHeight * 0.5f, ScreenWidth, ScreenHeight);
	}

	m_pGraphics->TextureClear();
	m_pGraphics->MapScreen(PrevScreen);
	return true;
}
