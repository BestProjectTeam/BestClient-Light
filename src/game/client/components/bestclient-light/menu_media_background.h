/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_MENU_MEDIA_BACKGROUND_H
#define GAME_CLIENT_COMPONENTS_MENU_MEDIA_BACKGROUND_H

#include "media_decoder.h"
#include "subsystem_runtime.h"

#include <base/types.h>

#include <engine/external/readerwriterqueue/ReaderWriterQueue.h>
#include <engine/graphics.h>
#include <engine/storage.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

class CMenuMediaBackground
{
	IGraphics *m_pGraphics = nullptr;
	IStorage *m_pStorage = nullptr;

	std::vector<SMediaFrame> m_vFrames;
	bool m_Animated = false;
	int m_Width = 0;
	int m_Height = 0;
	int64_t m_AnimationStart = 0;

	bool m_IsVideo = false;
	bool m_IsLoaded = false;
	bool m_HasError = false;
	char m_aStatus[128] = "";
	char m_aLoadedPath[IO_MAX_PATH_LENGTH] = "";
	int m_LastConfigEnabled = -1;
	char m_aLastConfigPath[IO_MAX_PATH_LENGTH] = "";

	IGraphics::CTextureHandle m_VideoTexture;
	std::chrono::nanoseconds m_NextFrameTime{0};
	int64_t m_LastDecodeStepTick = 0;

	bool m_StopPending = false;
	bool m_ReloadPending = false;
	bool m_LiveWork = false;
	int m_PendingEnabled = 0;
	char m_aPendingPath[IO_MAX_PATH_LENGTH] = "";
	char m_aDecoderPath[IO_MAX_PATH_LENGTH] = "";

	void SetStatus(const char *pText);
	void SetError(const char *pText);
	void ReleaseMainVideoResources();
	void FinishStoppedDecoder();
	void ApplyLoad(int Enabled, const char *pPath);
	void RecycleVideoBuffer(std::vector<uint8_t> &Buffer);
	bool LoadStaticMedia(const char *pPath, int StorageType);
	bool LoadVideo(const char *pPath, int StorageType);
	void RenderTextureCover(IGraphics::CTextureHandle Texture, int Width, int Height, float TargetCenterX, float TargetCenterY, float TargetWidth, float TargetHeight);

public:
	struct SRenderContext
	{
		float m_CameraCenterX = 0.0f;
		float m_CameraCenterY = 0.0f;
		float m_ViewWidth = 0.0f;
		float m_ViewHeight = 0.0f;
		float m_MapWidth = 0.0f;
		float m_MapHeight = 0.0f;
		float m_WorldOffset = 0.0f;
	};

	struct SDecodedVideoFrame
	{
		std::vector<uint8_t> m_vData;
		int m_Width = 0;
		int m_Height = 0;
		int m_DurationMs = 0;
	};

	std::thread m_DecoderThread;
	std::atomic<bool> m_StopRequested{false};
	std::atomic<bool> m_DecoderFailed{false};
	std::atomic<bool> m_DecoderExited{false};
	std::atomic<int> m_DecoderError{0};
	moodycamel::ReaderWriterQueue<SDecodedVideoFrame> m_FrameQueue{4};
	moodycamel::ReaderWriterQueue<std::vector<uint8_t>> m_FreeBufferQueue{4};

	void DecoderThreadMain();

public:
	~CMenuMediaBackground();

	void Init(IGraphics *pGraphics, IStorage *pStorage);
	void SyncFromConfig(int Enabled, const char *pPath);
	void UpdateFromConfig(int Enabled, const char *pPath);
	void ReloadFromConfig(int Enabled, const char *pPath);
	void ReloadFromConfig();
	void Shutdown();
	void Unload();
	void Update();
	bool Render(float ScreenWidth, float ScreenHeight, const SRenderContext *pRenderContext = nullptr);

	bool HasLiveWork() const { return m_LiveWork; }
	bool HasError() const { return m_HasError; }
	bool IsLoaded() const { return m_IsLoaded; }
	const char *StatusText() const { return m_aStatus; }
};

#endif
