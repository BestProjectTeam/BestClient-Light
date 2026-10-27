/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_KEYSTROKES_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_KEYSTROKES_H

#include <engine/textrender.h>

#include <game/client/component.h>
#include <game/client/ui_rect.h>

#include <generated/protocol.h>

class CBinds;

class CKeystrokes : public CComponent
{
	static constexpr int MAX_CACHED_KEYS = 8;

	struct SCachedKey
	{
		int m_Action = 0;
		char m_aLabel[32] = "";
		bool m_ShowLabel = true;
		float m_X = 0.0f;
		float m_Y = 0.0f;
		float m_W = 0.0f;
		float m_H = 0.0f;
		float m_FontSize = -1.0f;
		float m_TextWidth = 0.0f;
		STextContainerIndex m_Text;
	};

	int64_t m_Mouse1EndTime = 0;
	bool m_LayoutValid = false;
	int m_CachedBindRevision = -1;
	int m_CachedLayoutMode = -1;
	int m_CachedShowLmb = -1;
	int m_CachedShowRmb = -1;
	int m_CachedShowSpace = -1;
	int m_NumKeys = 0;
	float m_OverlayWidth = 0.0f;
	float m_OverlayHeight = 0.0f;
	SCachedKey m_aKeys[MAX_CACHED_KEYS];

	void ReleaseText();
	void ReleaseCached();
	void EnsureLayout(const CBinds *pBinds);
	void EnsureKeyText(SCachedKey &Key, float FontSize);

	CUIRect GetKeyboardRectInternal(bool IgnoreModuleEnabled);
	void RenderKeyboardInternal(bool ForcePreview, bool IgnoreModuleEnabled);
	int GetTrackedClientId() const;
	const CNetObj_PlayerInput *GetTrackedInput() const;

public:
	int Sizeof() const override { return sizeof(*this); }

	CUIRect GetKeyboardHudEditorRect() { return GetKeyboardRectInternal(true); }
	void RenderKeyboardPreview() { RenderKeyboardInternal(true, true); }

	void OnReset() override;
	void OnWindowResize() override;
	void OnShutdown() override;
	void OnRender() override;
};

#endif
