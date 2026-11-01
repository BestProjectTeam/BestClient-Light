/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_TOUCH_CONTROLS_EDITOR_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_TOUCH_CONTROLS_EDITOR_H

#include <game/client/component.h>
#include <game/client/components/touch_controls.h>
#include <game/client/ui.h>

class CTouchControlsEditor : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;
	bool OnInput(const IInput::CEvent &Event) override;
	bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;
	void OnStateChange(int NewState, int OldState) override;

	void Activate();
	void Deactivate();
	bool IsActive() const { return m_Active; }

private:
	enum class EGesture
	{
		NONE,
		PRESS,
		DRAG,
		RESIZE,
	};

	static constexpr int RESIZE_CORNER_NONE = -1;
	static constexpr int RESIZE_CORNER_TL = 0;
	static constexpr int RESIZE_CORNER_TR = 1;
	static constexpr int RESIZE_CORNER_BL = 2;
	static constexpr int RESIZE_CORNER_BR = 3;

	void Save();
	void StopOverlay();
	void OpenOriginalSettings(CTouchControls::CTouchButton *pButton);
	void EnsureScreen();
	int UnitFromScreenX(float ScreenX) const;
	int UnitFromScreenY(float ScreenY) const;
	int HitResizeHandle(const CTouchControls::CTouchButton &Button, vec2 Mouse) const;
	CTouchControls::CTouchButton *HitButton(const std::vector<CTouchControls::CTouchButton *> &vpButtons, vec2 Mouse);
	void MoveSelected(int UnitX, int UnitY);
	void ResizeSelected(int UnitX, int UnitY);
	static vec2 CornerPoint(const CUIRect &Rect, int Corner);
	void RenderOverlay(vec2 Mouse, const std::vector<CTouchControls::CTouchButton *> &vpButtons);
	void RenderOutline(const CUIRect &Rect, ColorRGBA Color);
	void RenderHandles(const CUIRect &Rect);

	bool m_Active = false;
	bool m_Dirty = false;
	bool m_SuppressClick = false;
	bool m_MouseDownLast = false;
	bool m_OpenSettingsOnRelease = false;
	EGesture m_Gesture = EGesture::NONE;
	int m_ResizeCorner = RESIZE_CORNER_NONE;
	int m_DragOffsetX = 0;
	int m_DragOffsetY = 0;
	vec2 m_PressMouse = vec2(0.0f, 0.0f);
	CTouchControls::CUnitRect m_ResizeAnchor{};
	CTouchControls::CTouchButton *m_pSelected = nullptr;
	float m_ScreenW = 1.0f;
	float m_ScreenH = 1.0f;
};

#endif
