/* Copyright © 2026 BestProject Team */
#include "hud_extras.h"

#include "ui_animations.h"

#include <base/color.h>
#include <base/str.h>

#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/localization.h>
#include <generated/protocol.h>

#include <algorithm>

bool HudExtras::HasPlayerBelowOnSameX(CGameClient *pGameClient, int ClientId, const vec2 &PosBlocks)
{
	if(!pGameClient || ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(i == ClientId || !pGameClient->m_aClients[i].m_Active || !pGameClient->m_Snap.m_aCharacters[i].m_Active)
			continue;
		if(pGameClient->m_Snap.m_apPlayerInfos[i] && pGameClient->m_Snap.m_apPlayerInfos[i]->m_Team == TEAM_SPECTATORS)
			continue;

		vec2 OtherPos;
		if(pGameClient->m_aClients[i].m_SpecCharPresent)
		{
			OtherPos = pGameClient->m_aClients[i].m_SpecChar / 32.0f;
		}
		else
		{
			const int Conn = (i == pGameClient->m_aLocalIds[1]) ? 1 : 0;
			const CNetObj_Character *pPrevChar = &pGameClient->m_Snap.m_aCharacters[i].m_Prev;
			const CNetObj_Character *pCurChar = &pGameClient->m_Snap.m_aCharacters[i].m_Cur;
			const float IntraTick = pGameClient->Client()->IntraGameTick(Conn);
			OtherPos = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pCurChar->m_X, pCurChar->m_Y), IntraTick) / 32.0f;
		}

		if(absolute(PosBlocks.x - OtherPos.x) < 0.01f && OtherPos.y > PosBlocks.y)
			return true;
	}
	return false;
}

int HudExtras::GetCheckpointId(CGameClient *pGameClient)
{
	if(!pGameClient)
		return -1;

	int PlayerId = pGameClient->m_Snap.m_LocalClientId;
	if(pGameClient->m_Snap.m_SpecInfo.m_Active && pGameClient->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW)
		PlayerId = pGameClient->m_Snap.m_SpecInfo.m_SpectatorId;

	if(PlayerId < 0 || PlayerId >= MAX_CLIENTS)
		return -1;

	const auto &Char = pGameClient->m_Snap.m_aCharacters[PlayerId];
	if(!Char.m_Active || !Char.m_HasExtendedData)
		return -1;
	return Char.m_ExtendedData.m_TeleCheckpoint;
}

struct SPlayerBelowLabel
{
	float m_Phase = 0.0f;
	bool m_WasEnabled = false;
	STextContainerIndex m_Container;
	float m_TextW = 0.0f;
	float m_CanvasW = -1.0f;
	float m_CanvasH = -1.0f;
	char m_aText[128] = "";
};

SPlayerBelowLabel s_PlayerBelowLabel;

void HudExtras::InvalidatePlayerBelowText(ITextRender *pTextRender)
{
	if(pTextRender)
		pTextRender->DeleteTextContainer(s_PlayerBelowLabel.m_Container);
	s_PlayerBelowLabel.m_TextW = 0.0f;
	s_PlayerBelowLabel.m_CanvasW = -1.0f;
	s_PlayerBelowLabel.m_CanvasH = -1.0f;
	s_PlayerBelowLabel.m_aText[0] = '\0';
}

void HudExtras::RenderPlayerBelowIndicator(CGameClient *pGameClient, IGraphics *pGraphics, ITextRender *pTextRender, float Width, float Height, float Dt)
{
	const bool Enabled = pGameClient && pGraphics && pTextRender && g_Config.m_BclShowhudDummyCoordIndicator != 0;
	if(!Enabled)
	{
		if(s_PlayerBelowLabel.m_WasEnabled)
		{
			s_PlayerBelowLabel.m_Phase = 0.0f;
			InvalidatePlayerBelowText(pTextRender);
		}
		s_PlayerBelowLabel.m_WasEnabled = false;
		return;
	}
	s_PlayerBelowLabel.m_WasEnabled = true;

	const int ClientId = pGameClient->m_Snap.m_SpecInfo.m_Active ? pGameClient->m_Snap.m_SpecInfo.m_SpectatorId : pGameClient->m_Snap.m_LocalClientId;
	bool Active = false;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		vec2 Pos;
		if(ClientId == SPEC_FREEVIEW)
		{
			Pos = pGameClient->m_Camera.m_Center / 32.0f;
		}
		else if(pGameClient->m_aClients[ClientId].m_SpecCharPresent)
		{
			Pos = pGameClient->m_aClients[ClientId].m_SpecChar / 32.0f;
		}
		else
		{
			const int Conn = g_Config.m_ClDummy;
			const CNetObj_Character *pPrevChar = &pGameClient->m_Snap.m_aCharacters[ClientId].m_Prev;
			const CNetObj_Character *pCurChar = &pGameClient->m_Snap.m_aCharacters[ClientId].m_Cur;
			const float IntraTick = pGameClient->Client()->IntraGameTick(Conn);
			Pos = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pCurChar->m_X, pCurChar->m_Y), IntraTick) / 32.0f;
		}
		Active = HasPlayerBelowOnSameX(pGameClient, ClientId, Pos);
	}

	constexpr float AnimDuration = 0.2f;
	BCUiAnimations::UpdatePhase(s_PlayerBelowLabel.m_Phase, Active ? 1.0f : 0.0f, Dt, AnimDuration);

	if(s_PlayerBelowLabel.m_Phase <= 0.001f)
		return;

	const float Alpha = BCUiAnimations::EaseOutCubic(s_PlayerBelowLabel.m_Phase);
	const float Slide = (1.0f - Alpha) * 10.0f;

	const char *pText = Localize("you below \\/");
	const float Fontsize = 8.0f;
	char aText[sizeof(s_PlayerBelowLabel.m_aText)];
	str_copy(aText, pText);
	if(!s_PlayerBelowLabel.m_Container.Valid() || s_PlayerBelowLabel.m_CanvasW != Width || s_PlayerBelowLabel.m_CanvasH != Height || str_comp(s_PlayerBelowLabel.m_aText, aText) != 0)
	{
		CTextCursor Cursor;
		Cursor.m_FontSize = Fontsize;
		Cursor.SetPosition(vec2(0.0f, 0.0f));
		pTextRender->DeleteTextContainer(s_PlayerBelowLabel.m_Container);
		pTextRender->CreateOrAppendTextContainer(s_PlayerBelowLabel.m_Container, &Cursor, aText);
		if(!s_PlayerBelowLabel.m_Container.Valid())
			return;
		s_PlayerBelowLabel.m_TextW = pTextRender->GetBoundingBoxTextContainer(s_PlayerBelowLabel.m_Container).m_W;
		s_PlayerBelowLabel.m_CanvasW = Width;
		s_PlayerBelowLabel.m_CanvasH = Height;
		str_copy(s_PlayerBelowLabel.m_aText, aText);
	}
	const float TextWidth = s_PlayerBelowLabel.m_TextW;
	const float PaddingX = 8.0f;
	const float PaddingY = 4.0f;
	const float BoxWidth = TextWidth + PaddingX * 2.0f;
	const float BoxHeight = Fontsize + PaddingY * 2.0f;
	const float AdjustedHeight = Height - (g_Config.m_TcStatusBar ? g_Config.m_TcStatusBarHeight : 0.0f);
	const float BoxX = (Width - BoxWidth) * 0.5f;
	const float BoxY = AdjustedHeight - BoxHeight - 20.0f + Slide;

	pGraphics->DrawRect(BoxX, BoxY, BoxWidth, BoxHeight, ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f * Alpha), IGraphics::CORNER_ALL, 5.0f);

	pTextRender->RenderTextContainer(s_PlayerBelowLabel.m_Container, ColorRGBA(0.2f, 1.0f, 0.2f, Alpha), pTextRender->DefaultTextOutlineColor(), BoxX + PaddingX, BoxY + PaddingY);
}

struct SSpectatorCountCache
{
	int m_Tick = -100000;
	bool m_Valid = false;
	bool m_Names = false;
	HudExtras::SSpectatorCountState m_State;
};

SSpectatorCountCache s_SpectatorCountCache;

bool HudExtras::GetSpectatorCountState(CGameClient *pGameClient, IClient *pClient, int &LastSpectatorCountTick, SSpectatorCountState &State, bool ForcePreview)
{
	State = SSpectatorCountState{};
	if(!pGameClient || !pClient)
		return false;

	if(ForcePreview)
	{
		State.m_Count = 5;
		str_copy(State.m_aCountBuf, "5");
		if(g_Config.m_BclShowSpectatorNames)
		{
			str_copy(State.m_aaNameLines[0], "sqwinix");
			str_copy(State.m_aaNameLines[1], "+4");
			State.m_NumNameLines = 2;
		}
		return true;
	}
	if(!g_Config.m_ClShowhudSpectatorCount)
		return false;

	const int Tick = pClient->GameTick(g_Config.m_ClDummy);
	const bool ShowNames = g_Config.m_BclShowSpectatorNames != 0;
	if(s_SpectatorCountCache.m_Valid && s_SpectatorCountCache.m_Names == ShowNames && Tick < s_SpectatorCountCache.m_Tick + pClient->GameTickSpeed())
	{
		State = s_SpectatorCountCache.m_State;
		LastSpectatorCountTick = s_SpectatorCountCache.m_Tick;
		return State.m_Count > 0;
	}

	int aSpectatorIds[MAX_CLIENTS];
	bool aSpectatorAdded[MAX_CLIENTS] = {};
	int NumSpectatorIds = 0;
	bool HasExactSpectatorNames = false;
	bool HasReliableFallbackSpectatorNames = false;
	auto AddSpectator = [&](int ClientId) {
		if(ClientId < 0 || ClientId >= MAX_CLIENTS || aSpectatorAdded[ClientId])
			return;
		aSpectatorAdded[ClientId] = true;
		aSpectatorIds[NumSpectatorIds] = ClientId;
		++NumSpectatorIds;
	};

	int Count = 0;
	const int LocalId = pGameClient->m_aLocalIds[0];
	const int DummyId = pClient->DummyConnected() ? pGameClient->m_aLocalIds[1] : -1;
	if(pClient->IsSixup())
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == LocalId || i == DummyId)
				continue;
			if(pClient->m_TranslationContext.m_aClients[i].m_PlayerFlags7 & protocol7::PLAYERFLAG_WATCHING)
				AddSpectator(i);
		}
		Count = NumSpectatorIds;
		HasExactSpectatorNames = true;
	}
	else
	{
		const CNetObj_SpectatorCount *pSpectatorCount = pGameClient->m_Snap.m_pSpectatorCount;
		if(!pSpectatorCount)
		{
			LastSpectatorCountTick = pClient->GameTick(g_Config.m_ClDummy);
			return false;
		}
		Count = pSpectatorCount->m_NumSpectators;

		if(ShowNames && pGameClient->m_Snap.m_NumSpectatorWatchers > 0)
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(pGameClient->m_Snap.m_aSpectatorWatchers[i])
					AddSpectator(i);
			}
			HasExactSpectatorNames = true;
		}
		else if(ShowNames)
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(i == LocalId || i == DummyId)
					continue;

				const CNetObj_PlayerInfo *pInfo = pGameClient->m_Snap.m_apPlayerInfos[i];
				if(!pInfo || !pGameClient->m_aClients[i].m_Active)
					continue;

				const bool IsLikelySpectator = pInfo->m_Team == TEAM_SPECTATORS || pGameClient->m_aClients[i].m_Spec || pGameClient->m_aClients[i].m_Paused;
				if(IsLikelySpectator)
					AddSpectator(i);
			}
			HasReliableFallbackSpectatorNames = NumSpectatorIds == Count;
		}
	}

	if(Count == 0)
	{
		s_SpectatorCountCache.m_Tick = Tick;
		s_SpectatorCountCache.m_Valid = true;
		s_SpectatorCountCache.m_Names = ShowNames;
		s_SpectatorCountCache.m_State = State;
		LastSpectatorCountTick = Tick;
		return false;
	}

	if(ShowNames && (HasExactSpectatorNames || HasReliableFallbackSpectatorNames) && NumSpectatorIds > 0)
	{
		const int MaxVisibleNames = 5;
		const int VisibleNames = std::min(NumSpectatorIds, MaxVisibleNames);

		int ShownNames = 0;
		for(int i = 0; i < NumSpectatorIds && ShownNames < VisibleNames; i++)
		{
			const char *pName = pGameClient->m_aClients[aSpectatorIds[i]].m_aName;
			if(pName[0] == '\0')
				continue;
			str_copy(State.m_aaNameLines[State.m_NumNameLines], pName);
			++State.m_NumNameLines;
			++ShownNames;
		}

		const int Remaining = std::max(0, Count - ShownNames);
		if(Remaining > 0 && State.m_NumNameLines < 6)
		{
			str_format(State.m_aaNameLines[State.m_NumNameLines], sizeof(State.m_aaNameLines[State.m_NumNameLines]), "+%d", Remaining);
			++State.m_NumNameLines;
		}
	}

	State.m_Count = Count;
	str_format(State.m_aCountBuf, sizeof(State.m_aCountBuf), "%d", Count);
	s_SpectatorCountCache.m_Tick = Tick;
	s_SpectatorCountCache.m_Valid = true;
	s_SpectatorCountCache.m_Names = ShowNames;
	s_SpectatorCountCache.m_State = State;
	LastSpectatorCountTick = Tick;
	return true;
}
