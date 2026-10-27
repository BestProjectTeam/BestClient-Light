/* Copyright © 2026 BestProject Team */
#include "aspect_ratio.h"

#include <base/color.h>
#include <base/str.h>
#include <base/time.h>

#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include <game/localization.h>

#include <algorithm>

CAspectRatio::CAspectConfig CAspectRatio::ReadConfig()
{
	return {
		g_Config.m_BclCustomAspectRatioMode,
		g_Config.m_BclCustomAspectRatio,
		g_Config.m_BclCustomAspectRatioApplyMode,
		g_Config.m_BclCustomAspectRatioNum,
		g_Config.m_BclCustomAspectRatioDen,
	};
}

bool CAspectRatio::ConfirmNeeded(const CAspectConfig &Cfg)
{
	return Cfg.m_ApplyMode == 1 && (Cfg.m_Mode > 0 || (Cfg.m_Mode < 0 && Cfg.m_Ratio > 0));
}

bool CAspectRatio::CustomAspectConfigured()
{
	const int AspectMode = g_Config.m_BclCustomAspectRatioMode >= 0 ? g_Config.m_BclCustomAspectRatioMode : (g_Config.m_BclCustomAspectRatio > 0 ? 1 : 0);
	return AspectMode > 0 && g_Config.m_BclCustomAspectRatio >= 100;
}

CAspectRatio::SForcedKey CAspectRatio::ReadForcedKey() const
{
	SForcedKey Key;
	Key.m_State = Client()->State();
	Key.m_Menu = GameClient()->m_Menus.IsActive() ? 1 : 0;
	Key.m_AllowZoom = GameClient()->m_GameInfo.m_AllowZoom ? 1 : 0;
	Key.m_Spec = GameClient()->m_Snap.m_SpecInfo.m_Active ? 1 : 0;
	Key.m_Mode = g_Config.m_BclCustomAspectRatioMode;
	Key.m_Ratio = g_Config.m_BclCustomAspectRatio;
	Key.m_Apply = g_Config.m_BclCustomAspectRatioApplyMode;
	Key.m_Num = g_Config.m_BclCustomAspectRatioNum;
	Key.m_Den = g_Config.m_BclCustomAspectRatioDen;
	Key.m_AnyRes = g_Config.m_TcAllowAnyRes;
	return Key;
}

void CAspectRatio::OnReset()
{
	m_ForcedKeyValid = false;
	m_CursorScaleValid = false;
	m_HudAspectDisabled = false;
	m_MusicPlayerAspectDisabled = false;
	m_CachedUseGameNoHudAspect = false;
}

void CAspectRatio::ResetToPrevious()
{
	g_Config.m_BclCustomAspectRatioMode = m_Prev.m_Mode;
	g_Config.m_BclCustomAspectRatio = m_Prev.m_Ratio;
	g_Config.m_BclCustomAspectRatioApplyMode = m_Prev.m_ApplyMode;
	g_Config.m_BclCustomAspectRatioNum = m_Prev.m_Num;
	g_Config.m_BclCustomAspectRatioDen = m_Prev.m_Den;
	m_Last = m_Prev;
	m_ConfirmActive = false;
	GameClient()->m_TClient.SetForcedAspect();
}

bool CAspectRatio::IsBlockedByFng() const
{
	const int State = Client()->State();
	if(State != IClient::STATE_ONLINE && State != IClient::STATE_DEMOPLAYBACK)
		return false;

	if(GameClient()->m_GameInfo.m_PredictFNG || GameClient()->m_GameInfo.m_EntitiesFNG)
		return true;

	auto ContainsFng = [](const char *pText) -> bool {
		return pText != nullptr && pText[0] != '\0' && str_find_nocase(pText, "fng") != nullptr;
	};

	const CServerInfo &ServerInfo = Client()->ServerInfo();
	const CServerInfo *apInfos[3] = {&ServerInfo, nullptr, nullptr};
	int NumInfos = 1;
	if(GameClient()->m_ConnectServerInfo.has_value())
		apInfos[NumInfos++] = &*GameClient()->m_ConnectServerInfo;
	const auto *pEntry = ServerBrowser()->Find(Client()->ServerAddress());
	if(pEntry)
		apInfos[NumInfos++] = &pEntry->m_Info;

	for(int i = 0; i < NumInfos; ++i)
	{
		const CServerInfo *pInfo = apInfos[i];
		if(ContainsFng(pInfo->m_aName) || ContainsFng(pInfo->m_aGameType) || ContainsFng(pInfo->m_aCommunityId) ||
			ContainsFng(pInfo->m_aCommunityCountry) || ContainsFng(pInfo->m_aCommunityType))
			return true;

		if(pInfo->m_aCommunityId[0] != '\0')
		{
			const CCommunity *pCommunity = ServerBrowser()->Community(pInfo->m_aCommunityId);
			if(pCommunity && ContainsFng(pCommunity->Name()))
				return true;
		}
	}

	return ContainsFng(GameClient()->m_GameInfo.m_aGameType);
}

bool CAspectRatio::IsAspectBlocked() const
{
	return Client()->State() == IClient::STATE_ONLINE && !GameClient()->m_Camera.ZoomAllowed();
}

bool CAspectRatio::UseGameNoHudAspect() const
{
	if(!CustomAspectConfigured())
		return false;

	const int State = Client()->State();
	const bool IsActiveGameplay = State == IClient::STATE_ONLINE || State == IClient::STATE_DEMOPLAYBACK;
	return IsActiveGameplay && !IsAspectBlocked() && g_Config.m_BclCustomAspectRatioApplyMode == 2;
}

bool CAspectRatio::ShouldApplyCustomAspect(bool IsActiveGameplay) const
{
	return !IsAspectBlocked() && (g_Config.m_BclCustomAspectRatioApplyMode == 1 || IsActiveGameplay);
}

void CAspectRatio::ScaleIngameCursorDelta(float &X, float &Y) const
{
	if(!g_Config.m_BclCustomAspectRatioCursorNative)
		return;
	if(!CustomAspectConfigured())
		return;

	const int State = Client()->State();
	const bool IsActiveGameplay = State == IClient::STATE_ONLINE || State == IClient::STATE_DEMOPLAYBACK;
	if(!ShouldApplyCustomAspect(IsActiveGameplay))
		return;

	const int AspectMode = g_Config.m_BclCustomAspectRatioMode >= 0 ? g_Config.m_BclCustomAspectRatioMode : (g_Config.m_BclCustomAspectRatio > 0 ? 1 : 0);
	const int Ratio = g_Config.m_BclCustomAspectRatio;

	const int ScreenW = Graphics()->ScreenWidth();
	const int ScreenH = Graphics()->ScreenHeight();
	if(ScreenW <= 0 || ScreenH <= 0)
		return;

	const int Num = g_Config.m_BclCustomAspectRatioNum;
	const int Den = g_Config.m_BclCustomAspectRatioDen;
	if(!m_CursorScaleValid || m_CursorScaleMode != AspectMode || m_CursorScaleRatio != Ratio || m_CursorScaleNum != Num || m_CursorScaleDen != Den || m_CursorScaleW != ScreenW || m_CursorScaleH != ScreenH)
	{
		float ForcedAspect;
		if(AspectMode == 2 && Num > 0 && Den > 0)
			ForcedAspect = (float)Num / (float)Den;
		else
			ForcedAspect = Ratio / 100.0f;

		float ForcedW, ForcedH, NativeW, NativeH;
		Graphics()->CalcScreenParams(ForcedAspect, 1.0f, &ForcedW, &ForcedH);
		Graphics()->CalcScreenParams((float)ScreenW / (float)ScreenH, 1.0f, &NativeW, &NativeH);
		if(NativeW <= 0.001f || NativeH <= 0.001f)
			return;

		m_CursorScaleMode = AspectMode;
		m_CursorScaleRatio = Ratio;
		m_CursorScaleNum = Num;
		m_CursorScaleDen = Den;
		m_CursorScaleW = ScreenW;
		m_CursorScaleH = ScreenH;
		m_CursorScaleX = ForcedW / NativeW;
		m_CursorScaleY = ForcedH / NativeH;
		m_CursorScaleValid = true;
	}

	X *= m_CursorScaleX;
	Y *= m_CursorScaleY;
}

void CAspectRatio::BeginFrame()
{
	if(!Graphics()->IsBackendInitialized())
	{
		m_ForcedKeyValid = false;
		return;
	}

	if(m_CachedUseGameNoHudAspect)
	{
		Graphics()->SetScreenAspectOverrideEnabled(true);
		m_HudAspectDisabled = false;
		m_MusicPlayerAspectDisabled = false;
	}

	const SForcedKey Key = ReadForcedKey();
	if(m_ForcedKeyValid && Key == m_ForcedKey)
		return;

	m_ForcedKey = Key;
	m_ForcedKeyValid = true;
	Graphics()->SetScreenAspectOverrideEnabled(true);
	m_HudAspectDisabled = false;
	m_MusicPlayerAspectDisabled = false;
	m_CachedUseGameNoHudAspect = UseGameNoHudAspect();
	m_CursorScaleValid = false;
	GameClient()->m_TClient.SetForcedAspect();
}

void CAspectRatio::PrepareComponent(const CComponent *pComponent)
{
	if(!m_CachedUseGameNoHudAspect)
		return;

	if(m_MusicPlayerAspectDisabled && pComponent != &GameClient()->m_MusicPlayer)
	{
		Graphics()->SetScreenAspectOverrideEnabled(true);
		m_MusicPlayerAspectDisabled = false;
	}

	if(pComponent == &GameClient()->m_MusicPlayer)
	{
		Graphics()->SetScreenAspectOverrideEnabled(false);
		m_MusicPlayerAspectDisabled = true;
		return;
	}

	if(!m_HudAspectDisabled && pComponent == &GameClient()->m_Hud)
	{
		Graphics()->SetScreenAspectOverrideEnabled(false);
		m_HudAspectDisabled = true;
	}
}

void CAspectRatio::BeforeFogRect()
{
	if(m_CachedUseGameNoHudAspect && m_HudAspectDisabled)
		Graphics()->SetScreenAspectOverrideEnabled(true);
}

void CAspectRatio::AfterFogRect()
{
	if(m_CachedUseGameNoHudAspect && m_HudAspectDisabled)
		Graphics()->SetScreenAspectOverrideEnabled(false);
}

void CAspectRatio::BeginCursorGameAspect()
{
	if(m_CachedUseGameNoHudAspect)
		Graphics()->SetScreenAspectOverrideEnabled(true);
}

void CAspectRatio::EndCursorGameAspect()
{
	if(m_CachedUseGameNoHudAspect)
		Graphics()->SetScreenAspectOverrideEnabled(false);
}

void CAspectRatio::UpdateConfirm()
{
	constexpr int CONFIRM_SECONDS = 10;
	const CAspectConfig Cur = ReadConfig();

	if(!m_ConfirmSeeded)
	{
		m_ConfirmSeeded = true;
		m_ConfirmActive = false;
		m_Last = Cur;
		m_Prev = Cur;
	}
	else if(m_Last != Cur)
	{
		const bool WasConfirmActive = m_ConfirmActive;
		if(ConfirmNeeded(Cur) && Cur != m_Prev)
		{
			if(!WasConfirmActive)
				m_Prev = m_Last;
			m_ConfirmActive = true;
			m_ConfirmDeadline = time_get() + time_freq() * CONFIRM_SECONDS;
		}
		else
		{
			m_ConfirmActive = false;
			m_Prev = Cur;
		}
		m_Last = Cur;
	}

	if(m_ConfirmActive && IsAspectBlocked())
		m_ConfirmActive = false;

	if(m_ConfirmActive && time_get() >= m_ConfirmDeadline)
		ResetToPrevious();
}

bool CAspectRatio::ShowConfirmOverlay() const
{
	return m_ConfirmActive && ConfirmNeeded(ReadConfig());
}

bool CAspectRatio::ConfirmWantsInput() const
{
	return ShowConfirmOverlay();
}

void CAspectRatio::ApplyMenuUiAspect(bool MenuActive)
{
	const bool IngameMenu = MenuActive && (Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK);
	const bool UseWindowAspectForUi = IngameMenu && (IsAspectBlocked() || g_Config.m_BclCustomAspectRatioApplyMode != 1);
	m_MenuRestoreGraphicsAspect = !UseWindowAspectForUi;
	Ui()->SetUseGraphicsScreenAspect(m_MenuRestoreGraphicsAspect);
}

void CAspectRatio::RestoreMenuUiAspect()
{
	Ui()->SetUseGraphicsScreenAspect(true);
}

void CAspectRatio::RenderConfirmOverlay(bool MenuActive)
{
	const bool ShowOverlay = ShowConfirmOverlay();
	if(ShowOverlay)
	{
		Ui()->SetUseGraphicsScreenAspect(false);
		Ui()->MapScreen();
		const CUIRect *pReal = Ui()->Screen();

		const float WinW = (float)Graphics()->ScreenWidth();
		const float WinH = (float)Graphics()->ScreenHeight();
		const vec2 RawMouse = Ui()->UpdatedMousePos();
		const vec2 RealMouse = vec2(RawMouse.x * pReal->w / WinW, RawMouse.y * pReal->h / WinH);

		const int SecondsLeft = std::max(0, (int)((m_ConfirmDeadline - time_get() + time_freq() - 1) / time_freq()));
		char aConfirmText[128];
		str_format(aConfirmText, sizeof(aConfirmText), Localize("Keep this aspect ratio? Reverting in %d..."), SecondsLeft);

		const float PanelW = std::min(420.0f, pReal->w - 12.0f);
		CUIRect Panel;
		Panel.x = pReal->x + (pReal->w - PanelW) * 0.5f;
		Panel.y = pReal->y + 6.0f;
		Panel.w = PanelW;
		Panel.h = 30.0f;
		Graphics()->DrawRect(Panel.x, Panel.y, Panel.w, Panel.h, ColorRGBA(0.0f, 0.0f, 0.0f, 0.82f), IGraphics::CORNER_ALL, 5.0f);
		Panel.Margin(2.0f, &Panel);

		CUIRect ApplyBtn, RevertBtn;
		Panel.VSplitRight(80.0f, &Panel, &RevertBtn);
		RevertBtn.VMargin(2.0f, &RevertBtn);
		Panel.VSplitRight(4.0f, &Panel, nullptr);
		Panel.VSplitRight(88.0f, &Panel, &ApplyBtn);
		ApplyBtn.VMargin(2.0f, &ApplyBtn);
		Panel.VSplitRight(4.0f, &Panel, nullptr);
		Ui()->DoLabel(&Panel, aConfirmText, 10.0f, TEXTALIGN_ML);

		auto RenderOverlayBtn = [&](const CUIRect &Rect, const char *pText, ColorRGBA Normal, ColorRGBA Hovered) -> bool {
			const bool Hover = RealMouse.x >= Rect.x && RealMouse.x < Rect.x + Rect.w &&
					   RealMouse.y >= Rect.y && RealMouse.y < Rect.y + Rect.h;
			Graphics()->DrawRect(Rect.x, Rect.y, Rect.w, Rect.h, Hover ? Hovered : Normal, IGraphics::CORNER_ALL, 3.0f);
			Ui()->DoLabel(&Rect, pText, 10.5f, TEXTALIGN_MC);
			return Hover && Ui()->MouseButtonClicked(0);
		};

		if(RenderOverlayBtn(ApplyBtn, Localize("Apply"), ColorRGBA(0.18f, 0.36f, 0.18f, 1.0f), ColorRGBA(0.28f, 0.55f, 0.28f, 1.0f)))
		{
			m_ConfirmActive = false;
			m_Prev = ReadConfig();
		}
		if(RenderOverlayBtn(RevertBtn, Localize("Revert"), ColorRGBA(0.45f, 0.18f, 0.18f, 1.0f), ColorRGBA(0.7f, 0.28f, 0.28f, 1.0f)))
			ResetToPrevious();

		Ui()->SetUseGraphicsScreenAspect(m_MenuRestoreGraphicsAspect);
		Ui()->MapScreen();
	}

	if(MenuActive || ShowOverlay)
	{
		vec2 CursorPos = Ui()->MousePos();
		if(ShowOverlay && !MenuActive)
		{
			const CUIRect *pScreen = Ui()->Screen();
			const vec2 RawMouse = Ui()->UpdatedMousePos();
			CursorPos = vec2(RawMouse.x * pScreen->w / (float)Graphics()->ScreenWidth(), RawMouse.y * pScreen->h / (float)Graphics()->ScreenHeight());
		}
		RenderTools()->RenderCursor(CursorPos, 24.0f);
	}
}
