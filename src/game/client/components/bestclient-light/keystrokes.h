/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_KEYSTROKES_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_KEYSTROKES_H

#include <game/client/component.h>
#include <game/client/ui_rect.h>

#include <generated/protocol.h>

class CKeystrokes : public CComponent
{
	int64_t m_Mouse1EndTime = 0;

	CUIRect GetKeyboardRectInternal(bool IgnoreModuleEnabled) const;
	void RenderKeyboardInternal(bool ForcePreview, bool IgnoreModuleEnabled);
	int GetTrackedClientId() const;
	const CNetObj_PlayerInput *GetTrackedInput() const;

public:
	int Sizeof() const override { return sizeof(*this); }

	CUIRect GetKeyboardHudEditorRect() const { return GetKeyboardRectInternal(true); }
	void RenderKeyboardPreview() { RenderKeyboardInternal(true, true); }

	void OnReset() override;
	void OnRender() override;
};

#endif
