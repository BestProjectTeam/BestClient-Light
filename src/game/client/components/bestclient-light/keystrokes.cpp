/* Copyright © 2026 BestProject Team */
#include "keystrokes.h"

#include <base/math.h>
#include <base/str.h>
#include <base/time.h>

#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#if defined(CONF_VIDEORECORDER)
#include <engine/shared/video.h>
#endif

#include <game/client/components/binds.h>
#include <game/client/components/controls.h>
#include <game/client/components/hud_layout.h>
#include <game/client/gameclient.h>
#include <game/client/prediction/entities/character.h>

#include <algorithm>

namespace
{
	constexpr float KEYSTROKES_ATLAS_SCALE = 0.14f;
	constexpr int KEYSTROKES_WHEEL_HIGHLIGHT_MS = 150;
	constexpr float KEYSTROKES_MC_KEY = 150.0f;
	constexpr float KEYSTROKES_MC_GAP = 8.0f;
	constexpr int KEYSTROKES_MC_MAX_KEYS = 8;

	bool IsKeystrokesPressed(const CNetObj_PlayerInput *pInput, int PrimaryKey, int SecondaryKey = 0)
	{
		if(pInput == nullptr)
			return false;

		auto IsPressed = [pInput](int Key) {
			switch(Key)
			{
			case KEY_A:
				return pInput->m_Direction < 0;
			case KEY_D:
				return pInput->m_Direction > 0;
			case KEY_W:
			case KEY_SPACE:
				return pInput->m_Jump != 0;
			case KEY_Q:
				return pInput->m_PrevWeapon != 0;
			case KEY_E:
				return pInput->m_NextWeapon != 0;
			default:
				return false;
			}
		};

		return IsPressed(PrimaryKey) || IsPressed(SecondaryKey);
	}

	bool IsKeystrokesPressed(const CNetObj_Character *pCharacter, int PrimaryKey, int SecondaryKey = 0)
	{
		if(pCharacter == nullptr)
			return false;

		auto IsPressed = [pCharacter](int Key) {
			switch(Key)
			{
			case KEY_A:
				return pCharacter->m_Direction < 0;
			case KEY_D:
				return pCharacter->m_Direction > 0;
			case KEY_W:
			case KEY_SPACE:
				return (pCharacter->m_Jumped & 1) != 0;
			default:
				return false;
			}
		};

		return IsPressed(PrimaryKey) || IsPressed(SecondaryKey);
	}

	bool IsKeystrokesMouseButtonPressed(const CNetObj_PlayerInput *pInput, int MouseButton)
	{
		if(pInput == nullptr)
			return false;

		switch(MouseButton)
		{
		case 1:
			return (pInput->m_Fire & 1) != 0;
		case 2:
			return pInput->m_Hook != 0;
		default:
			return false;
		}
	}

	bool IsKeystrokesMouseButtonPressedFromCharacter(const CNetObj_Character *pPrevCharacter, const CNetObj_Character *pCharacter, int MouseButton, int64_t Now, int64_t Mouse1EndTime)
	{
		if(pCharacter == nullptr)
			return false;

		switch(MouseButton)
		{
		case 1:
			return Mouse1EndTime > Now || (pPrevCharacter != nullptr && pPrevCharacter->m_AttackTick != pCharacter->m_AttackTick);
		case 2:
			return pCharacter->m_HookState != HOOK_IDLE;
		default:
			return false;
		}
	}

	float GetKeystrokesScale(const HudLayout::SModuleLayout &Layout)
	{
		return std::clamp(Layout.m_Scale / 100.0f, 0.25f, 3.0f) * KEYSTROKES_ATLAS_SCALE;
	}

	struct SKeystrokesMcKey
	{
		enum class EKind
		{
			KEY,
			MOUSE,
			SPACE,
		};

		enum class EAction
		{
			LEFT,
			RIGHT,
			UP,
			DOWN,
			JUMP,
			FIRE,
			HOOK,
		};

		EKind m_Kind = EKind::KEY;
		EAction m_Action = EAction::LEFT;
		char m_aLabel[32] = "";
		bool m_ShowLabel = true;
		float m_X = 0.0f;
		float m_Y = 0.0f;
		float m_W = 0.0f;
		float m_H = 0.0f;
	};

	struct SKeystrokesMcLayout
	{
		float m_OverlayWidth = 0.0f;
		float m_OverlayHeight = 0.0f;
		SKeystrokesMcKey m_aKeys[KEYSTROKES_MC_MAX_KEYS] = {};
		int m_NumKeys = 0;
	};

	void ResolveKeystrokesMcLabel(const CBinds *pBinds, const char *pCommand, const char *pFallback, char *pOut, size_t OutSize)
	{
		pOut[0] = '\0';
		if(pBinds != nullptr && pCommand != nullptr)
			pBinds->GetKey(pCommand, pOut, OutSize);
		if(pOut[0] == '\0')
		{
			str_copy(pOut, pFallback, OutSize);
			return;
		}

		if(str_comp_nocase(pOut, "mouse1") == 0)
			str_copy(pOut, "LMB", OutSize);
		else if(str_comp_nocase(pOut, "mouse2") == 0)
			str_copy(pOut, "RMB", OutSize);
		else if(str_comp_nocase(pOut, "mouse3") == 0)
			str_copy(pOut, "MMB", OutSize);
		else
		{
			for(size_t i = 0; pOut[i] != '\0'; ++i)
				pOut[i] = str_uppercase(pOut[i]);
		}
	}

	bool IsKeystrokesMcActionActiveLocal(const CGameClient *pGameClient, SKeystrokesMcKey::EAction Action)
	{
		if(pGameClient == nullptr)
			return false;

		const CNetObj_PlayerInput &Input = pGameClient->m_Controls.m_aInputData[g_Config.m_ClDummy];
		switch(Action)
		{
		case SKeystrokesMcKey::EAction::LEFT:
			return Input.m_Direction < 0;
		case SKeystrokesMcKey::EAction::RIGHT:
			return Input.m_Direction > 0;
		case SKeystrokesMcKey::EAction::UP:
		case SKeystrokesMcKey::EAction::JUMP:
			return Input.m_Jump != 0;
		case SKeystrokesMcKey::EAction::DOWN:
			return pGameClient->Input()->KeyIsPressed(KEY_S);
		case SKeystrokesMcKey::EAction::FIRE:
			return (Input.m_Fire & 1) != 0;
		case SKeystrokesMcKey::EAction::HOOK:
			return Input.m_Hook != 0;
		}
		return false;
	}

	bool IsKeystrokesMcActionActiveTracked(
		SKeystrokesMcKey::EAction Action,
		const CNetObj_PlayerInput *pTrackedInput,
		const CNetObj_Character *pPrevTrackedCharacter,
		const CNetObj_Character *pTrackedCharacter,
		int64_t Now,
		int64_t Mouse1EndTime)
	{
		switch(Action)
		{
		case SKeystrokesMcKey::EAction::LEFT:
			return pTrackedInput != nullptr ?
				       IsKeystrokesPressed(pTrackedInput, KEY_A) :
				       IsKeystrokesPressed(pTrackedCharacter, KEY_A);
		case SKeystrokesMcKey::EAction::RIGHT:
			return pTrackedInput != nullptr ?
				       IsKeystrokesPressed(pTrackedInput, KEY_D) :
				       IsKeystrokesPressed(pTrackedCharacter, KEY_D);
		case SKeystrokesMcKey::EAction::UP:
		case SKeystrokesMcKey::EAction::JUMP:
			return pTrackedInput != nullptr ?
				       IsKeystrokesPressed(pTrackedInput, KEY_SPACE) :
				       IsKeystrokesPressed(pTrackedCharacter, KEY_SPACE);
		case SKeystrokesMcKey::EAction::DOWN:
			return false;
		case SKeystrokesMcKey::EAction::FIRE:
			return pTrackedInput != nullptr ?
				       IsKeystrokesMouseButtonPressed(pTrackedInput, 1) :
				       IsKeystrokesMouseButtonPressedFromCharacter(pPrevTrackedCharacter, pTrackedCharacter, 1, Now, Mouse1EndTime);
		case SKeystrokesMcKey::EAction::HOOK:
			return pTrackedInput != nullptr ?
				       IsKeystrokesMouseButtonPressed(pTrackedInput, 2) :
				       IsKeystrokesMouseButtonPressedFromCharacter(pPrevTrackedCharacter, pTrackedCharacter, 2, Now, Mouse1EndTime);
		}
		return false;
	}

	SKeystrokesMcLayout BuildKeystrokesMcLayout(const CBinds *pBinds = nullptr)
	{
		SKeystrokesMcLayout Layout;
		const float K = KEYSTROKES_MC_KEY;
		const float G = KEYSTROKES_MC_GAP;
		const bool OnlyAd = g_Config.m_BclKeystrokesMcLayout == 1;
		const bool ShowWs = !OnlyAd;
		const bool ShowLmb = !OnlyAd || g_Config.m_BclKeystrokesMcShowLmb != 0;
		const bool ShowRmb = !OnlyAd || g_Config.m_BclKeystrokesMcShowRmb != 0;
		const bool ShowSpace = !OnlyAd || g_Config.m_BclKeystrokesMcShowSpace != 0;
		const float Width = ShowWs ? (3.0f * K + 2.0f * G) : (2.0f * K + G);

		float Y = 0.0f;
		bool HasRow = false;
		auto StartRow = [&]() {
			if(HasRow)
				Y += K + G;
			HasRow = true;
		};
		auto AddKey = [&](SKeystrokesMcKey::EKind Kind, SKeystrokesMcKey::EAction Action, const char *pCommand, const char *pFallback, float X, float W, bool ForceLine = false) {
			if(Layout.m_NumKeys >= KEYSTROKES_MC_MAX_KEYS)
				return;
			SKeystrokesMcKey &Key = Layout.m_aKeys[Layout.m_NumKeys++];
			Key.m_Kind = Kind;
			Key.m_Action = Action;
			Key.m_X = X;
			Key.m_Y = Y;
			Key.m_W = W;
			Key.m_H = K;
			if(ForceLine)
			{
				Key.m_aLabel[0] = '\0';
				Key.m_ShowLabel = false;
			}
			else
			{
				ResolveKeystrokesMcLabel(pBinds, pCommand, pFallback, Key.m_aLabel, sizeof(Key.m_aLabel));
				Key.m_ShowLabel = true;
			}
		};

		if(ShowWs)
		{
			StartRow();
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::UP, nullptr, "W", K + G, K);
			StartRow();
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::LEFT, "+left", "A", 0.0f, K);
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::DOWN, nullptr, "S", K + G, K);
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::RIGHT, "+right", "D", 2.0f * (K + G), K);
		}
		else
		{
			StartRow();
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::LEFT, "+left", "A", 0.0f, K);
			AddKey(SKeystrokesMcKey::EKind::KEY, SKeystrokesMcKey::EAction::RIGHT, "+right", "D", K + G, K);
		}

		if(ShowLmb || ShowRmb)
		{
			StartRow();
			if(ShowLmb && ShowRmb)
			{
				const float Half = (Width - G) * 0.5f;
				AddKey(SKeystrokesMcKey::EKind::MOUSE, SKeystrokesMcKey::EAction::FIRE, "+fire", "LMB", 0.0f, Half);
				AddKey(SKeystrokesMcKey::EKind::MOUSE, SKeystrokesMcKey::EAction::HOOK, "+hook", "RMB", Half + G, Half);
			}
			else if(ShowLmb)
			{
				AddKey(SKeystrokesMcKey::EKind::MOUSE, SKeystrokesMcKey::EAction::FIRE, "+fire", "LMB", 0.0f, Width);
			}
			else
			{
				AddKey(SKeystrokesMcKey::EKind::MOUSE, SKeystrokesMcKey::EAction::HOOK, "+hook", "RMB", 0.0f, Width);
			}
		}

		if(ShowSpace)
		{
			StartRow();
			char aJumpKey[64] = "";
			if(pBinds != nullptr)
				pBinds->GetKey("+jump", aJumpKey, sizeof(aJumpKey));
			const bool JumpIsSpace = aJumpKey[0] == '\0' || str_comp_nocase(aJumpKey, "space") == 0;
			AddKey(SKeystrokesMcKey::EKind::SPACE, SKeystrokesMcKey::EAction::JUMP, "+jump", "SPACE", 0.0f, Width, JumpIsSpace);
		}

		Layout.m_OverlayWidth = Width;
		Layout.m_OverlayHeight = HasRow ? (Y + K) : 0.0f;
		return Layout;
	}

	void DrawKeystrokesMcKey(IGraphics *pGraphics, ITextRender *pTextRender, float X, float Y, float W, float H, bool Active, const char *pLabel)
	{
		if(W <= 0.0f || H <= 0.0f)
			return;

		const float PressedOpacity = std::clamp(g_Config.m_BclKeystrokesMcPressedOpacity * 0.01f, 0.0f, 1.0f);
		const ColorRGBA Bg = Active ? ColorRGBA(1.0f, 1.0f, 1.0f, PressedOpacity) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.55f);
		pGraphics->DrawRect(X, Y, W, H, Bg, IGraphics::CORNER_NONE, 0.0f);

		if(pLabel == nullptr)
		{
			const ColorRGBA LineColor = Active ? ColorRGBA(0.0f, 0.0f, 0.0f, 0.85f) : ColorRGBA(1.0f, 1.0f, 1.0f, 0.9f);
			const float LineW = W * 0.35f;
			const float LineH = std::max(2.0f, H * 0.08f);
			pGraphics->DrawRect(X + (W - LineW) * 0.5f, Y + (H - LineH) * 0.5f, LineW, LineH, LineColor, IGraphics::CORNER_NONE, 0.0f);
			return;
		}

		const ColorRGBA TextColor = Active ? ColorRGBA(0.0f, 0.0f, 0.0f, 1.0f) : ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		const float FontSize = H * (str_length(pLabel) > 1 ? 0.32f : 0.45f);
		pTextRender->TextColor(TextColor);
		const float TextW = pTextRender->TextWidth(FontSize, pLabel, -1, -1.0f);
		pTextRender->Text(X + (W - TextW) * 0.5f, Y + (H - FontSize) * 0.5f, FontSize, pLabel);
		pTextRender->TextColor(pTextRender->DefaultTextColor());
	}

} // namespace

void CKeystrokes::OnReset()
{
	m_Mouse1EndTime = 0;
}

void CKeystrokes::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(GameClient()->m_HudEditor.IsActive())
		return;
#if defined(CONF_VIDEORECORDER)
	if(!((IVideo::Current() && g_Config.m_ClVideoShowhud) || (!IVideo::Current() && g_Config.m_ClShowhud)))
		return;
#else
	if(!g_Config.m_ClShowhud)
		return;
#endif
	if(g_Config.m_ClFocusMode && (g_Config.m_ClFocusModeHideHud || g_Config.m_ClFocusModeHideUI))
		return;

	if(!HudLayout::IsEnabled(HudLayout::MODULE_KEYSTROKES_KEYBOARD) || g_Config.m_BclKeystrokesKeyboard == 0)
		return;

	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	Graphics()->MapScreenToSize(HudWidth, HudHeight);

	RenderKeyboardInternal(false, false);
}

CUIRect CKeystrokes::GetKeyboardRectInternal(bool IgnoreModuleEnabled) const
{
	if(!IgnoreModuleEnabled && (!HudLayout::IsEnabled(HudLayout::MODULE_KEYSTROKES_KEYBOARD) || g_Config.m_BclKeystrokesKeyboard == 0))
		return {};

	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	const auto Layout = HudLayout::Get(HudLayout::MODULE_KEYSTROKES_KEYBOARD, HudWidth, HudHeight);
	const float Scale = GetKeystrokesScale(Layout);
	const auto McLayout = BuildKeystrokesMcLayout();
	const HudLayout::SModuleRect RawRect = {Layout.m_X, Layout.m_Y, McLayout.m_OverlayWidth * Scale, McLayout.m_OverlayHeight * Scale, 0.0f};
	const auto Rect = HudLayout::ClampRectToScreen(RawRect, HudWidth, HudHeight);
	return {Rect.m_X, Rect.m_Y, Rect.m_W, Rect.m_H};
}

int CKeystrokes::GetTrackedClientId() const
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		if(!GameClient()->m_Snap.m_SpecInfo.m_Active &&
			in_range(GameClient()->m_Snap.m_LocalClientId, 0, MAX_CLIENTS - 1) &&
			GameClient()->m_Snap.m_aCharacters[GameClient()->m_Snap.m_LocalClientId].m_Active)
			return GameClient()->m_Snap.m_LocalClientId;
		if(GameClient()->m_DemoSpecId > SPEC_FREEVIEW && GameClient()->m_DemoSpecId < MAX_CLIENTS)
			return GameClient()->m_DemoSpecId;
	}

	if(!GameClient()->m_Snap.m_SpecInfo.m_Active)
		return -1;

	const int SpectatorId = GameClient()->m_Snap.m_SpecInfo.m_SpectatorId;
	if(SpectatorId <= SPEC_FREEVIEW || SpectatorId >= MAX_CLIENTS)
		return -1;

	return SpectatorId;
}

const CNetObj_PlayerInput *CKeystrokes::GetTrackedInput() const
{
	const int SpectatorId = GetTrackedClientId();
	if(SpectatorId < 0)
		return nullptr;

	if(CCharacter *pCharacter = GameClient()->m_GameWorld.GetCharacterById(SpectatorId))
		return pCharacter->LatestInput();

	return nullptr;
}

void CKeystrokes::RenderKeyboardInternal(bool ForcePreview, bool IgnoreModuleEnabled)
{
	const CUIRect Rect = GetKeyboardRectInternal(IgnoreModuleEnabled);
	if(Rect.w <= 0.0f || Rect.h <= 0.0f)
		return;

	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	const auto Layout = HudLayout::Get(HudLayout::MODULE_KEYSTROKES_KEYBOARD, HudWidth, HudHeight);
	const float Scale = GetKeystrokesScale(Layout);
	const int TrackedClientId = ForcePreview ? -1 : GetTrackedClientId();
	const bool HasTrackedPlayer = TrackedClientId >= 0;
	const CNetObj_PlayerInput *pTrackedInput = ForcePreview ? nullptr : GetTrackedInput();
	const CNetObj_Character *pTrackedCharacter = HasTrackedPlayer && GameClient()->m_Snap.m_aCharacters[TrackedClientId].m_Active ?
							     &GameClient()->m_Snap.m_aCharacters[TrackedClientId].m_Cur :
							     nullptr;
	const CNetObj_Character *pPrevTrackedCharacter = HasTrackedPlayer && GameClient()->m_Snap.m_aCharacters[TrackedClientId].m_Active ?
								 &GameClient()->m_Snap.m_aCharacters[TrackedClientId].m_Prev :
								 nullptr;

	const int64_t Now = time_get();
	const int64_t HighlightDuration = time_freq() * KEYSTROKES_WHEEL_HIGHLIGHT_MS / 1000;
	if(!ForcePreview && HasTrackedPlayer && pTrackedInput == nullptr && pTrackedCharacter != nullptr && pPrevTrackedCharacter != nullptr &&
		pPrevTrackedCharacter->m_AttackTick != pTrackedCharacter->m_AttackTick)
	{
		m_Mouse1EndTime = Now + HighlightDuration;
	}

	const auto McLayout = BuildKeystrokesMcLayout(&GameClient()->m_Binds);
	for(int i = 0; i < McLayout.m_NumKeys; ++i)
	{
		const auto &Key = McLayout.m_aKeys[i];
		bool Active = false;
		if(!ForcePreview)
		{
			Active = HasTrackedPlayer ?
					 IsKeystrokesMcActionActiveTracked(Key.m_Action, pTrackedInput, pPrevTrackedCharacter, pTrackedCharacter, Now, m_Mouse1EndTime) :
					 IsKeystrokesMcActionActiveLocal(GameClient(), Key.m_Action);
		}

		DrawKeystrokesMcKey(
			Graphics(),
			TextRender(),
			Rect.x + Key.m_X * Scale,
			Rect.y + Key.m_Y * Scale,
			Key.m_W * Scale,
			Key.m_H * Scale,
			Active,
			Key.m_ShowLabel ? Key.m_aLabel : nullptr);
	}
}
