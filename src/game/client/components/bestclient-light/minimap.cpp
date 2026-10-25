/* Copyright © 2026 BestProject Team */
#include "minimap.h"

#include <base/color.h>
#include <base/math.h>
#include <base/str.h>
#include <base/vmath.h>

#include <engine/client.h>
#include <engine/console.h>
#include <engine/font_icons.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/components/bestclient-light/hud_editor.h>
#include <game/client/components/hud_layout.h>
#include <game/client/components/chat.h>
#include <game/client/components/console.h>
#include <game/client/components/controls.h>
#include <game/client/components/mapimages.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/collision.h>
#include <game/localization.h>
#include <game/mapitems.h>
#include <game/teamscore.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

enum class EMinimapTileKind
{
	Empty = 0,
	Solid,
	Death,
	Freeze,
	Unfreeze,
	Tele,
	Start,
	Finish,
};

static bool MinimapTileHasAny(int Tile, int A, int B)
{
	return Tile == A || Tile == B;
}

static bool MinimapTileHasAny(int Tile, int A, int B, int C)
{
	return Tile == A || Tile == B || Tile == C;
}

constexpr unsigned MINIMAP_COLOR_SOLID = 4282795716;
constexpr unsigned MINIMAP_COLOR_FREEZE = 4287880335;
constexpr unsigned MINIMAP_COLOR_UNFREEZE = 4285031500;
constexpr unsigned MINIMAP_COLOR_DEATH = 4278516234;
constexpr unsigned MINIMAP_COLOR_TELE = 4289886470;
constexpr unsigned MINIMAP_COLOR_START = 4280785037;
constexpr unsigned MINIMAP_COLOR_FINISH = 4279252383;
constexpr unsigned MINIMAP_COLOR_ALIVE = 0xFF0000FF;
constexpr unsigned MINIMAP_COLOR_FROZEN = 0xFF15FF8C;
constexpr unsigned MINIMAP_COLOR_CRITICAL = 0xFF00FF99;
constexpr float LARGE_MAP_SCREEN_SCALE = 0.86f;

static ColorRGBA MinimapConfigColor(unsigned Color, float Alpha)
{
	ColorRGBA Result = color_cast<ColorRGBA>(ColorHSLA(Color, false));
	Result.a *= Alpha;
	return Result;
}

static ColorRGBA MinimapBackground(float Alpha)
{
	const int Packed = g_Config.m_ClOverlayEntities ? g_Config.m_ClBackgroundEntitiesColor : g_Config.m_ClBackgroundColor;
	return color_cast<ColorRGBA>(ColorHSLA(Packed)).WithAlpha(Alpha);
}

static int ActivityTile(const CCollision *pCollision, vec2 Pos)
{
	if(!pCollision || pCollision->GetWidth() <= 0 || pCollision->GetHeight() <= 0)
		return -1;
	const int Tx = (int)Pos.x / 32;
	const int Ty = (int)Pos.y / 32;
	if(Tx < 0 || Ty < 0 || Tx >= pCollision->GetWidth() || Ty >= pCollision->GetHeight())
		return -1;
	return Ty * pCollision->GetWidth() + Tx;
}

static EMinimapTileKind MinimapTileKind(const CCollision *pCollision, int X, int Y)
{
	if(!pCollision || X < 0 || Y < 0 || X >= pCollision->GetWidth() || Y >= pCollision->GetHeight())
		return EMinimapTileKind::Empty;

	const int Index = Y * pCollision->GetWidth() + X;
	const int Tile = pCollision->GetTileIndex(Index);
	const int Front = pCollision->GetFrontTileIndex(Index);
	const CTeleTile *pTele = pCollision->TeleLayer();

	if(MinimapTileHasAny(Tile, TILE_DEATH, TILE_DEATH) || MinimapTileHasAny(Front, TILE_DEATH, TILE_DEATH))
		return EMinimapTileKind::Death;
	if(MinimapTileHasAny(Tile, TILE_FREEZE, TILE_DFREEZE, TILE_LFREEZE) || MinimapTileHasAny(Front, TILE_FREEZE, TILE_DFREEZE, TILE_LFREEZE))
		return EMinimapTileKind::Freeze;
	if(MinimapTileHasAny(Tile, TILE_UNFREEZE, TILE_DUNFREEZE, TILE_LUNFREEZE) || MinimapTileHasAny(Front, TILE_UNFREEZE, TILE_DUNFREEZE, TILE_LUNFREEZE))
		return EMinimapTileKind::Unfreeze;
	if(pTele && pTele[Index].m_Type != 0 && pTele[Index].m_Number != 0)
		return EMinimapTileKind::Tele;
	if(Tile == TILE_START || Front == TILE_START)
		return EMinimapTileKind::Start;
	if(Tile == TILE_FINISH || Front == TILE_FINISH)
		return EMinimapTileKind::Finish;
	if(MinimapTileHasAny(Tile, TILE_SOLID, TILE_NOHOOK) || MinimapTileHasAny(Front, TILE_SOLID, TILE_NOHOOK))
		return EMinimapTileKind::Solid;
	return EMinimapTileKind::Empty;
}

static ColorRGBA MinimapTileColor(EMinimapTileKind Kind, float Alpha)
{
	switch(Kind)
	{
	case EMinimapTileKind::Solid:
		return MinimapConfigColor(MINIMAP_COLOR_SOLID, Alpha);
	case EMinimapTileKind::Death:
		return MinimapConfigColor(MINIMAP_COLOR_DEATH, Alpha);
	case EMinimapTileKind::Freeze:
		return MinimapConfigColor(MINIMAP_COLOR_FREEZE, Alpha);
	case EMinimapTileKind::Unfreeze:
		return MinimapConfigColor(MINIMAP_COLOR_UNFREEZE, Alpha);
	case EMinimapTileKind::Tele:
		return MinimapConfigColor(MINIMAP_COLOR_TELE, Alpha);
	case EMinimapTileKind::Start:
		return MinimapConfigColor(MINIMAP_COLOR_START, Alpha);
	case EMinimapTileKind::Finish:
		return MinimapConfigColor(MINIMAP_COLOR_FINISH, Alpha);
	case EMinimapTileKind::Empty:
		break;
	}
	return ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
}

static uint16_t PackMinimapVisual(unsigned char Index, unsigned char Flags)
{
	const unsigned char Kept = Flags & (TILEFLAG_XFLIP | TILEFLAG_YFLIP | TILEFLAG_ROTATE);
	return (uint16_t)Index | ((uint16_t)Kept << 8);
}

static void MinimapEntityUv(int Index, int Flags, bool Arrays, float Frac, float Nudge, float &x0, float &y0, float &x1, float &y1, float &x2, float &y2, float &x3, float &y3)
{
	if(Arrays)
	{
		x0 = 0.0f;
		y0 = 0.0f;
		x1 = 1.0f;
		y1 = 0.0f;
		x2 = 1.0f;
		y2 = 1.0f;
		x3 = 0.0f;
		y3 = 1.0f;
	}
	else
	{
		const float TexSize = 1024.0f;
		const int Tx = Index % 16;
		const int Ty = Index / 16;
		const int Px0 = Tx * 64;
		const int Py0 = Ty * 64;
		const int Px1 = Px0 + 63;
		const int Py1 = Py0 + 63;
		x0 = Nudge + Px0 / TexSize + Frac;
		y0 = Nudge + Py0 / TexSize + Frac;
		x1 = Nudge + Px1 / TexSize - Frac;
		y1 = Nudge + Py0 / TexSize + Frac;
		x2 = Nudge + Px1 / TexSize - Frac;
		y2 = Nudge + Py1 / TexSize - Frac;
		x3 = Nudge + Px0 / TexSize + Frac;
		y3 = Nudge + Py1 / TexSize - Frac;
	}

	if(Flags & TILEFLAG_XFLIP)
	{
		x0 = x2;
		x1 = x3;
		x2 = x3;
		x3 = x0;
	}

	if(Flags & TILEFLAG_YFLIP)
	{
		y0 = y3;
		y2 = y1;
		y3 = y1;
		y1 = y0;
	}

	if(Flags & TILEFLAG_ROTATE)
	{
		float Tmp = x0;
		x0 = x3;
		x3 = x2;
		x2 = x1;
		x1 = Tmp;
		Tmp = y0;
		y0 = y3;
		y3 = y2;
		y2 = y1;
		y1 = Tmp;
	}
}

constexpr int MINIMAP_ZOOM_MIN = 300;
constexpr int MINIMAP_ZOOM_MAX = 2400;
constexpr int LARGE_ZOOM_MIN = 20;
constexpr int LARGE_ZOOM_MAX = 2400;

static int ZoomToDisplay(int Value, int Min, int Max)
{
	const int Range = std::max(Max - Min, 1);
	return std::clamp(((Value - Min) * 100 + Range / 2) / Range, 0, 100);
}

static float TileSeamBleed(float Pixel, float TileSpan)
{
	return std::min(Pixel * 0.75f, std::max(TileSpan, 0.0f) * 0.10f);
}

static int ZoomFromDisplay(int Display, int Min, int Max)
{
	const int Range = std::max(Max - Min, 1);
	return Min + std::clamp(Display, 0, 100) * Range / 100;
}

static uint64_t ActivityKey(int Tile, int ClientId, int Kind)
{
	return ((uint64_t)(uint32_t)Tile << 16) | ((uint64_t)(ClientId & 0xFF) << 8) | (uint64_t)Kind;
}

void CMinimap::UpdateScreen()
{
	m_Width = 300.0f * Graphics()->ScreenAspect();
	m_Height = 300.0f;
}

void CMinimap::ResetLargeZoom()
{
	m_ZoomOverride = -1;
}

void CMinimap::RestoreSpecLock()
{
	if(!m_SpecRestoreArmed)
		return;
	const int RestoreId = m_SpecRestoreId;
	m_SpecRestoreArmed = false;
	m_SpecRestoreId = -1;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_SpecInfo.m_Active)
		return;
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK && GameClient()->m_DemoSpecId == SPEC_FOLLOW)
		return;
	GameClient()->m_Spectator.Spectate(RestoreId);
}

void CMinimap::ReleaseFollowInput()
{
	CControls &Controls = GameClient()->m_Controls;
	for(int Dummy = 0; Dummy < NUM_DUMMIES; ++Dummy)
	{
		Controls.m_aInputDirectionLeft[Dummy] = 0;
		Controls.m_aInputDirectionRight[Dummy] = 0;
		Controls.m_aInputDebounce[Dummy] = {};
		CNetObj_PlayerInput &Input = Controls.m_aInputData[Dummy];
		Input.m_Direction = 0;
		Input.m_Jump = 0;
		Input.m_Hook = 0;
		if((Input.m_Fire & 1) != 0)
			Input.m_Fire++;
		Input.m_Fire &= INPUT_STATE_MASK;
		Input.m_NextWeapon = 0;
		Input.m_PrevWeapon = 0;
		GameClient()->m_SnapTap.Reset(Dummy);
	}
	Controls.m_aMousePosOnAction[g_Config.m_ClDummy] = vec2(0.0f, 0.0f);
}

void CMinimap::CloseLarge()
{
	RestoreSpecLock();
	if(m_LargeActive && m_ZoomOverride >= 0)
		g_Config.m_BclLargeMinimapCurrentZoom = m_ZoomOverride;
	m_LargeActive = false;
	m_ForceFullSize = false;
	m_FullBindWasPressed = false;
	m_Dragging = false;
	m_FollowClientId = -1;
	m_FollowArm = -1;
	m_HoveredPlayerId = -1;
	m_MenuPrevPressed = false;
	m_ZoomSliderDrag = false;
	m_ZoomSliderGrab = 0.0f;
	m_MapCursorActive = false;
	m_PanOffset = vec2(0.0f, 0.0f);
	ResetLargeZoom();
}

void CMinimap::OnConsoleInit()
{
	Console()->Register("+bcl_large_minimap", "", CFGFLAG_CLIENT, [](IConsole::IResult *pResult, void *pUserData) {
		CMinimap *pSelf = static_cast<CMinimap *>(pUserData);
		const bool Active = pResult->GetInteger(0) != 0 && g_Config.m_BclMinimap != 0;
		if(Active && !pSelf->m_LargeActive)
			pSelf->m_ZoomOverride = std::clamp(g_Config.m_BclLargeMinimapCurrentZoom, LARGE_ZOOM_MIN, LARGE_ZOOM_MAX);
		else if(!Active && pSelf->m_LargeActive)
		{
			if(pSelf->m_ZoomOverride >= 0)
				g_Config.m_BclLargeMinimapCurrentZoom = pSelf->m_ZoomOverride;
			pSelf->ResetLargeZoom();
		}
		pSelf->m_LargeActive = Active;
		if(!Active)
			pSelf->m_ForceFullSize = false;
	}, this, "Show the large minimap while held");

	Console()->Register("+bcl_large_minimap_full", "", CFGFLAG_CLIENT, [](IConsole::IResult *pResult, void *pUserData) {
		CMinimap *pSelf = static_cast<CMinimap *>(pUserData);
		const bool Pressed = pResult->GetInteger(0) != 0;
		const bool WasPressed = pSelf->m_FullBindWasPressed;
		pSelf->m_FullBindWasPressed = Pressed;
		if(g_Config.m_BclMinimap == 0)
		{
			if(pSelf->m_LargeActive)
				pSelf->CloseLarge();
			return;
		}
		if(!Pressed || WasPressed)
			return;

		if(pSelf->m_LargeActive && pSelf->m_ForceFullSize)
		{
			if(pSelf->m_ZoomOverride >= 0)
				g_Config.m_BclLargeMinimapCurrentZoom = pSelf->m_ZoomOverride;
			pSelf->ResetLargeZoom();
			pSelf->m_MapCursorActive = false;
			pSelf->m_LargeActive = false;
			pSelf->m_ForceFullSize = false;
		}
		else
			pSelf->OpenFullMap();
	}, this, "Toggle the full-size minimap");
}

void CMinimap::OpenFullMap()
{
	if(g_Config.m_BclMinimap == 0 || (m_LargeActive && m_ForceFullSize))
		return;
	if(!m_LargeActive)
		m_ZoomOverride = std::clamp(g_Config.m_BclLargeMinimapCurrentZoom, LARGE_ZOOM_MIN, LARGE_ZOOM_MAX);
	UpdateScreen();
	m_MapCursor = vec2(m_Width * 0.5f, m_Height * 0.5f);
	m_MapCursorActive = true;
	m_LargeActive = true;
	m_ForceFullSize = true;
}

int CMinimap::ClampLargeZoomOut(int Zoom) const
{
	int NewZoom = std::clamp(Zoom, LARGE_ZOOM_MIN, LARGE_ZOOM_MAX);
	if(!Collision() || !Collision()->GameLayer() || Collision()->GetWidth() <= 0 || Collision()->GetHeight() <= 0 || m_Width <= 0.0f || m_Height <= 0.0f)
		return NewZoom;
	const float MapWorldW = Collision()->GetWidth() * 32.0f;
	const float MapWorldH = Collision()->GetHeight() * 32.0f;
	const float LargeScale = m_ForceFullSize ? 1.0f : LARGE_MAP_SCREEN_SCALE;
	const float MinimapW = m_Width * LargeScale;
	const float MinimapH = m_Height * LargeScale;
	const float FitScale = std::max(m_Width / std::max(MapWorldW, 1.0f), m_Height / std::max(MapWorldH, 1.0f));
	const float FullMapZoom = std::min(MinimapW / std::max(MapWorldW * FitScale, 1.0f), MinimapH / std::max(MapWorldH * FitScale, 1.0f));
	return std::max(NewZoom, std::clamp(round_to_int(FullMapZoom * 100.0f), LARGE_ZOOM_MIN, LARGE_ZOOM_MAX));
}

void CMinimap::SetLargeZoom(int Zoom)
{
	UpdateScreen();
	const int NewZoom = ClampLargeZoomOut(Zoom);
	m_ZoomOverride = NewZoom;
	g_Config.m_BclLargeMinimapCurrentZoom = NewZoom;
}

void CMinimap::AdjustLargeZoom(float Steps)
{
	const int Delta = Steps > 0.0f ? 5 : Steps < 0.0f ? -5 : 0;
	if(Delta == 0)
		return;
	const int MinZoom = ClampLargeZoomOut(LARGE_ZOOM_MIN);
	const int CurrentZoom = m_ZoomOverride >= 0 ? m_ZoomOverride : g_Config.m_BclLargeMinimapZoom;
	const int Display = std::clamp(ZoomToDisplay(CurrentZoom, MinZoom, LARGE_ZOOM_MAX) + Delta, 0, 100);
	SetLargeZoom(ZoomFromDisplay(Display, MinZoom, LARGE_ZOOM_MAX));
}

void CMinimap::OnReset()
{
	ReleaseIdle();
	m_MenuPrevPressed = false;
	CloseLarge();
}

void CMinimap::OnMapLoad()
{
	ClearActivity();
}

void CMinimap::OnRelease()
{
	CloseLarge();
}

bool CMinimap::IsLargeMapBind(int Key) const
{
	const int ActiveMask = CBinds::GetModifierMask(Input());
	const int aMasks[] = {ActiveMask, KeyModifier::NONE};
	for(const int Mask : aMasks)
	{
		const char *pBind = GameClient()->m_Binds.Get(Key, Mask);
		if(pBind && str_find(pBind, "bcl_large_minimap"))
			return true;
	}
	return false;
}

bool CMinimap::OnInput(const IInput::CEvent &Event)
{
	if(!m_LargeActive)
		return false;
	if((Event.m_Flags & IInput::FLAG_PRESS) == 0)
		return false;
	if(Event.m_Key == KEY_ESCAPE)
	{
		CloseLarge();
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_WHEEL_UP)
	{
		AdjustLargeZoom(1.0f);
		return true;
	}
	if(Event.m_Key == KEY_MOUSE_WHEEL_DOWN)
	{
		AdjustLargeZoom(-1.0f);
		return true;
	}
	if(!m_ForceFullSize)
		return false;
	if(Event.m_Key == KEY_MOUSE_2)
	{
		if(m_MarkerHoverValid && !m_CursorOverChrome)
			RemoveActivity(m_MarkerHoverIndex);
		return true;
	}
	if(IsLargeMapBind(Event.m_Key))
		return false;
	return true;
}

bool CMinimap::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(!m_LargeActive || !m_ForceFullSize || !m_MapCursorActive)
		return false;

	float Factor = 1.0f;
	if(g_Config.m_ClDyncam && g_Config.m_ClDyncamMousesens)
		Factor = g_Config.m_ClDyncamMousesens / 100.0f;
	else if(CursorType == IInput::CURSOR_JOYSTICK)
		Factor = g_Config.m_InpControllerSens / 100.0f;
	else
		Factor = g_Config.m_InpMousesens / 100.0f;
	if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_SpectatorId < 0)
		Factor *= GameClient()->m_Camera.m_Zoom;
	GameClient()->m_AspectRatio.ScaleIngameCursorDelta(x, y);

	UpdateScreen();
	float WorldW = 1.0f;
	float WorldH = 1.0f;
	Graphics()->CalcScreenParams(Graphics()->ScreenAspect(), 1.0f, &WorldW, &WorldH);
	const float HudPerWorld = m_Height / std::max(WorldH, 1.0f);
	m_MapCursor.x = std::clamp(m_MapCursor.x + x * Factor * HudPerWorld, 0.0f, m_Width);
	m_MapCursor.y = std::clamp(m_MapCursor.y + y * Factor * HudPerWorld, 0.0f, m_Height);
	return true;
}

bool CMinimap::HudCursor(float &X, float &Y) const
{
	if(!m_MapCursorActive)
		return false;
	X = m_MapCursor.x;
	Y = m_MapCursor.y;
	return true;
}

bool CMinimap::ConsumesProgress() const
{
	if(GameClient()->m_HudEditor.IsActive())
		return false;
	const bool FocusHidesHud = g_Config.m_ClFocusMode && g_Config.m_ClFocusModeHideHud;
	if(g_Config.m_BclLargeMinimapProgress == 0)
		return false;
	if(m_LargeActive && g_Config.m_BclMinimap != 0)
		return true;
	if(!g_Config.m_BclMinimap || g_Config.m_BclMinimapHud == 0 || FocusHidesHud)
		return false;
	return true;
}

void CMinimap::SmallMapRect(float HudW, float HudH, float &X, float &Y, float &Size) const
{
	const auto Layout = HudLayout::Get(HudLayout::MODULE_MINIMAP, HudW, HudH);
	const float MapScale = std::clamp(Layout.m_Scale / 100.0f, 0.25f, 3.0f);
	Size = 86.0f * MapScale;
	if(!HudLayout::HasPositionOverride(HudLayout::MODULE_MINIMAP))
	{
		X = HudW - Size - 2.0f;
		Y = (HudH - Size - 4.0f) * 0.12f + 2.0f;
	}
	else
	{
		X = Layout.m_X;
		Y = Layout.m_Y;
	}
	X = std::clamp(X, 0.0f, std::max(0.0f, HudW - Size));
	Y = std::clamp(Y, 0.0f, std::max(0.0f, HudH - Size));
}

void CMinimap::ClearActivity()
{
	m_vActivity.clear();
	m_ActivityIndex.clear();
	for(SActivityPlayer &Player : m_aActivity)
		Player = {};
	m_ActivityOn = false;
	m_MarkerHoverValid = false;
	m_MarkerHoverIndex = -1;
}

void CMinimap::RebuildActivityIndex()
{
	m_ActivityIndex.clear();
	for(int i = 0; i < (int)m_vActivity.size(); ++i)
	{
		const SActivityMarker &Marker = m_vActivity[i];
		m_ActivityIndex.emplace(ActivityKey(Marker.m_Tile, Marker.m_ClientId, Marker.m_Kind), i);
	}
}

void CMinimap::RemoveActivity(int Index)
{
	if(Index < 0 || Index >= (int)m_vActivity.size())
		return;
	m_vActivity.erase(m_vActivity.begin() + Index);
	RebuildActivityIndex();
	m_MarkerHoverValid = false;
	m_MarkerHoverIndex = -1;
}

void CMinimap::ClearMarkers()
{
	m_vActivity.clear();
	m_ActivityIndex.clear();
	m_MarkerHoverValid = false;
	m_MarkerHoverIndex = -1;
}

void CMinimap::NoteActivity(int Tile, int ClientId, int Team, int Kind, int Amount)
{
	if(Tile < 0 || ClientId < 0 || ClientId >= MAX_CLIENTS || Kind < 0 || Kind > 3 || Amount <= 0)
		return;
	const uint64_t Key = ActivityKey(Tile, ClientId, Kind);
	const auto Found = m_ActivityIndex.find(Key);
	if(Found != m_ActivityIndex.end())
	{
		m_vActivity[Found->second].m_Count += Amount;
		return;
	}
	if(m_vActivity.size() >= 8192)
		return;
	SActivityMarker Marker;
	Marker.m_Tile = Tile;
	Marker.m_Team = Team;
	Marker.m_ClientId = ClientId;
	Marker.m_Kind = (uint8_t)Kind;
	Marker.m_Count = Amount;
	m_ActivityIndex.emplace(Key, (int)m_vActivity.size());
	m_vActivity.push_back(Marker);
}

void CMinimap::SampleActivity()
{
	const bool Active = g_Config.m_BclMinimap != 0;
	if(!Active)
	{
		if(m_ActivityOn || !m_vActivity.empty())
			ClearActivity();
		return;
	}
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;

	CCollision *pCollision = Collision();
	if(!pCollision)
		return;

	m_ActivityOn = true;
	const int CurTick = Client()->GameTick(g_Config.m_ClDummy);
	constexpr float HammerRangeSq = 100.0f * 100.0f;

	auto FrozenNow = [&](int ClientId) {
		const CGameClient::CClientData &ClientData = GameClient()->m_aClients[ClientId];
		return ClientData.m_DeepFrozen || ClientData.m_LiveFrozen || (ClientData.m_FreezeEnd != 0 && ClientData.m_FreezeEnd > CurTick);
	};
	auto CanInteract = [&](int A, int B) {
		if(GameClient()->m_aClients[A].m_Solo || GameClient()->m_aClients[B].m_Solo)
			return false;
		const int TeamA = GameClient()->m_Teams.Team(A);
		const int TeamB = GameClient()->m_Teams.Team(B);
		if(TeamA > TEAM_FLOCK && TeamB > TEAM_FLOCK && TeamA != TeamB)
			return false;
		return true;
	};

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
			continue;
		if(GameClient()->m_aClients[i].m_HammerHitDisabled)
			continue;
		const CNetObj_Character &Cur = GameClient()->m_aClients[i].m_Snapped;
		SActivityPlayer &Attacker = m_aActivity[i];
		const bool NewSwing = Attacker.m_WasActive && Cur.m_Weapon == WEAPON_HAMMER && Cur.m_AttackTick != Attacker.m_PrevAttackTick;
		if(!NewSwing)
			continue;
		const int AttackerTeam = GameClient()->m_Teams.Team(i);
		for(int j = 0; j < MAX_CLIENTS; j++)
		{
			if(j == i || !GameClient()->m_Snap.m_aCharacters[j].m_Active || !CanInteract(i, j))
				continue;
			const CNetObj_Character &Other = GameClient()->m_aClients[j].m_Snapped;
			const float Dx = (float)(Cur.m_X - Other.m_X);
			const float Dy = (float)(Cur.m_Y - Other.m_Y);
			if(Dx * Dx + Dy * Dy > HammerRangeSq)
				continue;
			Attacker.m_Hits++;
			NoteActivity(ActivityTile(pCollision, vec2((float)Other.m_X, (float)Other.m_Y)), i, AttackerTeam, 1);
			if(FrozenNow(j))
				m_aActivity[j].m_LastInteractor = i;
		}
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
			continue;
		const int Hooked = GameClient()->m_aClients[i].m_Snapped.m_HookedPlayer;
		if(Hooked < 0 || Hooked >= MAX_CLIENTS || Hooked == i)
			continue;
		if(!GameClient()->m_Snap.m_aCharacters[Hooked].m_Active || !CanInteract(i, Hooked) || !FrozenNow(Hooked))
			continue;
		m_aActivity[Hooked].m_LastInteractor = i;
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		SActivityPlayer &Player = m_aActivity[i];
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
		{
			Player.m_WasActive = false;
			Player.m_WasFrozen = false;
			Player.m_LastInteractor = -1;
			Player.m_PrevAttackTick = -1;
			continue;
		}

		const CNetObj_Character &Cur = GameClient()->m_aClients[i].m_Snapped;
		const vec2 Pos((float)Cur.m_X, (float)Cur.m_Y);
		const int Tile = ActivityTile(pCollision, Pos);
		const int Team = GameClient()->m_Teams.Team(i);
		Player.m_Team = Team;
		const bool Frozen = FrozenNow(i);
		const bool WasActive = Player.m_WasActive;

		if(WasActive && (Cur.m_Jumped & 2) && !(Player.m_PrevJumped & 2))
		{
			Player.m_Jumps++;
			NoteActivity(Tile, i, Team, 0);
		}

		if(WasActive && !Player.m_WasFrozen && Frozen)
		{
			Player.m_Freezes++;
			NoteActivity(Tile, i, Team, 3);
		}
		if(WasActive && Player.m_WasFrozen && !Frozen)
		{
			const int Interactor = Player.m_LastInteractor;
			if(Interactor >= 0 && Interactor < MAX_CLIENTS && Interactor != i)
			{
				m_aActivity[Interactor].m_Saves++;
				NoteActivity(Tile, Interactor, GameClient()->m_Teams.Team(Interactor), 2);
			}
			Player.m_LastInteractor = -1;
		}
		if(!Player.m_WasFrozen && Frozen)
			Player.m_LastInteractor = -1;

		Player.m_PrevJumped = Cur.m_Jumped;
		Player.m_PrevAttackTick = Cur.m_AttackTick;
		Player.m_WasFrozen = Frozen;
		Player.m_WasActive = true;
	}
}

static bool ActivityKindEnabled(int Kind)
{
	switch(Kind)
	{
	case 0: return g_Config.m_BclMinimapShowJumps != 0;
	case 1: return g_Config.m_BclMinimapShowHits != 0;
	case 2: return g_Config.m_BclMinimapShowSaves != 0;
	case 3: return g_Config.m_BclMinimapShowFreezes != 0;
	default: return false;
	}
}

void CMinimap::DrawActivity(float X, float Y, float W, float H, vec2 WorldMin, float Scale, float Alpha, int LocalId, float CursorX, float CursorY, bool CursorValid)
{
	m_MarkerHoverValid = false;
	m_MarkerHoverIndex = -1;
	m_MarkerHoverClient = -1;
	m_MarkerHoverDist = std::numeric_limits<float>::max();
	if(m_vActivity.empty() || m_KindsWidth <= 0)
		return;

	const int LocalDDTeam = LocalId >= 0 ? GameClient()->m_Teams.Team(LocalId) : TEAM_FLOCK;
	const float Outer = std::clamp(3.2f + Scale * 6.0f, 3.2f, 6.5f);
	const float Inner = Outer * 0.78f;
	static const ColorRGBA s_aColors[4] = {
		ColorRGBA(0.30f, 0.82f, 0.42f, 1.0f),
		ColorRGBA(0.95f, 0.76f, 0.18f, 1.0f),
		ColorRGBA(0.35f, 0.68f, 1.0f, 1.0f),
		ColorRGBA(0.96f, 0.38f, 0.12f, 1.0f),
	};

	auto PushDiamond = [&](vec2 Center) {
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
			m_vMarkerQuads.emplace_back(aInner[i], aInner[Next], aOuter[i], aOuter[Next]);
		}
	};

	Graphics()->TextureClear();
	for(int Kind = 0; Kind < 4; Kind++)
	{
		m_vMarkerQuads.clear();
		for(int MarkerIndex = 0; MarkerIndex < (int)m_vActivity.size(); ++MarkerIndex)
		{
			const SActivityMarker &Marker = m_vActivity[MarkerIndex];
			if(Marker.m_Kind != Kind || !ActivityKindEnabled(Kind))
				continue;
			if(LocalDDTeam > TEAM_FLOCK && Marker.m_Team != LocalDDTeam)
				continue;
			if(Marker.m_Tile < 0)
				continue;
			const int TileX = Marker.m_Tile % m_KindsWidth;
			const int TileY = Marker.m_Tile / m_KindsWidth;
			const vec2 World((TileX + 0.5f) * 32.0f, (TileY + 0.5f) * 32.0f);
			const vec2 Dot(X + (World.x - WorldMin.x) * Scale, Y + (World.y - WorldMin.y) * Scale);
			if(Dot.x < X - Outer || Dot.x > X + W + Outer || Dot.y < Y - Outer || Dot.y > Y + H + Outer)
				continue;
			if(CursorValid)
			{
				const float Dx = CursorX - Dot.x;
				const float Dy = CursorY - Dot.y;
				const float DistSq = Dx * Dx + Dy * Dy;
				const float Radius = Outer + 3.0f;
				if(DistSq <= Radius * Radius && DistSq < m_MarkerHoverDist)
				{
					m_MarkerHoverDist = DistSq;
					m_MarkerHoverIndex = MarkerIndex;
					m_MarkerHoverKind = Kind;
					m_MarkerHoverClient = Marker.m_ClientId;
					m_MarkerHoverCount = Marker.m_Count;
					m_MarkerHoverPos = Dot;
					m_MarkerHoverValid = true;
				}
			}
			PushDiamond(Dot);
		}
		if(m_vMarkerQuads.empty())
			continue;
		Graphics()->QuadsBegin();
		const ColorRGBA Color = s_aColors[Kind].WithAlpha(0.92f * Alpha);
		Graphics()->SetColor(Color);
		Graphics()->QuadsDrawFreeform(m_vMarkerQuads.data(), (int)m_vMarkerQuads.size());
		Graphics()->QuadsEnd();
	}
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CMinimap::RenderWorldMarkers()
{
	if(g_Config.m_BclMinimapMarksInGame == 0 || m_vActivity.empty() || m_KindsWidth <= 0)
		return;

	const int LocalId = GameClient()->m_Snap.m_LocalClientId;
	const int LocalDDTeam = LocalId >= 0 ? GameClient()->m_Teams.Team(LocalId) : TEAM_FLOCK;
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
	static const ColorRGBA s_aColors[4] = {
		ColorRGBA(0.30f, 0.82f, 0.42f, 1.0f),
		ColorRGBA(0.95f, 0.76f, 0.18f, 1.0f),
		ColorRGBA(0.35f, 0.68f, 1.0f, 1.0f),
		ColorRGBA(0.96f, 0.38f, 0.12f, 1.0f),
	};

	Graphics()->TextureClear();
	for(int Kind = 0; Kind < 4; Kind++)
	{
		if(!ActivityKindEnabled(Kind))
			continue;
		m_vMarkerQuads.clear();
		for(const SActivityMarker &Marker : m_vActivity)
		{
			if(Marker.m_Kind != Kind)
				continue;
			if(LocalDDTeam > TEAM_FLOCK && Marker.m_Team != LocalDDTeam)
				continue;
			if(Marker.m_Tile < 0)
				continue;
			const float CenterX = (Marker.m_Tile % m_KindsWidth) * 32.0f + 16.0f;
			const float CenterY = (Marker.m_Tile / m_KindsWidth) * 32.0f + 16.0f;
			if(CenterX < ScreenL - 24.0f || CenterX > ScreenR + 80.0f || CenterY < ScreenT - 24.0f || CenterY > ScreenB + 24.0f)
				continue;
			constexpr float Outer = 11.0f;
			constexpr float Inner = 9.35f;
			const vec2 Center(CenterX, CenterY);
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
				m_vMarkerQuads.emplace_back(aInner[i], aInner[Next], aOuter[i], aOuter[Next]);
			}
		}
		if(m_vMarkerQuads.empty())
			continue;
		Graphics()->QuadsBegin();
		Graphics()->SetColor(s_aColors[Kind]);
		Graphics()->QuadsDrawFreeform(m_vMarkerQuads.data(), (int)m_vMarkerQuads.size());
		Graphics()->QuadsEnd();
	}
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	constexpr float FontSize = 8.0f;
	const ColorRGBA PreviousTextColor = TextRender()->GetTextColor();
	const ColorRGBA PreviousOutlineColor = TextRender()->GetTextOutlineColor();
	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->TextOutlineColor(TextRender()->DefaultTextOutlineColor());
	for(const SActivityMarker &Marker : m_vActivity)
	{
		if(!ActivityKindEnabled(Marker.m_Kind))
			continue;
		if(LocalDDTeam > TEAM_FLOCK && Marker.m_Team != LocalDDTeam)
			continue;
		if(Marker.m_Tile < 0 || Marker.m_ClientId < 0 || Marker.m_ClientId >= MAX_CLIENTS)
			continue;
		const float CenterX = (Marker.m_Tile % m_KindsWidth) * 32.0f + 16.0f;
		const float CenterY = (Marker.m_Tile / m_KindsWidth) * 32.0f + 16.0f;
		if(CenterX < ScreenL - 24.0f || CenterX > ScreenR + 80.0f || CenterY < ScreenT - 40.0f || CenterY > ScreenB + 24.0f)
			continue;
		const char *pNick = GameClient()->m_aClients[Marker.m_ClientId].m_aName;
		char aLabel[64];
		if(pNick && pNick[0])
			str_format(aLabel, sizeof(aLabel), "%s x%d", pNick, Marker.m_Count);
		else
			str_format(aLabel, sizeof(aLabel), "x%d", Marker.m_Count);
		const float TextH = TextRender()->TextBoundingBox(FontSize, aLabel).m_H;
		const float Stack = (float)(Marker.m_ClientId % 4) * (TextH + 1.0f);
		TextRender()->Text(CenterX + 13.0f, CenterY - TextH * 0.5f - Stack, FontSize, aLabel);
	}
	TextRender()->TextColor(PreviousTextColor);
	TextRender()->TextOutlineColor(PreviousOutlineColor);
	Graphics()->MapScreen(PreviousScreen);
}

void CMinimap::ReleaseIdle()
{
	if(m_ActivityOn || !m_vActivity.empty())
		ClearActivity();
	if(m_KindsWidth == 0 && m_vKinds.empty() && m_vTileQuads.empty())
		return;
	m_vKinds.clear();
	m_vVisual.clear();
	m_vFront.clear();
	m_vSwitch.clear();
	m_vPlatform.clear();
	m_vTileQuads.clear();
	m_vHookLines.clear();
	m_vLaserLines.clear();
	m_vMarkerQuads.clear();
	m_vKinds.shrink_to_fit();
	m_vVisual.shrink_to_fit();
	m_vFront.shrink_to_fit();
	m_vSwitch.shrink_to_fit();
	m_vPlatform.shrink_to_fit();
	m_vTileQuads.shrink_to_fit();
	m_vHookLines.shrink_to_fit();
	m_vLaserLines.shrink_to_fit();
	m_vMarkerQuads.shrink_to_fit();
	m_KindsCollision = nullptr;
	m_KindsWidth = 0;
	m_KindsHeight = 0;
	m_HasSwitchTiles = false;
}

void CMinimap::OnNewSnapshot()
{
	if(g_Config.m_BclMinimap == 0)
	{
		if(m_LargeActive)
			CloseLarge();
		ReleaseIdle();
		SampleActivity();
		return;
	}
	SampleActivity();
}

bool CMinimap::EnsureCache(CCollision *pCollision)
{
	if(!pCollision || !pCollision->GameLayer() || pCollision->GetWidth() <= 0 || pCollision->GetHeight() <= 0)
		return false;

	const int CollisionW = pCollision->GetWidth();
	const int CollisionH = pCollision->GetHeight();
	if(m_KindsCollision == pCollision && m_KindsWidth == CollisionW && m_KindsHeight == CollisionH && (int)m_vKinds.size() == CollisionW * CollisionH && m_vVisual.size() == m_vKinds.size())
		return true;

	m_KindsCollision = pCollision;
	m_KindsWidth = CollisionW;
	m_KindsHeight = CollisionH;
	const size_t TileCount = (size_t)CollisionW * (size_t)CollisionH;
	m_vKinds.assign(TileCount, (uint8_t)EMinimapTileKind::Empty);
	m_vVisual.assign(TileCount, 0);
	m_vFront.assign(TileCount, 0);
	m_vSwitch.assign(TileCount, 0);
	m_HasSwitchTiles = false;
	const CTile *pGame = pCollision->GameLayer();
	const CTile *pFront = pCollision->FrontLayer();
	const CTeleTile *pTele = pCollision->TeleLayer();
	const CSwitchTile *pSwitch = pCollision->SwitchLayer();
	for(int Ty = 0; Ty < CollisionH; ++Ty)
	{
		for(int Tx = 0; Tx < CollisionW; ++Tx)
		{
			const size_t Idx = (size_t)Ty * (size_t)CollisionW + (size_t)Tx;
			m_vKinds[Idx] = (uint8_t)MinimapTileKind(pCollision, Tx, Ty);
			if(pGame[Idx].m_Index)
				m_vVisual[Idx] = PackMinimapVisual(pGame[Idx].m_Index, pGame[Idx].m_Flags);
			else if(pTele && pTele[Idx].m_Type)
				m_vVisual[Idx] = PackMinimapVisual(pTele[Idx].m_Type, 0);
			if(pFront && pFront[Idx].m_Index)
				m_vFront[Idx] = PackMinimapVisual(pFront[Idx].m_Index, pFront[Idx].m_Flags);
			if(pSwitch && pSwitch[Idx].m_Type)
			{
				m_vSwitch[Idx] = PackMinimapVisual(pSwitch[Idx].m_Type, pSwitch[Idx].m_Flags);
				m_HasSwitchTiles = true;
			}
		}
	}

	auto Flood = [&](EMinimapTileKind SeedKind, EMinimapTileKind BarrierKind, std::vector<uint8_t> &vOut) -> bool {
		vOut.assign(TileCount, 0);
		std::vector<int> vQueue;
		vQueue.reserve(TileCount / 4 + 16);
		auto Passable = [&](size_t Idx) {
			const EMinimapTileKind Kind = (EMinimapTileKind)m_vKinds[Idx];
			return Kind != EMinimapTileKind::Solid && Kind != EMinimapTileKind::Death && Kind != BarrierKind;
		};
		bool HasSeed = false;
		for(size_t i = 0; i < TileCount; ++i)
		{
			if((EMinimapTileKind)m_vKinds[i] != SeedKind)
				continue;
			HasSeed = true;
			if(Passable(i) && !vOut[i])
			{
				vOut[i] = 1;
				vQueue.push_back((int)i);
			}
		}
		for(size_t Head = 0; Head < vQueue.size(); ++Head)
		{
			const int Idx = vQueue[Head];
			const int Tx = Idx % CollisionW;
			const int Ty = Idx / CollisionW;
			const int aNb[4] = {
				Tx > 0 ? Idx - 1 : -1,
				Tx + 1 < CollisionW ? Idx + 1 : -1,
				Ty > 0 ? Idx - CollisionW : -1,
				Ty + 1 < CollisionH ? Idx + CollisionW : -1,
			};
			for(int Nb : aNb)
			{
				if(Nb < 0 || vOut[Nb] || !Passable((size_t)Nb))
					continue;
				vOut[Nb] = 1;
				vQueue.push_back(Nb);
			}
		}
		return HasSeed;
	};

	std::vector<uint8_t> vFromStart, vFromFinish;
	const bool HasStart = Flood(EMinimapTileKind::Start, EMinimapTileKind::Finish, vFromStart);
	const bool HasFinish = Flood(EMinimapTileKind::Finish, EMinimapTileKind::Start, vFromFinish);
	auto OnRoute = [&](size_t Idx) -> bool {
		if(HasStart && HasFinish)
			return vFromStart[Idx] && vFromFinish[Idx];
		if(HasStart)
			return vFromStart[Idx] != 0;
		if(HasFinish)
			return vFromFinish[Idx] != 0;
		return false;
	};
	auto KindCached = [&](int Tx, int Ty) -> EMinimapTileKind {
		if(Tx < 0 || Ty < 0 || Tx >= CollisionW || Ty >= CollisionH)
			return EMinimapTileKind::Empty;
		return (EMinimapTileKind)m_vKinds[(size_t)Ty * (size_t)CollisionW + (size_t)Tx];
	};

	m_vPlatform.assign(TileCount, 0);
	for(int Ty = 1; Ty < CollisionH; ++Ty)
	{
		for(int Tx = 0; Tx < CollisionW; ++Tx)
		{
			const size_t Idx = (size_t)Ty * (size_t)CollisionW + (size_t)Tx;
			if((EMinimapTileKind)m_vKinds[Idx] != EMinimapTileKind::Solid)
				continue;
			const size_t Above = (size_t)(Ty - 1) * (size_t)CollisionW + (size_t)Tx;
			const EMinimapTileKind AboveKind = (EMinimapTileKind)m_vKinds[Above];
			const bool Stand = AboveKind == EMinimapTileKind::Empty || AboveKind == EMinimapTileKind::Unfreeze || AboveKind == EMinimapTileKind::Start;
			if(!Stand || ((HasStart || HasFinish) && !OnRoute(Above)))
				continue;
			const EMinimapTileKind Above2 = KindCached(Tx, Ty - 2);
			if(Above2 == EMinimapTileKind::Freeze || Above2 == EMinimapTileKind::Death)
				continue;
			m_vPlatform[Idx] = 1;
		}
	}
	return true;
}

void CMinimap::DrawEntityQuads(const std::vector<uint16_t> &vCells, IGraphics::CTextureHandle Texture, int StartX, int StartY, int EndX, int EndY, float X, float Y, float W, float H, vec2 WorldMin, float Scale, float Alpha)
{
	if(!Texture.IsValid() || vCells.empty() || StartX >= EndX || StartY >= EndY)
		return;
	const int CollisionW = m_KindsWidth;
	const int CollisionH = m_KindsHeight;
	if(CollisionW <= 0 || CollisionH <= 0)
		return;

	auto Accept = [&](int Tx, int Ty, uint16_t &Cell, float &DrawX, float &DrawY) -> bool {
		if(Tx < 0 || Ty < 0 || Tx >= CollisionW || Ty >= CollisionH)
			return false;
		const size_t Idx = (size_t)Ty * (size_t)CollisionW + (size_t)Tx;
		if(Idx >= vCells.size())
			return false;
		Cell = vCells[Idx];
		if((Cell & 255) == 0)
			return false;
		DrawX = X + (Tx * 32.0f - WorldMin.x) * Scale;
		DrawY = Y + (Ty * 32.0f - WorldMin.y) * Scale;
		const float TileSpan = 32.0f * Scale;
		return DrawX < X + W && DrawY < Y + H && DrawX + TileSpan > X && DrawY + TileSpan > Y;
	};

	std::array<int, 2048> aCount{};
	int Total = 0;
	for(int Ty = StartY; Ty < EndY; ++Ty)
	{
		for(int Tx = StartX; Tx < EndX; ++Tx)
		{
			uint16_t Cell = 0;
			float DrawX = 0.0f;
			float DrawY = 0.0f;
			if(!Accept(Tx, Ty, Cell, DrawX, DrawY))
				continue;
			const int Flags = Cell >> 8;
			const int FlagId = (Flags & 3) | ((Flags & TILEFLAG_ROTATE) ? 4 : 0);
			const int Key = (Cell & 255) | (FlagId << 8);
			++aCount[Key];
			++Total;
		}
	}
	if(Total <= 0)
		return;

	m_vTileQuads.resize((size_t)Total);
	std::array<int, 2048> aOffset{};
	int Cursor = 0;
	for(int Key = 0; Key < 2048; ++Key)
	{
		aOffset[Key] = Cursor;
		Cursor += aCount[Key];
	}
	std::array<int, 2048> aWrite = aOffset;
	const float ScreenW = std::max(Graphics()->GetScreen().Width(), 1.0f);
	const float Pixel = ScreenW / std::max((float)Graphics()->ScreenWidth(), 1.0f);
	const float TileSpan = 32.0f * Scale;
	const float Bleed = TileSeamBleed(Pixel, TileSpan);
	const float TileDraw = TileSpan + Bleed * 2.0f;
	for(int Ty = StartY; Ty < EndY; ++Ty)
	{
		for(int Tx = StartX; Tx < EndX; ++Tx)
		{
			uint16_t Cell = 0;
			float DrawX = 0.0f;
			float DrawY = 0.0f;
			if(!Accept(Tx, Ty, Cell, DrawX, DrawY))
				continue;
			const int Flags = Cell >> 8;
			const int FlagId = (Flags & 3) | ((Flags & TILEFLAG_ROTATE) ? 4 : 0);
			const int Key = (Cell & 255) | (FlagId << 8);
			m_vTileQuads[(size_t)aWrite[Key]++] = IGraphics::CQuadItem(DrawX - Bleed, DrawY - Bleed, TileDraw, TileDraw);
		}
	}

	const float PixelTile = (32.0f * Scale) / ScreenW * (float)Graphics()->ScreenWidth();
	const float TilesetScale = std::max(PixelTile / 32.0f, 0.02f);
	const float MaxInset = 10.0f / 1024.0f;
	const float Frac = std::min((1.25f / 1024.0f) / TilesetScale, MaxInset);
	const float Nudge = std::min((0.5f / 1024.0f) / TilesetScale, MaxInset);
	const bool Arrays = Graphics()->HasTextureArraysSupport();

	Graphics()->WrapClamp();
	Graphics()->TextureSet(Texture);
	if(Arrays)
		Graphics()->QuadsTex3DBegin();
	else
		Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);

	constexpr int Chunk = 2048;
	for(int Key = 0; Key < 2048; ++Key)
	{
		if(aCount[Key] <= 0)
			continue;
		const int Index = Key & 255;
		const int FlagId = Key >> 8;
		const int Flags = (FlagId & 3) | ((FlagId & 4) ? TILEFLAG_ROTATE : 0);
		float x0, y0, x1, y1, x2, y2, x3, y3;
		MinimapEntityUv(Index, Flags, Arrays, Frac, Nudge, x0, y0, x1, y1, x2, y2, x3, y3);
		if(Arrays)
			Graphics()->QuadsSetSubsetFree(x0, y0, x1, y1, x2, y2, x3, y3, Index);
		else
			Graphics()->QuadsSetSubsetFree(x0, y0, x1, y1, x2, y2, x3, y3);
		int Left = aCount[Key];
		int Off = aOffset[Key];
		while(Left > 0)
		{
			const int Batch = std::min(Left, Chunk);
			if(Arrays)
			{
				Graphics()->QuadsTex3DDrawTL(&m_vTileQuads[(size_t)Off], Batch);
				Graphics()->FlushVerticesTex3D();
			}
			else
			{
				Graphics()->QuadsDrawTL(&m_vTileQuads[(size_t)Off], Batch);
				Graphics()->FlushVertices();
			}
			Off += Batch;
			Left -= Batch;
		}
	}

	if(Arrays)
		Graphics()->QuadsTex3DEnd();
	else
		Graphics()->QuadsEnd();
	Graphics()->WrapNormal();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CMinimap::DrawTiles(float X, float Y, float W, float H, vec2 WorldMin, vec2 WorldMax, float Scale, float Alpha, bool Platforms)
{
	const int CollisionW = m_KindsWidth;
	const int CollisionH = m_KindsHeight;
	const int StartTileX = std::max((int)std::floor(WorldMin.x / 32.0f) - 1, 0);
	const int StartTileY = std::max((int)std::floor(WorldMin.y / 32.0f) - 1, 0);
	const int EndTileX = std::min((int)std::ceil(WorldMax.x / 32.0f) + 1, CollisionW);
	const int EndTileY = std::min((int)std::ceil(WorldMax.y / 32.0f) + 1, CollisionH);

	const IGraphics::CTextureHandle Entities = GameClient()->m_MapImages.GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH);
	if(Entities.IsValid())
	{
		DrawEntityQuads(m_vVisual, Entities, StartTileX, StartTileY, EndTileX, EndTileY, X, Y, W, H, WorldMin, Scale, Alpha);
		DrawEntityQuads(m_vFront, Entities, StartTileX, StartTileY, EndTileX, EndTileY, X, Y, W, H, WorldMin, Scale, Alpha);
		if(m_HasSwitchTiles)
			DrawEntityQuads(m_vSwitch, GameClient()->m_MapImages.GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH), StartTileX, StartTileY, EndTileX, EndTileY, X, Y, W, H, WorldMin, Scale, Alpha);
	}
	else
	{
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		const float ScreenW = std::max(Graphics()->GetScreen().Width(), 1.0f);
		const float Pixel = ScreenW / std::max((float)Graphics()->ScreenWidth(), 1.0f);
		const float TileSpan = 32.0f * Scale;
		const float Bleed = TileSeamBleed(Pixel, TileSpan);
		const float TileDrawSize = TileSpan + Bleed * 2.0f;
		for(int TileY = StartTileY; TileY < EndTileY; ++TileY)
		{
			for(int TileX = StartTileX; TileX < EndTileX; ++TileX)
			{
				if(TileX < 0 || TileY < 0 || TileX >= CollisionW || TileY >= CollisionH)
					continue;
				const EMinimapTileKind Kind = (EMinimapTileKind)m_vKinds[(size_t)TileY * (size_t)CollisionW + (size_t)TileX];
				const ColorRGBA Color = MinimapTileColor(Kind, Alpha);
				if(Color.a <= 0.0f)
					continue;
				const float DrawX = X + (TileX * 32.0f - WorldMin.x) * Scale - Bleed;
				const float DrawY = Y + (TileY * 32.0f - WorldMin.y) * Scale - Bleed;
				IGraphics::CQuadItem Quad(DrawX, DrawY, TileDrawSize, TileDrawSize);
				Graphics()->SetColor(Color);
				Graphics()->QuadsDrawTL(&Quad, 1);
			}
		}
		Graphics()->QuadsEnd();
	}

	auto DrawBlock = [&](float DrawX, float DrawY, float DrawW, float DrawH, ColorRGBA Color) {
		if(DrawX >= X + W || DrawY >= Y + H || DrawX + DrawW <= X || DrawY + DrawH <= Y)
			return;
		const float ClippedX0 = std::clamp(DrawX, X, X + W);
		const float ClippedY0 = std::clamp(DrawY, Y, Y + H);
		const float ClippedX1 = std::clamp(DrawX + DrawW, X, X + W);
		const float ClippedY1 = std::clamp(DrawY + DrawH, Y, Y + H);
		if(ClippedX1 <= ClippedX0 || ClippedY1 <= ClippedY0)
			return;
		IGraphics::CQuadItem Quad(ClippedX0, ClippedY0, ClippedX1 - ClippedX0, ClippedY1 - ClippedY0);
		Graphics()->SetColor(Color);
		Graphics()->QuadsDrawTL(&Quad, 1);
	};

	if(Platforms && !m_vPlatform.empty())
	{
		const ColorRGBA HiColor(1.0f, 0.85f, 0.15f, std::clamp(0.85f * Alpha + 0.15f, 0.0f, 1.0f));
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		for(int TileY = StartTileY; TileY < EndTileY; ++TileY)
		{
			for(int TileX = StartTileX; TileX < EndTileX; ++TileX)
			{
				const size_t Idx = (size_t)TileY * (size_t)CollisionW + (size_t)TileX;
				if(Idx >= m_vPlatform.size() || !m_vPlatform[Idx])
					continue;
				int Run = 1;
				while(TileX + Run < EndTileX)
				{
					const size_t Next = (size_t)TileY * (size_t)CollisionW + (size_t)(TileX + Run);
					if(Next >= m_vPlatform.size() || !m_vPlatform[Next])
						break;
					++Run;
				}
				const float DrawX = X + (TileX * 32.0f - WorldMin.x) * Scale;
				const float DrawY = Y + (TileY * 32.0f - WorldMin.y) * Scale;
				const float DrawW = std::max(Run * 32.0f * Scale, 2.0f);
				const float DrawH = std::max(32.0f * Scale, 2.0f);
				DrawBlock(DrawX, DrawY, DrawW, DrawH, HiColor);
				TileX += Run - 1;
			}
		}
		Graphics()->QuadsEnd();
	}
}

struct SFullMapChrome
{
	float m_Left = 0.0f;
	float m_RowW = 82.0f;
	float m_RowH = 13.0f;
	float m_Gap = 3.0f;
	float m_ArrowX = 0.0f;
	float m_ArrowY = 0.0f;
	float m_ArrowW = 22.0f;
	float m_ArrowH = 11.0f;
	bool m_Expanded = false;
	int m_NumRows = 0;
	float m_RowsTop = 0.0f;
	float m_BoundTop = 0.0f;
	float m_SliderX = 0.0f;
	float m_SliderY = 0.0f;
	float m_SliderW = 8.0f;
	float m_SliderH = 120.0f;
	int m_ClearRow = 0;

	float RowY(int Index) const { return m_RowsTop + Index * (m_RowH + m_Gap); }
	bool Hit(float X, float Y) const
	{
		const float Right = m_Left + std::max(m_RowW, m_ArrowW);
		const bool Menu = X >= m_Left && X <= Right && Y >= m_BoundTop && Y <= m_ArrowY + m_ArrowH;
		const bool Slider = X >= m_SliderX - 6.0f && X <= m_SliderX + m_SliderW + 6.0f && Y >= m_SliderY - 4.0f && Y <= m_SliderY + m_SliderH + 4.0f;
		return Menu || Slider;
	}
};

static const char *const s_apFullMapLabels[] = {"Jumps", "Hits", "Saves", "Freezes", "Platforms", "Display marks in game"};

static SFullMapChrome MakeFullMapChrome(ITextRender *pTextRender, float MapX, float MapY, float MapW, float MapH, bool Expanded)
{
	SFullMapChrome Menu;
	const float Margin = 6.0f;
	float LabelW = 0.0f;
	for(const char *pLabel : s_apFullMapLabels)
		LabelW = std::max(LabelW, pTextRender->TextWidth(7.0f, BcLocalize(pLabel)));
	const float ClearW = pTextRender->TextWidth(7.0f, BcLocalize("Clear marks"));
	LabelW = std::max(LabelW, ClearW);
	Menu.m_RowW = std::max(82.0f, LabelW + 16.0f);
	Menu.m_ClearRow = (int)std::size(s_apFullMapLabels);
	Menu.m_NumRows = Menu.m_ClearRow + 1;
	Menu.m_Expanded = Expanded;
	Menu.m_Left = MapX + Margin;
	Menu.m_ArrowX = MapX + Margin;
	Menu.m_ArrowY = MapY + MapH - Margin - Menu.m_ArrowH;
	const float RowsBlockH = Menu.m_NumRows * Menu.m_RowH + (Menu.m_NumRows - 1) * Menu.m_Gap;
	Menu.m_RowsTop = Menu.m_ArrowY - Menu.m_Gap - RowsBlockH;
	Menu.m_BoundTop = Menu.m_Expanded ? Menu.m_RowsTop : Menu.m_ArrowY;
	Menu.m_SliderW = 16.0f;
	Menu.m_SliderH = std::clamp(MapH * 0.55f, 100.0f, 210.0f);
	Menu.m_SliderX = MapX + MapW - Margin - Menu.m_SliderW;
	Menu.m_SliderY = MapY + (MapH - Menu.m_SliderH) * 0.5f;
	return Menu;
}

bool CMinimap::FullMapChromeHit(float HudX, float HudY, float MapX, float MapY, float MapW, float MapH)
{
	return MakeFullMapChrome(TextRender(), MapX, MapY, MapW, MapH, m_MenuExpanded).Hit(HudX, HudY);
}

void CMinimap::RenderFullMapChrome(float MapX, float MapY, float MapW, float MapH, bool AllowClick)
{
	const SFullMapChrome Menu = MakeFullMapChrome(TextRender(), MapX, MapY, MapW, MapH, m_MenuExpanded);
	float HudX = 0.0f;
	float HudY = 0.0f;
	if(!HudCursor(HudX, HudY))
	{
		const vec2 NativeMouse = Input()->NativeMousePos();
		HudX = NativeMouse.x / std::max((float)Graphics()->ScreenWidth(), 1.0f) * m_Width;
		HudY = NativeMouse.y / std::max((float)Graphics()->ScreenHeight(), 1.0f) * m_Height;
	}
	const bool RawPressed = Input()->NativeMousePressed(1);
	const int MinZoom = ClampLargeZoomOut(LARGE_ZOOM_MIN);
	const int CurrentZoom = m_ZoomOverride >= 0 ? m_ZoomOverride : g_Config.m_BclLargeMinimapZoom;
	const float Value = std::clamp(ZoomToDisplay(CurrentZoom, MinZoom, LARGE_ZOOM_MAX) / 100.0f, 0.0f, 1.0f);
	const float HandleH = std::min(26.0f, std::max(12.0f, Menu.m_SliderH * 0.28f));
	const float HandleW = std::min(6.0f, Menu.m_SliderW);
	const float RailW = HandleW;
	const float Travel = std::max(1.0f, Menu.m_SliderH - HandleH);
	const float HandleX = Menu.m_SliderX + (Menu.m_SliderW - HandleW) * 0.5f;
	const float RailX = Menu.m_SliderX + (Menu.m_SliderW - RailW) * 0.5f;
	float HandleY = Menu.m_SliderY + Travel * (1.0f - Value);
	const bool OverSlider = HudX >= Menu.m_SliderX - 6.0f && HudX <= Menu.m_SliderX + Menu.m_SliderW + 6.0f && HudY >= Menu.m_SliderY - 4.0f && HudY <= Menu.m_SliderY + Menu.m_SliderH + 4.0f;
	const bool OverHandle = HudX >= HandleX && HudX <= HandleX + HandleW && HudY >= HandleY && HudY <= HandleY + HandleH;
	if(!RawPressed)
		m_ZoomSliderDrag = false;
	else if(AllowClick && !m_ZoomSliderDrag && OverSlider && !m_MenuPrevPressed)
	{
		m_ZoomSliderDrag = true;
		m_ZoomSliderGrab = OverHandle ? HudY - HandleY : HandleH * 0.5f;
	}
	if(m_ZoomSliderDrag)
	{
		const float T = std::clamp((HudY - m_ZoomSliderGrab - Menu.m_SliderY) / Travel, 0.0f, 1.0f);
		const int Display = (int)std::lround((1.0f - T) * 100.0f);
		const int Zoom = ZoomFromDisplay(Display, MinZoom, LARGE_ZOOM_MAX);
		if(Zoom != m_ZoomOverride)
			SetLargeZoom(Zoom);
		HandleY = Menu.m_SliderY + Travel * T;
	}
	const bool ClickEdge = AllowClick && RawPressed && !m_MenuPrevPressed && !m_ZoomSliderDrag;

	auto LabelButton = [&](float X, float Y, const char *pLabel, int *pValue) {
		const bool Hover = HudX >= X && HudX <= X + Menu.m_RowW && HudY >= Y && HudY <= Y + Menu.m_RowH;
		const bool On = *pValue != 0;
		const ColorRGBA Bg = On ? ColorRGBA(0.10f, 0.50f, 0.24f, Hover ? 0.97f : 0.88f) : ColorRGBA(0.16f, 0.16f, 0.19f, Hover ? 0.95f : 0.80f);
		Graphics()->DrawRect(X, Y, Menu.m_RowW, Menu.m_RowH, Bg, IGraphics::CORNER_ALL, 2.5f);
		const ColorRGBA Dot = On ? ColorRGBA(0.45f, 1.0f, 0.55f, 1.0f) : ColorRGBA(0.55f, 0.55f, 0.6f, 1.0f);
		Graphics()->DrawRect(X + 3.0f, Y + Menu.m_RowH / 2.0f - 2.0f, 4.0f, 4.0f, Dot, IGraphics::CORNER_ALL, 2.0f);
		TextRender()->TextColor(ColorRGBA(0.95f, 0.95f, 0.97f, 1.0f));
		TextRender()->Text(X + 10.0f, Y + (Menu.m_RowH - 7.0f) / 2.0f, 7.0f, pLabel, -1.0f);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
		if(ClickEdge && Hover)
			*pValue ^= 1;
	};

	if(Menu.m_Expanded)
	{
		int *apValues[] = {
			&g_Config.m_BclMinimapShowJumps,
			&g_Config.m_BclMinimapShowHits,
			&g_Config.m_BclMinimapShowSaves,
			&g_Config.m_BclMinimapShowFreezes,
			&g_Config.m_BclMinimapPlatforms,
			&g_Config.m_BclMinimapMarksInGame,
		};
		for(int i = 0; i < Menu.m_ClearRow; i++)
			LabelButton(Menu.m_Left, Menu.RowY(i), BcLocalize(s_apFullMapLabels[i]), apValues[i]);

		const float ClearX = Menu.m_Left;
		const float ClearY = Menu.RowY(Menu.m_ClearRow);
		const bool ClearHover = HudX >= ClearX && HudX <= ClearX + Menu.m_RowW && HudY >= ClearY && HudY <= ClearY + Menu.m_RowH;
		Graphics()->DrawRect(ClearX, ClearY, Menu.m_RowW, Menu.m_RowH, ColorRGBA(0.45f, 0.16f, 0.14f, ClearHover ? 0.97f : 0.86f), IGraphics::CORNER_ALL, 2.5f);
		TextRender()->TextColor(ColorRGBA(0.98f, 0.94f, 0.94f, 1.0f));
		TextRender()->Text(ClearX + 8.0f, ClearY + (Menu.m_RowH - 7.0f) / 2.0f, 7.0f, BcLocalize("Clear marks"), -1.0f);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
		if(ClickEdge && ClearHover)
			ClearMarkers();
	}

	{
		const ColorRGBA HandleColor = m_ZoomSliderDrag ? ColorRGBA(0.9f, 0.9f, 0.9f, 1.0f) : (OverSlider ? ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f) : ColorRGBA(0.8f, 0.8f, 0.8f, 1.0f));
		Graphics()->DrawRect(RailX, Menu.m_SliderY, RailW, Menu.m_SliderH, ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f), IGraphics::CORNER_ALL, RailW * 0.5f);
		Graphics()->DrawRect(HandleX, HandleY, HandleW, HandleH, HandleColor, IGraphics::CORNER_ALL, HandleW * 0.5f);
	}

	const bool ArrowHover = HudX >= Menu.m_ArrowX && HudX <= Menu.m_ArrowX + Menu.m_ArrowW && HudY >= Menu.m_ArrowY && HudY <= Menu.m_ArrowY + Menu.m_ArrowH;
	Graphics()->DrawRect(Menu.m_ArrowX, Menu.m_ArrowY, Menu.m_ArrowW, Menu.m_ArrowH, ColorRGBA(0.16f, 0.16f, 0.19f, ArrowHover ? 0.95f : 0.80f), IGraphics::CORNER_ALL, 2.5f);
	TextRender()->SetFontPreset(EFontPreset::ICON_FONT);
	TextRender()->TextColor(ColorRGBA(0.9f, 0.9f, 0.95f, 1.0f));
	const char *pArrow = Menu.m_Expanded ? FontIcon::CHEVRON_DOWN : FontIcon::CHEVRON_UP;
	const float ArrowSize = 6.5f;
	const float ArrowW = TextRender()->TextWidth(ArrowSize, pArrow);
	TextRender()->Text(Menu.m_ArrowX + (Menu.m_ArrowW - ArrowW) / 2.0f, Menu.m_ArrowY + (Menu.m_ArrowH - ArrowSize) / 2.0f, ArrowSize, pArrow, -1.0f);
	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
	if(ClickEdge && ArrowHover)
		m_MenuExpanded = !m_MenuExpanded;

	const char *pMapName = GameClient()->Map()->BaseName();
	if(pMapName && pMapName[0])
	{
		const float Margin = 6.0f;
		const float Pad = 5.0f;
		const float NameSize = 9.0f;
		const float NameW = TextRender()->TextWidth(NameSize, pMapName);
		const float BannerW = NameW + Pad * 2.0f;
		const float BannerH = NameSize + Pad * 2.0f;
		const float BannerX = MapX + MapW - Margin - BannerW;
		const float BannerY = MapY + MapH - Margin - BannerH;
		Graphics()->DrawRect(BannerX, BannerY, BannerW, BannerH, ColorRGBA(0.0f, 0.0f, 0.0f, 0.7f), IGraphics::CORNER_ALL, 3.0f);
		TextRender()->TextColor(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
		TextRender()->Text(BannerX + Pad, BannerY + Pad, NameSize, pMapName, -1.0f);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	}

	m_MenuPrevPressed = RawPressed;
}

void CMinimap::RenderHud(bool EditorPreview)
{
	const bool LargeMinimap = m_LargeActive && !EditorPreview;
	m_CursorOverChrome = false;
	if(!EditorPreview && (!g_Config.m_BclMinimap || (LargeMinimap == false && g_Config.m_BclMinimapHud == 0)))
		return;
	if(!EditorPreview && !LargeMinimap && !HudLayout::IsEnabled(HudLayout::MODULE_MINIMAP))
		return;
	const bool FocusHidesHud = g_Config.m_ClFocusMode && g_Config.m_ClFocusModeHideHud;
	if(!EditorPreview && !LargeMinimap && FocusHidesHud)
		return;

	CCollision *pCollision = Collision();
	if(!EnsureCache(pCollision))
		return;

	UpdateScreen();
	Graphics()->MapScreenToSize(m_Width, m_Height);

	vec2 Center = GameClient()->m_Camera.m_Center;
	const int LocalId = GameClient()->m_Snap.m_LocalClientId;
	if(!GameClient()->m_Snap.m_SpecInfo.m_Active && LocalId >= 0 && GameClient()->m_Snap.m_aCharacters[LocalId].m_Active)
		Center = GameClient()->m_aClients[LocalId].m_RenderPos;
	else if(GameClient()->m_Snap.m_SpecInfo.m_Active)
	{
		const int SpectatorId = GameClient()->m_Snap.m_SpecInfo.m_SpectatorId;
		if(SpectatorId >= 0 && SpectatorId < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[SpectatorId].m_Active)
			Center = GameClient()->m_aClients[SpectatorId].m_RenderPos;
		else if(GameClient()->m_Snap.m_SpecInfo.m_UsePosition)
			Center = GameClient()->m_Snap.m_SpecInfo.m_Position;
	}

	const float LargeBaseSize = std::min(m_Height, m_Width);
	const float LargeScale = LargeMinimap && m_ForceFullSize ? 1.0f : LARGE_MAP_SCREEN_SCALE;
	float Size = 0.0f;
	float X = 0.0f;
	float Y = 0.0f;
	if(LargeMinimap)
	{
		Size = LargeBaseSize * LargeScale;
		const float MinimapWPreview = m_Width * LargeScale;
		const float MinimapHPreview = m_Height * LargeScale;
		X = (m_Width - MinimapWPreview - 4.0f) * 0.5f + 2.0f;
		Y = (m_Height - MinimapHPreview - 4.0f) * 0.5f + 2.0f;
	}
	else
		SmallMapRect(m_Width, m_Height, X, Y, Size);
	const float MinimapW = LargeMinimap ? m_Width * LargeScale : Size;
	const float MinimapH = LargeMinimap ? m_Height * LargeScale : Size;
	const bool FullMap = LargeMinimap && m_ForceFullSize;
	const float AlphaMin = FullMap ? 0.0f : 0.1f;
	const float ConfigAlpha = (FullMap ? g_Config.m_BclLargeMinimapAlpha : g_Config.m_BclMinimapAlpha) / 100.0f;
	const float ModuleAlpha = LargeMinimap ? 1.0f : HudLayout::AlphaFactor(HudLayout::MODULE_MINIMAP);
	const float Alpha = std::clamp(ConfigAlpha * ModuleAlpha, AlphaMin, 1.0f);
	const int ConfiguredZoom = LargeMinimap ? (m_ZoomOverride >= 0 ? m_ZoomOverride : g_Config.m_BclLargeMinimapZoom) : g_Config.m_BclMinimapZoom;
	float Zoom = std::clamp(ConfiguredZoom / 100.0f, LargeMinimap ? 0.2f : 3.0f, 24.0f);
	if(!LargeMinimap && g_Config.m_BclMinimapSyncZoom)
		Zoom = std::clamp(Zoom / std::max(GameClient()->m_Camera.m_Zoom, 0.05f), 0.05f, 24.0f);
	const float MapWorldW = m_KindsWidth * 32.0f;
	const float MapWorldH = m_KindsHeight * 32.0f;
	const float ScaleReferenceW = LargeMinimap ? m_Width : MinimapW;
	const float ScaleReferenceH = LargeMinimap ? m_Height : MinimapH;
	const float FitScale = std::max(ScaleReferenceW / std::max(MapWorldW, 1.0f), ScaleReferenceH / std::max(MapWorldH, 1.0f));
	const float Scale = FitScale * Zoom;
	const float ConfiguredSmallZoom = std::clamp(g_Config.m_BclMinimapZoom / 100.0f, 3.0f, 24.0f);

	float CursorX = 0.0f;
	float CursorY = 0.0f;
	const bool HaveCursor = HudCursor(CursorX, CursorY);

	const bool UiBlocksDrag = GameClient()->m_Menus.IsActive() || GameClient()->m_GameConsole.IsActive() || GameClient()->m_Chat.IsActive();
	bool OverChrome = false;
	if(LargeMinimap && m_ForceFullSize)
	{
		float HudX = CursorX;
		float HudY = CursorY;
		if(!HaveCursor)
		{
			const vec2 NativeMouse = Input()->NativeMousePos();
			HudX = NativeMouse.x / std::max((float)Graphics()->ScreenWidth(), 1.0f) * m_Width;
			HudY = NativeMouse.y / std::max((float)Graphics()->ScreenHeight(), 1.0f) * m_Height;
		}
		OverChrome = FullMapChromeHit(HudX, HudY, X, Y, MinimapW, MinimapH);
		m_CursorOverChrome = OverChrome;
	}
	if(LargeMinimap && m_ForceFullSize && HaveCursor && !UiBlocksDrag)
	{
		const bool MousePressed = Input()->NativeMousePressed(1) && !OverChrome;
		const vec2 Cursor(CursorX, CursorY);
		if(MousePressed)
		{
			if(!m_Dragging)
			{
				m_Dragging = true;
				m_DragStart = Cursor;
				m_PrevMouse = Cursor;
				m_FollowArm = OverChrome ? -1 : m_HoveredPlayerId;
			}
			else if(length(Cursor - m_DragStart) > 4.0f)
			{
				if(m_FollowClientId >= 0 && m_FollowClientId < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[m_FollowClientId].m_Active)
					m_PanOffset = GameClient()->m_aClients[m_FollowClientId].m_RenderPos - Center;
				m_FollowClientId = -1;
				m_FollowArm = -1;
				RestoreSpecLock();
				const vec2 Step = Cursor - m_PrevMouse;
				m_PanOffset.x -= Step.x / std::max(Scale, 0.0001f);
				m_PanOffset.y -= Step.y / std::max(Scale, 0.0001f);
				m_PrevMouse = Cursor;
			}
		}
		else if(m_Dragging)
		{
			if(length(Cursor - m_DragStart) <= 4.0f)
			{
				if(m_FollowArm >= 0 && m_FollowArm < MAX_CLIENTS)
				{
					m_FollowClientId = m_FollowArm;
					m_PanOffset = vec2(0.0f, 0.0f);
					const bool SpecActive = GameClient()->m_Snap.m_SpecInfo.m_Active;
					const bool DemoFollow = Client()->State() == IClient::STATE_DEMOPLAYBACK && GameClient()->m_DemoSpecId == SPEC_FOLLOW;
					if(SpecActive && !DemoFollow)
					{
						if(!m_SpecRestoreArmed)
						{
							m_SpecRestoreId = GameClient()->m_Snap.m_SpecInfo.m_SpectatorId;
							m_SpecRestoreArmed = true;
						}
						GameClient()->m_Spectator.Spectate(m_FollowClientId);
					}
					ReleaseFollowInput();
				}
				else
				{
					m_FollowClientId = -1;
					RestoreSpecLock();
				}
			}
			m_Dragging = false;
			m_FollowArm = -1;
		}
	}
	else
	{
		m_Dragging = false;
		m_FollowArm = -1;
		if(!(LargeMinimap && m_ForceFullSize))
			m_PanOffset = vec2(0.0f, 0.0f);
	}

	const float RangePixelsX = MinimapW / std::max(Scale, 0.0001f) * 0.5f;
	const float RangePixelsY = MinimapH / std::max(Scale, 0.0001f) * 0.5f;
	vec2 ViewCenter = Center;
	if(LargeMinimap && m_ForceFullSize && m_FollowClientId >= 0 && m_FollowClientId < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[m_FollowClientId].m_Active)
		ViewCenter = GameClient()->m_aClients[m_FollowClientId].m_RenderPos;
	else if(LargeMinimap && m_ForceFullSize)
		ViewCenter += m_PanOffset;
	else
	{
		if(RangePixelsX * 2.0f >= MapWorldW)
			ViewCenter.x = MapWorldW * 0.5f;
		else
			ViewCenter.x = std::clamp(ViewCenter.x, RangePixelsX, MapWorldW - RangePixelsX);
		if(RangePixelsY * 2.0f >= MapWorldH)
			ViewCenter.y = MapWorldH * 0.5f;
		else
			ViewCenter.y = std::clamp(ViewCenter.y, RangePixelsY, MapWorldH - RangePixelsY);
	}
	const vec2 WorldMin = ViewCenter - vec2(RangePixelsX, RangePixelsY);
	const vec2 WorldMax = ViewCenter + vec2(RangePixelsX, RangePixelsY);

	const ColorRGBA BgColor = MinimapBackground(Alpha);
	int MapCorners = IGraphics::CORNER_ALL;
	const float FrameRounding = 4.0f;
	const float FillRounding = 3.0f;
	if(!LargeMinimap)
		MapCorners = HudLayout::BackgroundCorners(IGraphics::CORNER_ALL, X, Y, MinimapW, MinimapH, m_Width, m_Height);
	Graphics()->TextureClear();
	Graphics()->DrawRect(X - 1.5f, Y - 1.5f, MinimapW + 3.0f, MinimapH + 3.0f, ColorRGBA(0.0f, 0.0f, 0.0f, 0.45f * Alpha), MapCorners, FrameRounding);
	Graphics()->DrawRect(X, Y, MinimapW, MinimapH, BgColor, MapCorners, FillRounding);

	const float XScale = Graphics()->ScreenWidth() / m_Width;
	const float YScale = Graphics()->ScreenHeight() / m_Height;
	Graphics()->ClipEnable((int)std::round(X * XScale), (int)std::round(Y * YScale), (int)std::round(MinimapW * XScale), (int)std::round(MinimapH * YScale));
	DrawTiles(X, Y, MinimapW, MinimapH, WorldMin, WorldMax, Scale, Alpha, g_Config.m_BclMinimapPlatforms != 0);

	auto WorldToMinimap = [&](vec2 Pos) {
		return vec2(X + (Pos.x - WorldMin.x) * Scale, Y + (Pos.y - WorldMin.y) * Scale);
	};
	auto InMinimapShape = [&](vec2 Pos) {
		return Pos.x >= X && Pos.x <= X + MinimapW && Pos.y >= Y && Pos.y <= Y + MinimapH;
	};

	DrawActivity(X, Y, MinimapW, MinimapH, WorldMin, Scale, Alpha, LocalId, CursorX, CursorY, HaveCursor && !UiBlocksDrag);

	const bool FullHover = LargeMinimap && m_ForceFullSize && HaveCursor && !UiBlocksDrag;
	const float HudMouseX = FullHover ? CursorX : 0.0f;
	const float HudMouseY = FullHover ? CursorY : 0.0f;
	const bool MouseOver = FullHover && HudMouseX >= X && HudMouseX <= X + MinimapW && HudMouseY >= Y && HudMouseY <= Y + MinimapH;
	int HoveredPlayerId = -1;
	vec2 HoveredPlayerDot(0.0f, 0.0f);
	float HoveredPlayerDistSq = std::numeric_limits<float>::max();

	if(g_Config.m_BclMinimapPlayers)
	{
		const int CurTick = Client()->GameTick(g_Config.m_ClDummy);
		const int LocalDDTeam = LocalId >= 0 ? GameClient()->m_Teams.Team(LocalId) : TEAM_FLOCK;
		const int LocalTeam = LocalId >= 0 ? GameClient()->m_aClients[LocalId].m_Team : TEAM_SPECTATORS;
		const bool LocalAlive = LocalId >= 0 && !(GameClient()->m_aClients[LocalId].m_FreezeEnd != 0 && GameClient()->m_aClients[LocalId].m_FreezeEnd > CurTick);
		bool AnyOtherAlive = false;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == LocalId || !GameClient()->m_Snap.m_apPlayerInfos[i] || !GameClient()->m_Snap.m_aCharacters[i].m_Active)
				continue;
			const int OtherDDTeam = GameClient()->m_Teams.Team(i);
			const bool SameTeam = LocalDDTeam > TEAM_FLOCK ? OtherDDTeam == LocalDDTeam : GameClient()->m_aClients[i].m_Team == LocalTeam;
			const bool Frozen = GameClient()->m_aClients[i].m_DeepFrozen || (GameClient()->m_aClients[i].m_FreezeEnd != 0 && GameClient()->m_aClients[i].m_FreezeEnd > CurTick);
			if(SameTeam && !Frozen)
				AnyOtherAlive = true;
		}
		const bool LastManStanding = LocalAlive && !AnyOtherAlive;

		static const ColorRGBA TintAlive = color_cast<ColorRGBA>(ColorHSLA(MINIMAP_COLOR_ALIVE, false));
		static const ColorRGBA TintFrozen = color_cast<ColorRGBA>(ColorHSLA(MINIMAP_COLOR_FROZEN, false));
		static const ColorRGBA TintCritical = color_cast<ColorRGBA>(ColorHSLA(MINIMAP_COLOR_CRITICAL, false));

		m_vHookLines.clear();
		m_vLaserLines.clear();
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameClient()->m_Snap.m_apPlayerInfos[i] || !GameClient()->m_Snap.m_aCharacters[i].m_Active)
				continue;
			const int OtherDDTeam = GameClient()->m_Teams.Team(i);
			const bool SameTeam = LocalDDTeam > TEAM_FLOCK ? OtherDDTeam == LocalDDTeam : GameClient()->m_aClients[i].m_Team == LocalTeam;
			if(!SameTeam && !g_Config.m_BclMinimapShowOthers)
				continue;

			const vec2 Pos = GameClient()->m_aClients[i].m_RenderPos;
			if(Pos.x < WorldMin.x || Pos.x > WorldMax.x || Pos.y < WorldMin.y || Pos.y > WorldMax.y)
				continue;
			const vec2 Dot = WorldToMinimap(Pos);
			if(!InMinimapShape(Dot))
				continue;

			const bool Frozen = GameClient()->m_aClients[i].m_DeepFrozen || (GameClient()->m_aClients[i].m_FreezeEnd != 0 && GameClient()->m_aClients[i].m_FreezeEnd > CurTick);
			const CNetObj_Character &PrevChar = GameClient()->m_aClients[i].m_RenderPrev;
			const CNetObj_Character &CurChar = GameClient()->m_aClients[i].m_RenderCur;
			if(CurChar.m_HookState > 0)
			{
				vec2 HookPos;
				if(in_range(CurChar.m_HookedPlayer, MAX_CLIENTS - 1) && GameClient()->m_Snap.m_aCharacters[CurChar.m_HookedPlayer].m_Active)
					HookPos = GameClient()->m_aClients[CurChar.m_HookedPlayer].m_RenderPos;
				else
					HookPos = mix(vec2(PrevChar.m_HookX, PrevChar.m_HookY), vec2(CurChar.m_HookX, CurChar.m_HookY), Client()->IntraGameTick(g_Config.m_ClDummy));
				vec2 HookDot = WorldToMinimap(HookPos);
				HookDot.x = std::clamp(HookDot.x, X + 1.0f, X + MinimapW - 1.0f);
				HookDot.y = std::clamp(HookDot.y, Y + 1.0f, Y + MinimapH - 1.0f);
				const vec2 HookDir = HookDot - Dot;
				const float Len = length(HookDir);
				const vec2 Perp = Len > 0.001f ? vec2(-HookDir.y / Len, HookDir.x / Len) * 0.45f : vec2(0.0f, 0.0f);
				m_vHookLines.emplace_back(Dot.x, Dot.y, HookDot.x, HookDot.y);
				m_vHookLines.emplace_back(Dot.x + Perp.x, Dot.y + Perp.y, HookDot.x + Perp.x, HookDot.y + Perp.y);
				m_vHookLines.emplace_back(Dot.x - Perp.x, Dot.y - Perp.y, HookDot.x - Perp.x, HookDot.y - Perp.y);
			}
			if(CurChar.m_Weapon == WEAPON_LASER)
			{
				const float Angle = GameClient()->m_Players.GetPlayerTargetAngle(&PrevChar, &CurChar, i, Client()->IntraGameTick(g_Config.m_ClDummy));
				vec2 LaserDot = WorldToMinimap(Pos + direction(Angle) * 800.0f);
				LaserDot.x = std::clamp(LaserDot.x, X, X + MinimapW);
				LaserDot.y = std::clamp(LaserDot.y, Y, Y + MinimapH);
				m_vLaserLines.emplace_back(Dot.x, Dot.y, LaserDot.x, LaserDot.y);
			}

			CTeeRenderInfo TeeInfo = GameClient()->m_aClients[i].m_RenderInfo;
			float TeeSize = 32.0f * Scale * ((float)g_Config.m_BclMinimapTeeSize / 4.0f);
			if(g_Config.m_BclMinimapTeeSizeFlexible)
				TeeSize *= std::max(ConfiguredSmallZoom / 5.0f, 0.2f);
			TeeInfo.m_Size = std::max(TeeSize, 0.5f);
			TeeInfo.m_TeeRenderFlags |= TEE_NO_WEAPON;
			const float IconAlpha = i == LocalId ? 1.0f : (SameTeam ? 0.95f : 0.72f);

			bool ShowIcon = true;
			if(i != LocalId && SameTeam)
			{
				if(!Frozen)
					ShowIcon = g_Config.m_BclMinimapShowAlive != 0;
				else if(LastManStanding)
					ShowIcon = g_Config.m_BclMinimapShowCritical != 0;
				else
					ShowIcon = g_Config.m_BclMinimapShowFrozen != 0;
				const ColorRGBA Tint = !Frozen ? TintAlive : LastManStanding ? TintCritical : TintFrozen;
				TeeInfo.m_CustomColoredSkin = true;
				TeeInfo.m_ColorBody = Tint.WithAlpha(1.0f);
				TeeInfo.m_ColorFeet = Tint.WithAlpha(1.0f);
			}
			if(!ShowIcon)
				continue;

			if(MouseOver)
			{
				const float HoverRadius = std::max(5.0f, TeeInfo.m_Size * 0.55f);
				const float Dx = HudMouseX - Dot.x;
				const float Dy = HudMouseY - Dot.y;
				const float DistSq = Dx * Dx + Dy * Dy;
				if(DistSq <= HoverRadius * HoverRadius && DistSq < HoveredPlayerDistSq)
				{
					HoveredPlayerId = i;
					HoveredPlayerDot = Dot;
					HoveredPlayerDistSq = DistSq;
				}
			}

			const float Angle = GameClient()->m_Players.GetPlayerTargetAngle(&PrevChar, &CurChar, i, Client()->IntraGameTick(g_Config.m_ClDummy));
			RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, Frozen ? EMOTE_PAIN : EMOTE_NORMAL, direction(Angle), Dot, IconAlpha * Alpha);
		}

		if(!m_vLaserLines.empty() || !m_vHookLines.empty())
		{
			Graphics()->TextureClear();
			Graphics()->LinesBegin();
			if(!m_vLaserLines.empty())
			{
				Graphics()->SetColor(ColorRGBA(0.45f, 0.78f, 1.0f, 0.42f * Alpha));
				Graphics()->LinesDraw(m_vLaserLines.data(), m_vLaserLines.size());
			}
			if(!m_vHookLines.empty())
			{
				Graphics()->SetColor(ColorRGBA(1.0f, 1.0f, 1.0f, 0.72f * Alpha));
				Graphics()->LinesDraw(m_vHookLines.data(), m_vHookLines.size());
			}
			Graphics()->LinesEnd();
		}
	}
	m_HoveredPlayerId = HoveredPlayerId;

	Graphics()->ClipDisable();

	if(LargeMinimap && m_ForceFullSize)
		RenderFullMapChrome(X, Y, MinimapW, MinimapH, !UiBlocksDrag);
	else
	{
		m_MenuPrevPressed = false;
		m_ZoomSliderDrag = false;
	}

	if(HoveredPlayerId >= 0 && HoveredPlayerId < MAX_CLIENTS)
	{
		const SActivityPlayer &Stat = m_aActivity[HoveredPlayerId];
		const char *pName = GameClient()->m_aClients[HoveredPlayerId].m_aName;
		char aJumps[64];
		char aHits[64];
		char aSaves[64];
		char aFreezes[64];
		str_format(aJumps, sizeof(aJumps), "%s: %d", BcLocalize("Jumps"), Stat.m_Jumps);
		str_format(aHits, sizeof(aHits), "%s: %d", BcLocalize("Hits"), Stat.m_Hits);
		str_format(aSaves, sizeof(aSaves), "%s: %d", BcLocalize("Saves"), Stat.m_Saves);
		str_format(aFreezes, sizeof(aFreezes), "%s: %d", BcLocalize("Freezes"), Stat.m_Freezes);
		const char *apLines[] = {pName && pName[0] ? pName : nullptr, aJumps, aHits, aSaves, aFreezes};
		const float FontSize = 6.0f;
		const float Pad = 3.5f;
		const float LineH = FontSize + 1.0f;
		float TipW = 0.0f;
		int Lines = 0;
		for(const char *pLine : apLines)
		{
			if(!pLine)
				continue;
			TipW = std::max(TipW, TextRender()->TextWidth(FontSize, pLine));
			Lines++;
		}
		TipW += Pad * 2.0f;
		const float TipH = Lines * LineH + Pad * 2.0f - 1.0f;
		float TipX = HoveredPlayerDot.x + 7.0f;
		float TipY = HoveredPlayerDot.y - TipH - 5.0f;
		if(TipX + TipW > m_Width)
			TipX = HoveredPlayerDot.x - TipW - 7.0f;
		if(TipY < 2.0f)
			TipY = HoveredPlayerDot.y + 7.0f;
		TipX = std::clamp(TipX, 2.0f, std::max(2.0f, m_Width - TipW - 2.0f));
		TipY = std::clamp(TipY, 2.0f, std::max(2.0f, m_Height - TipH - 2.0f));
		Graphics()->DrawRect(TipX, TipY, TipW, TipH, ColorRGBA(0.0f, 0.0f, 0.0f, 0.82f), IGraphics::CORNER_ALL, 3.0f);
		TextRender()->TextColor(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
		float LineY = TipY + Pad;
		for(const char *pLine : apLines)
		{
			if(!pLine)
				continue;
			TextRender()->Text(TipX + Pad, LineY, FontSize, pLine, -1.0f);
			LineY += LineH;
		}
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	}
	else if(m_MarkerHoverValid && m_MarkerHoverKind >= 0 && m_MarkerHoverKind < 4)
	{
		static const char *const s_apMarkerNames[] = {"Jumps", "Hits", "Saves", "Freezes"};
		char aLabel[96];
		const char *pKind = BcLocalize(s_apMarkerNames[m_MarkerHoverKind]);
		const char *pNick = m_MarkerHoverClient >= 0 && m_MarkerHoverClient < MAX_CLIENTS ? GameClient()->m_aClients[m_MarkerHoverClient].m_aName : nullptr;
		if(pNick && pNick[0])
			str_format(aLabel, sizeof(aLabel), "%s x%d  %s", pNick, m_MarkerHoverCount, pKind);
		else
			str_format(aLabel, sizeof(aLabel), "x%d  %s", m_MarkerHoverCount, pKind);
		const float FontSize = 6.0f;
		const float Pad = 3.5f;
		const float TipW = TextRender()->TextWidth(FontSize, aLabel) + Pad * 2.0f;
		const float TipH = FontSize + Pad * 2.0f;
		float TipX = m_MarkerHoverPos.x + 7.0f;
		float TipY = m_MarkerHoverPos.y - TipH - 5.0f;
		if(TipX + TipW > m_Width)
			TipX = m_MarkerHoverPos.x - TipW - 7.0f;
		if(TipY < 2.0f)
			TipY = m_MarkerHoverPos.y + 7.0f;
		TipX = std::clamp(TipX, 2.0f, std::max(2.0f, m_Width - TipW - 2.0f));
		TipY = std::clamp(TipY, 2.0f, std::max(2.0f, m_Height - TipH - 2.0f));
		Graphics()->DrawRect(TipX, TipY, TipW, TipH, ColorRGBA(0.0f, 0.0f, 0.0f, 0.82f), IGraphics::CORNER_ALL, 3.0f);
		TextRender()->TextColor(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
		TextRender()->Text(TipX + Pad, TipY + Pad, FontSize, aLabel, -1.0f);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	}

	float MapProgress = 0.0f;
	if(ConsumesProgress() && GameClient()->m_FinishPrediction.TryGetProgress(MapProgress))
	{
		char aProgressLine[32];
		str_format(aProgressLine, sizeof(aProgressLine), "%.1f%%", MapProgress * 100.0f);
		const float FontSize = std::clamp(Size * 0.065f, 4.0f, 16.0f);
		const float TextW = TextRender()->TextWidth(FontSize, aProgressLine);
		const float TextX = X + (MinimapW - TextW) * 0.5f;
		const float TextY = Y + (MinimapH - FontSize) * 0.92f;
		const ColorRGBA PreviousTextColor = TextRender()->GetTextColor();
		const ColorRGBA PreviousOutlineColor = TextRender()->GetTextOutlineColor();
		ColorRGBA TextColor = TextRender()->DefaultTextColor();
		ColorRGBA OutlineColor = TextRender()->DefaultTextOutlineColor();
		TextColor.a *= Alpha;
		OutlineColor.a *= Alpha;
		TextRender()->TextColor(TextColor);
		TextRender()->TextOutlineColor(OutlineColor);
		TextRender()->Text(TextX, TextY, FontSize, aProgressLine, -1.0f);
		TextRender()->TextColor(PreviousTextColor);
		TextRender()->TextOutlineColor(PreviousOutlineColor);
	}
	if(m_MapCursorActive && m_ForceFullSize)
		RenderTools()->RenderCursor(m_MapCursor, 12.0f);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CMinimap::OnRender()
{
	if(!(m_LargeActive && m_ForceFullSize))
	{
		if(m_FollowClientId >= 0 || m_SpecRestoreArmed)
			RestoreSpecLock();
		m_FollowClientId = -1;
		m_FollowArm = -1;
	}
	HudLayout::SetEnabled(HudLayout::MODULE_MINIMAP, g_Config.m_BclMinimap != 0 && g_Config.m_BclMinimapHud != 0);
	if(g_Config.m_BclMinimap == 0)
	{
		if(m_LargeActive)
			CloseLarge();
		ReleaseIdle();
		return;
	}
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
	if(GameClient()->m_HudEditor.IsActive())
		return;
	if(!(m_LargeActive && m_ForceFullSize))
		RenderWorldMarkers();
	if(m_LargeActive)
		return;
	RenderHud(false);
}

void CMinimap::RenderFullOverlay()
{
	if(!m_LargeActive)
		return;
	if(GameClient()->m_HudEditor.IsActive())
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
	const CScreenRect PreviousScreen = Graphics()->GetScreen();
	RenderHud(false);
	Graphics()->MapScreen(PreviousScreen);
}

CUIRect CMinimap::GetHudEditorRect() const
{
	const float HudH = HudLayout::CANVAS_HEIGHT;
	const float HudW = HudH * Graphics()->ScreenAspect();
	float X = 0.0f;
	float Y = 0.0f;
	float Size = 0.0f;
	SmallMapRect(HudW, HudH, X, Y, Size);
	return {X, Y, Size, Size};
}

void CMinimap::RenderEditorPreview()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
	RenderHud(true);
}
