/* Copyright © 2026 BestProject Team */
#include "touch_controls_editor.h"

#include <base/color.h>
#include <base/math.h>

#include <engine/keys.h>

#include <game/client/gameclient.h>
#include <game/localization.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr float HANDLE_HALF = 14.0f;
constexpr float HANDLE_HIT = 22.0f;
constexpr float OUTLINE_THICKNESS = 3.0f;
constexpr float DRAG_THRESHOLD = 6.0f;

CUIRect UnitToScreen(const CTouchControls::CUnitRect &Unit, float ScreenW, float ScreenH)
{
	const float Scale = (float)CTouchControls::BUTTON_SIZE_SCALE;
	CUIRect Screen;
	Screen.x = Unit.m_X * ScreenW / Scale;
	Screen.y = Unit.m_Y * ScreenH / Scale;
	Screen.w = Unit.m_W * ScreenW / Scale;
	Screen.h = Unit.m_H * ScreenH / Scale;
	return Screen;
}

bool EditorButton(const CTouchControls::CTouchButton *pButton)
{
	return pButton != nullptr && pButton->m_pBehavior != nullptr && pButton->IsVisible();
}
}

void CTouchControlsEditor::Activate()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	if(GameClient()->m_HudEditor.IsActive())
		GameClient()->m_HudEditor.Deactivate();

	m_Active = true;
	m_SuppressClick = true;
	m_MouseDownLast = false;
	m_Gesture = EGesture::NONE;
	m_ResizeCorner = RESIZE_CORNER_NONE;
	m_pSelected = nullptr;
	Ui()->ClosePopupMenus();
}

void CTouchControlsEditor::StopOverlay()
{
	Save();
	m_Active = false;
	m_SuppressClick = false;
	m_MouseDownLast = false;
	m_Gesture = EGesture::NONE;
	m_ResizeCorner = RESIZE_CORNER_NONE;
	m_pSelected = nullptr;
	Ui()->ClosePopupMenus();
}

void CTouchControlsEditor::Deactivate()
{
	if(!m_Active)
		return;

	StopOverlay();
	GameClient()->m_TouchControls.ResetButtonPointers();
}

void CMenus::OpenTouchButtonSettings(CTouchControls::CTouchButton *pButton)
{
	if(pButton == nullptr)
		return;

	CTouchControls &Controls = GameClient()->m_TouchControls;
	Controls.ResetButtonPointers();
	Controls.SetEditingActive(true);
	Controls.ResetVirtualVisibilities();
	Controls.SetUnsavedChanges(false);
	Controls.SetSelectedButton(pButton);

	m_MenusIngameTouchControls.m_CurrentMenu = CMenusIngameTouchControls::EMenuType::MENU_BUTTONS;
	m_MenusIngameTouchControls.m_EditElement = CMenusIngameTouchControls::EElementType::LAYOUT;
	m_MenusIngameTouchControls.CacheAllSettingsFromTarget(pButton);
	m_MenusIngameTouchControls.SetUnsavedChanges(false);
	m_MenusIngameTouchControls.UpdateSampleButton();
	if(Client()->State() == IClient::STATE_ONLINE)
		m_GamePage = PAGE_GAME;
	SetActive(true);
}

void CTouchControlsEditor::OpenOriginalSettings(CTouchControls::CTouchButton *pButton)
{
	if(!m_Active || pButton == nullptr)
		return;

	StopOverlay();
	GameClient()->m_Menus.OpenTouchButtonSettings(pButton);
}

void CTouchControlsEditor::OnStateChange(int NewState, int OldState)
{
	(void)OldState;
	if(NewState != IClient::STATE_ONLINE && NewState != IClient::STATE_DEMOPLAYBACK)
		Deactivate();
}

bool CTouchControlsEditor::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(!m_Active)
		return false;

	Ui()->ConvertMouseMove(&x, &y, CursorType);
	Ui()->OnCursorMove(x, y);
	return true;
}

bool CTouchControlsEditor::OnInput(const IInput::CEvent &Event)
{
	if(!m_Active)
		return false;

	if((Event.m_Flags & IInput::FLAG_PRESS) != 0 && Event.m_Key == KEY_ESCAPE)
	{
		Deactivate();
		return true;
	}
	return true;
}

void CTouchControlsEditor::Save()
{
	if(!m_Dirty)
		return;
	GameClient()->m_TouchControls.SaveConfigurationToFile();
	m_Dirty = false;
}

void CTouchControlsEditor::EnsureScreen()
{
	CTouchControls::CUnitRect Full;
	Full.m_X = 0;
	Full.m_Y = 0;
	Full.m_W = CTouchControls::BUTTON_SIZE_SCALE;
	Full.m_H = CTouchControls::BUTTON_SIZE_SCALE;
	const CUIRect Screen = GameClient()->m_TouchControls.CalculateScreenFromUnitRect(Full);
	m_ScreenW = std::max(Screen.w, 1.0f);
	m_ScreenH = std::max(Screen.h, 1.0f);
}

int CTouchControlsEditor::UnitFromScreenX(float ScreenX) const
{
	const long long Value = std::llround((double)ScreenX * (double)CTouchControls::BUTTON_SIZE_SCALE / (double)m_ScreenW);
	return (int)std::clamp(Value, 0LL, (long long)CTouchControls::BUTTON_SIZE_SCALE);
}

int CTouchControlsEditor::UnitFromScreenY(float ScreenY) const
{
	const long long Value = std::llround((double)ScreenY * (double)CTouchControls::BUTTON_SIZE_SCALE / (double)m_ScreenH);
	return (int)std::clamp(Value, 0LL, (long long)CTouchControls::BUTTON_SIZE_SCALE);
}

vec2 CTouchControlsEditor::CornerPoint(const CUIRect &Rect, int Corner)
{
	switch(Corner)
	{
	case RESIZE_CORNER_TL: return vec2(Rect.x, Rect.y);
	case RESIZE_CORNER_TR: return vec2(Rect.x + Rect.w, Rect.y);
	case RESIZE_CORNER_BL: return vec2(Rect.x, Rect.y + Rect.h);
	default: return vec2(Rect.x + Rect.w, Rect.y + Rect.h);
	}
}

int CTouchControlsEditor::HitResizeHandle(const CTouchControls::CTouchButton &Button, vec2 Mouse) const
{
	const CUIRect Rect = UnitToScreen(Button.m_UnitRect, m_ScreenW, m_ScreenH);
	static constexpr int aCorners[] = {RESIZE_CORNER_TL, RESIZE_CORNER_TR, RESIZE_CORNER_BL, RESIZE_CORNER_BR};
	for(const int Corner : aCorners)
	{
		if(distance(Mouse, CornerPoint(Rect, Corner)) <= HANDLE_HIT)
			return Corner;
	}
	return RESIZE_CORNER_NONE;
}

CTouchControls::CTouchButton *CTouchControlsEditor::HitButton(const std::vector<CTouchControls::CTouchButton *> &vpButtons, vec2 Mouse)
{
	for(int i = (int)vpButtons.size() - 1; i >= 0; --i)
	{
		CTouchControls::CTouchButton *pButton = vpButtons[i];
		if(EditorButton(pButton) && pButton->IsInside(Mouse))
			return pButton;
	}
	return nullptr;
}

void CTouchControlsEditor::MoveSelected(int UnitX, int UnitY)
{
	if(m_pSelected == nullptr)
		return;

	CTouchControls::CUnitRect Rect = m_pSelected->m_UnitRect;
	Rect.m_X = std::clamp(UnitX - m_DragOffsetX, 0, CTouchControls::BUTTON_SIZE_SCALE - Rect.m_W);
	Rect.m_Y = std::clamp(UnitY - m_DragOffsetY, 0, CTouchControls::BUTTON_SIZE_SCALE - Rect.m_H);
	if(Rect.m_X == m_pSelected->m_UnitRect.m_X && Rect.m_Y == m_pSelected->m_UnitRect.m_Y)
		return;
	m_pSelected->m_UnitRect = Rect;
	m_pSelected->UpdateScreenFromUnitRect();
	m_Dirty = true;
}

void CTouchControlsEditor::ResizeSelected(int UnitX, int UnitY)
{
	if(m_pSelected == nullptr || m_ResizeCorner == RESIZE_CORNER_NONE)
		return;

	const CTouchControls::CUnitRect Anchor = m_ResizeAnchor;
	const bool MoveLeft = m_ResizeCorner == RESIZE_CORNER_TL || m_ResizeCorner == RESIZE_CORNER_BL;
	const bool MoveTop = m_ResizeCorner == RESIZE_CORNER_TL || m_ResizeCorner == RESIZE_CORNER_TR;
	int Left = Anchor.m_X;
	int Right = Anchor.m_X + Anchor.m_W;
	int Top = Anchor.m_Y;
	int Bottom = Anchor.m_Y + Anchor.m_H;
	bool AnchorLeft = !MoveLeft;
	bool AnchorTop = !MoveTop;
	if(MoveLeft)
		Left = UnitX;
	else
		Right = UnitX;
	if(MoveTop)
		Top = UnitY;
	else
		Bottom = UnitY;
	if(Left > Right)
	{
		std::swap(Left, Right);
		AnchorLeft = !AnchorLeft;
	}
	if(Top > Bottom)
	{
		std::swap(Top, Bottom);
		AnchorTop = !AnchorTop;
	}

	const int Width = std::clamp(Right - Left, CTouchControls::BUTTON_SIZE_MINIMUM, CTouchControls::BUTTON_SIZE_MAXIMUM);
	const int Height = std::clamp(Bottom - Top, CTouchControls::BUTTON_SIZE_MINIMUM, CTouchControls::BUTTON_SIZE_MAXIMUM);
	if(AnchorLeft)
		Right = Left + Width;
	else
		Left = Right - Width;
	if(AnchorTop)
		Bottom = Top + Height;
	else
		Top = Bottom - Height;
	CTouchControls::CUnitRect Rect;
	Rect.m_W = Width;
	Rect.m_H = Height;
	Rect.m_X = std::clamp(Left, 0, CTouchControls::BUTTON_SIZE_SCALE - Width);
	Rect.m_Y = std::clamp(Top, 0, CTouchControls::BUTTON_SIZE_SCALE - Height);
	if(Rect == m_pSelected->m_UnitRect)
		return;
	m_pSelected->m_UnitRect = Rect;
	m_pSelected->UpdateScreenFromUnitRect();
	m_Dirty = true;
}

void CTouchControlsEditor::RenderOutline(const CUIRect &Rect, ColorRGBA Color)
{
	Graphics()->DrawRect(Rect.x, Rect.y, Rect.w, OUTLINE_THICKNESS, Color, IGraphics::CORNER_NONE, 0.0f);
	Graphics()->DrawRect(Rect.x, Rect.y + Rect.h - OUTLINE_THICKNESS, Rect.w, OUTLINE_THICKNESS, Color, IGraphics::CORNER_NONE, 0.0f);
	Graphics()->DrawRect(Rect.x, Rect.y, OUTLINE_THICKNESS, Rect.h, Color, IGraphics::CORNER_NONE, 0.0f);
	Graphics()->DrawRect(Rect.x + Rect.w - OUTLINE_THICKNESS, Rect.y, OUTLINE_THICKNESS, Rect.h, Color, IGraphics::CORNER_NONE, 0.0f);
}

void CTouchControlsEditor::RenderHandles(const CUIRect &Rect)
{
	static constexpr int aCorners[] = {RESIZE_CORNER_TL, RESIZE_CORNER_TR, RESIZE_CORNER_BL, RESIZE_CORNER_BR};
	const ColorRGBA Color = m_Gesture == EGesture::RESIZE ? ColorRGBA(1.0f, 0.82f, 0.35f, 1.0f) : ColorRGBA(1.0f, 1.0f, 1.0f, 0.95f);
	for(const int Corner : aCorners)
	{
		const vec2 Center = CornerPoint(Rect, Corner);
		Graphics()->DrawRect(Center.x - HANDLE_HALF, Center.y - HANDLE_HALF, HANDLE_HALF * 2.0f, HANDLE_HALF * 2.0f, Color, IGraphics::CORNER_ALL, 4.0f);
	}
}

void CTouchControlsEditor::RenderOverlay(vec2 Mouse, const std::vector<CTouchControls::CTouchButton *> &vpButtons)
{
	Graphics()->MapScreenToSize(m_ScreenW, m_ScreenH);
	Graphics()->TextureClear();
	Graphics()->DrawRect(0.0f, 0.0f, m_ScreenW, m_ScreenH, ColorRGBA(0.0f, 0.0f, 0.0f, 0.38f), IGraphics::CORNER_ALL, 0.0f);

	for(CTouchControls::CTouchButton *pButton : vpButtons)
	{
		if(!EditorButton(pButton))
			continue;
		pButton->UpdateBackgroundCorners();
		pButton->Render(pButton == m_pSelected);
	}

	for(CTouchControls::CTouchButton *pButton : vpButtons)
	{
		if(!EditorButton(pButton))
			continue;
		const bool Selected = pButton == m_pSelected;
		const bool Hovered = pButton->IsInside(Mouse);
		if(!Selected && !Hovered)
			continue;
		const ColorRGBA Color = Selected ? ColorRGBA(1.0f, 1.0f, 1.0f, 0.95f) : ColorRGBA(1.0f, 1.0f, 1.0f, 0.55f);
		const CUIRect Rect = UnitToScreen(pButton->m_UnitRect, m_ScreenW, m_ScreenH);
		RenderOutline(Rect, Color);
		RenderHandles(Rect);
	}

	CUIRect Hint = {16.0f, 16.0f, m_ScreenW - 32.0f, 36.0f};
	Ui()->DoLabel(&Hint, BcLocalize("Click for settings. Drag to move, corners to resize. Esc to close."), 20.0f, TEXTALIGN_ML);
	RenderTools()->RenderCursor(Mouse, 48.0f);
}

void CTouchControlsEditor::OnRender()
{
	if(!m_Active)
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		Deactivate();
		return;
	}

	Ui()->StartCheck();
	Ui()->Update();
	EnsureScreen();
	GameClient()->m_TouchControls.ResetVirtualVisibilities();

	const vec2 WindowSize(std::max(1.0f, (float)Graphics()->WindowWidth()), std::max(1.0f, (float)Graphics()->WindowHeight()));
	const vec2 Mouse = Ui()->UpdatedMousePos() * vec2(m_ScreenW, m_ScreenH) / WindowSize;
	const bool LeftDown = Ui()->MouseButton(0) != 0;
	bool LeftClicked = Ui()->MouseButtonClicked(0) != 0;
	if(m_SuppressClick)
	{
		LeftClicked = false;
		if(!LeftDown)
			m_SuppressClick = false;
	}

	std::vector<CTouchControls::CTouchButton *> vpButtons = GameClient()->m_TouchControls.GetButtonsEditor();
	for(CTouchControls::CTouchButton *pButton : vpButtons)
	{
		if(EditorButton(pButton))
			pButton->UpdateScreenFromUnitRect();
	}
	if(m_pSelected != nullptr && !m_pSelected->IsVisible())
	{
		m_pSelected = nullptr;
		m_Gesture = EGesture::NONE;
		m_ResizeCorner = RESIZE_CORNER_NONE;
	}

	CTouchControls::CTouchButton *pHovered = HitButton(vpButtons, Mouse);
	CTouchControls::CTouchButton *pHandleButton = nullptr;
	int HandleCorner = RESIZE_CORNER_NONE;
	for(int i = (int)vpButtons.size() - 1; i >= 0 && HandleCorner == RESIZE_CORNER_NONE; --i)
	{
		CTouchControls::CTouchButton *pButton = vpButtons[i];
		if(!EditorButton(pButton))
			continue;
		const int Corner = HitResizeHandle(*pButton, Mouse);
		if(Corner != RESIZE_CORNER_NONE)
		{
			pHandleButton = pButton;
			HandleCorner = Corner;
		}
	}
	if(pHandleButton != nullptr && pHovered != nullptr && pHandleButton != pHovered)
	{
		pHandleButton = nullptr;
		HandleCorner = RESIZE_CORNER_NONE;
	}

	if(LeftClicked && pHandleButton != nullptr)
	{
		m_pSelected = pHandleButton;
		m_ResizeCorner = HandleCorner;
		m_ResizeAnchor = pHandleButton->m_UnitRect;
		m_Gesture = EGesture::RESIZE;
	}
	else if(LeftClicked && pHovered != nullptr)
	{
		m_pSelected = pHovered;
		m_DragOffsetX = UnitFromScreenX(Mouse.x) - pHovered->m_UnitRect.m_X;
		m_DragOffsetY = UnitFromScreenY(Mouse.y) - pHovered->m_UnitRect.m_Y;
		m_PressMouse = Mouse;
		m_Gesture = EGesture::PRESS;
	}
	else if(LeftClicked)
	{
		m_pSelected = nullptr;
		m_Gesture = EGesture::NONE;
	}
	else if(LeftDown && m_MouseDownLast && m_Gesture == EGesture::RESIZE)
	{
		ResizeSelected(UnitFromScreenX(Mouse.x), UnitFromScreenY(Mouse.y));
	}
	else if(LeftDown && m_MouseDownLast && (m_Gesture == EGesture::PRESS || m_Gesture == EGesture::DRAG))
	{
		if(m_Gesture == EGesture::PRESS && distance(m_PressMouse, Mouse) > DRAG_THRESHOLD)
			m_Gesture = EGesture::DRAG;
		if(m_Gesture == EGesture::DRAG)
			MoveSelected(UnitFromScreenX(Mouse.x), UnitFromScreenY(Mouse.y));
	}
	else if(!LeftDown && m_MouseDownLast)
	{
		if(m_Gesture == EGesture::PRESS && m_pSelected != nullptr)
		{
			OpenOriginalSettings(m_pSelected);
			Ui()->FinishCheck();
			return;
		}
		if(m_Gesture == EGesture::DRAG || m_Gesture == EGesture::RESIZE)
			Save();
		m_Gesture = EGesture::NONE;
		m_ResizeCorner = RESIZE_CORNER_NONE;
	}

	m_MouseDownLast = LeftDown;
	RenderOverlay(Mouse, vpButtons);
	Ui()->FinishCheck();
}
