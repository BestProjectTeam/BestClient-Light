/* Copyright © 2026 BestProject Team */
#include "freeze_marker.h"

#include <base/color.h>
#include <base/str.h>

#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/collision.h>
#include <game/mapitems.h>

namespace
{
void DrawDiamond(IGraphics *pGraphics, vec2 Center)
{
	constexpr float Outer = 11.0f;
	constexpr float Inner = 9.35f;
	const vec2 aOuter[4] = {
		vec2(Center.x, Center.y - Outer),
		vec2(Center.x + Outer, Center.y),
		vec2(Center.x, Center.y + Outer),
		vec2(Center.x - Outer, Center.y),
	};
	const vec2 aInner[4] = {
		vec2(Center.x, Center.y - Inner),
		vec2(Center.x + Inner, Center.y),
		vec2(Center.x, Center.y + Inner),
		vec2(Center.x - Inner, Center.y),
	};
	for(int i = 0; i < 4; i++)
	{
		const int Next = (i + 1) % 4;
		const IGraphics::CFreeformItem Item(aInner[i], aInner[Next], aOuter[i], aOuter[Next]);
		pGraphics->QuadsDrawFreeform(&Item, 1);
	}
}
}

void CFreezeMarker::Clear()
{
	for(SMarker &Marker : m_vMarkers)
		TextRender()->DeleteTextContainer(Marker.m_TextContainer);
	m_vMarkers.clear();
	m_MarkerByTile.clear();
	for(SLastTouch &Touch : m_aLastTouch)
		Touch = {};
	m_HasData = false;
}

void CFreezeMarker::OnReset()
{
	Clear();
}

void CFreezeMarker::OnMapLoad()
{
	Clear();
}

bool CFreezeMarker::KeepLastTouch(int ClientId) const
{
	if(GameClient()->m_Snap.m_SpecInfo.m_Active || GameClient()->IsWorldPaused() || GameClient()->IsDemoPlaybackPaused())
		return true;
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;
	const CGameClient::CClientData &ClientData = GameClient()->m_aClients[ClientId];
	return ClientData.m_Paused || ClientData.m_Spec;
}

bool CFreezeMarker::IsFrozen(int ClientId) const
{
	const CGameClient::CClientData &ClientData = GameClient()->m_aClients[ClientId];
	return ClientData.m_FreezeEnd != 0 || ClientData.m_LiveFrozen;
}

bool CFreezeMarker::IsFreezeTile(int ClientId, int TileIndex) const
{
	const CCollision *pCollision = Collision();
	const int Tile = pCollision->GetTileIndex(TileIndex);
	const int Front = pCollision->GetFrontTileIndex(TileIndex);
	if(Tile == TILE_FREEZE || Tile == TILE_DFREEZE || Tile == TILE_LFREEZE ||
		Front == TILE_FREEZE || Front == TILE_DFREEZE || Front == TILE_LFREEZE)
		return true;

	const int SwitchType = pCollision->GetSwitchType(TileIndex);
	if(SwitchType != TILE_FREEZE && SwitchType != TILE_DFREEZE && SwitchType != TILE_LFREEZE)
		return false;

	const int Number = pCollision->GetSwitchNumber(TileIndex);
	if(Number <= 0)
		return true;

	const std::vector<SSwitchers> &vSwitchers = GameClient()->Switchers();
	if(Number >= (int)vSwitchers.size())
		return false;
	const int Team = GameClient()->m_Teams.Team(ClientId);
	if(Team < 0 || Team >= NUM_DDRACE_TEAMS)
		return false;
	return vSwitchers[Number].m_aStatus[Team];
}

int CFreezeMarker::FindFreezeTile(int ClientId, vec2 From, vec2 To) const
{
	const float Dist = distance(From, To);
	if(Dist > 32.0f * 48.0f)
		return -1;

	const CCollision *pCollision = Collision();
	const int Steps = (int)Dist + 1;
	int LastIndex = -1;
	for(int i = 0; i <= Steps; i++)
	{
		float Mix = 0.0f;
		if(Dist > 0.0f)
		{
			Mix = i / Dist;
			if(Mix > 1.0f)
				Mix = 1.0f;
		}
		const int Index = pCollision->GetMapIndex(mix(From, To, Mix));
		if(Index < 0 || Index == LastIndex)
			continue;
		LastIndex = Index;
		if(IsFreezeTile(ClientId, Index))
			return Index;
	}
	return -1;
}

void CFreezeMarker::EnsureCountText(SMarker &Marker)
{
	const int ScreenW = Graphics()->ScreenWidth();
	const int ScreenH = Graphics()->ScreenHeight();
	const float Zoom = GameClient()->m_Camera.m_Zoom;
	if(Marker.m_TextContainer.Valid() && Marker.m_BuiltCount == Marker.m_Count && Marker.m_BuiltScreenW == ScreenW && Marker.m_BuiltScreenH == ScreenH && Marker.m_BuiltZoom == Zoom)
		return;

	char aBuf[16];
	str_format(aBuf, sizeof(aBuf), "x%d", Marker.m_Count);
	CTextCursor Cursor;
	Cursor.m_FontSize = 8.0f;
	Cursor.SetPosition(vec2(0.0f, 0.0f));
	TextRender()->DeleteTextContainer(Marker.m_TextContainer);
	TextRender()->CreateOrAppendTextContainer(Marker.m_TextContainer, &Cursor, aBuf);
	if(!Marker.m_TextContainer.Valid())
		return;

	Marker.m_TextH = TextRender()->GetBoundingBoxTextContainer(Marker.m_TextContainer).m_H;
	Marker.m_BuiltCount = Marker.m_Count;
	Marker.m_BuiltScreenW = ScreenW;
	Marker.m_BuiltScreenH = ScreenH;
	Marker.m_BuiltZoom = Zoom;
}

void CFreezeMarker::NoteTouch(int TileX, int TileY, int TileIndex)
{
	const auto It = m_MarkerByTile.find(TileIndex);
	if(It != m_MarkerByTile.end())
	{
		m_vMarkers[It->second].m_Count++;
		return;
	}

	SMarker Marker;
	Marker.m_TileX = TileX;
	Marker.m_TileY = TileY;
	Marker.m_Count = 1;
	m_MarkerByTile.emplace(TileIndex, (int)m_vMarkers.size());
	m_vMarkers.push_back(Marker);
}

void CFreezeMarker::SampleTee(int Slot, int ClientId)
{
	SLastTouch &Last = m_aLastTouch[Slot];
	const bool Active = ClientId >= 0 && ClientId < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[ClientId].m_Active;
	if(!Active)
	{
		if(!KeepLastTouch(ClientId))
			Last = {};
		return;
	}

	const CCollision *pCollision = Collision();
	const int Width = pCollision->GetWidth();
	if(Width <= 0 || pCollision->GetHeight() <= 0)
		return;

	const CNetObj_Character &Snapped = GameClient()->m_aClients[ClientId].m_Snapped;
	const vec2 Pos(Snapped.m_X, Snapped.m_Y);
	const bool Frozen = IsFrozen(ClientId);
	const bool StartedFreeze = Frozen && (!Last.m_Valid || !Last.m_Frozen);
	if(StartedFreeze)
	{
		int TileIndex = -1;
		if(Last.m_HasOlder)
			TileIndex = FindFreezeTile(ClientId, Last.m_OlderPos, Last.m_Pos);
		if(TileIndex < 0 && Last.m_HasPos)
			TileIndex = FindFreezeTile(ClientId, Last.m_Pos, Pos);
		if(TileIndex < 0)
		{
			const int CurIndex = pCollision->GetMapIndex(Pos);
			if(CurIndex >= 0 && IsFreezeTile(ClientId, CurIndex))
				TileIndex = CurIndex;
		}
		if(TileIndex >= 0)
			NoteTouch(TileIndex % Width, TileIndex / Width, TileIndex);
	}
	if(Last.m_HasPos)
	{
		Last.m_OlderPos = Last.m_Pos;
		Last.m_HasOlder = true;
	}
	Last.m_Pos = Pos;
	Last.m_HasPos = true;
	Last.m_Valid = true;
	Last.m_Frozen = Frozen;
	m_HasData = true;
}

void CFreezeMarker::OnUpdate()
{
	if(!g_Config.m_BclDisplayFreezeMarker && m_HasData)
		Clear();
}

void CFreezeMarker::OnNewSnapshot()
{
	if(!g_Config.m_BclDisplayFreezeMarker)
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	const int FirstId = GameClient()->m_aLocalIds[0];
	for(int Slot = 0; Slot < NUM_DUMMIES; Slot++)
	{
		const int ClientId = GameClient()->m_aLocalIds[Slot];
		if(Slot > 0 && ClientId == FirstId)
			continue;
		SampleTee(Slot, ClientId);
	}
}

void CFreezeMarker::OnRender()
{
	if(!g_Config.m_BclDisplayFreezeMarker || m_vMarkers.empty())
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	const CScreenRect PreviousScreen = Graphics()->GetScreen();
	const CScreenRect World = Graphics()->MapScreenToWorld(
		GameClient()->m_Camera.m_Center.x, GameClient()->m_Camera.m_Center.y,
		100.0f, 100.0f, 100.0f, 0, 0,
		Graphics()->ScreenAspect(), GameClient()->m_Camera.m_Zoom);
	Graphics()->MapScreen(World);

	const float ScreenL = World.m_TopLeft.x;
	const float ScreenT = World.m_TopLeft.y;
	const float ScreenR = World.m_BottomRight.x;
	const float ScreenB = World.m_BottomRight.y;

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.96f, 0.38f, 0.12f, 1.0f);
	for(const SMarker &Marker : m_vMarkers)
	{
		const float CenterX = Marker.m_TileX * 32.0f + 16.0f;
		const float CenterY = Marker.m_TileY * 32.0f + 16.0f;
		if(CenterX < ScreenL - 24.0f || CenterX > ScreenR + 48.0f || CenterY < ScreenT - 24.0f || CenterY > ScreenB + 24.0f)
			continue;
		DrawDiamond(Graphics(), vec2(CenterX, CenterY));
	}
	Graphics()->QuadsEnd();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	for(SMarker &Marker : m_vMarkers)
	{
		const float CenterX = Marker.m_TileX * 32.0f + 16.0f;
		const float CenterY = Marker.m_TileY * 32.0f + 16.0f;
		if(CenterX < ScreenL - 24.0f || CenterX > ScreenR + 48.0f || CenterY < ScreenT - 24.0f || CenterY > ScreenB + 24.0f)
			continue;
		EnsureCountText(Marker);
		if(!Marker.m_TextContainer.Valid())
			continue;
		TextRender()->RenderTextContainer(Marker.m_TextContainer, ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f), ColorRGBA(0.0f, 0.0f, 0.0f, 0.45f), CenterX + 13.0f, CenterY - Marker.m_TextH * 0.5f);
	}

	Graphics()->MapScreen(PreviousScreen);
}
