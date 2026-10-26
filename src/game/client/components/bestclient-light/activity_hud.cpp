/* Copyright © 2026 BestProject Team */
#include "activity_hud.h"

#include <base/color.h>
#include <base/str.h>

#include <engine/font_icons.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#if defined(CONF_VIDEORECORDER)
#include <engine/shared/video.h>
#endif

#include <generated/protocol.h>

#include <game/client/components/hud_layout.h>
#include <game/client/gameclient.h>

#include <algorithm>

namespace
{
constexpr int NUM_COLUMNS = 4;
constexpr int NAME_LIMIT = 14;

const char *const s_apIcons[NUM_COLUMNS] = {
	FontIcon::ARROW_UP,
	FontIcon::HAMMER,
	FontIcon::STAR,
	FontIcon::SNOWFLAKE,
};

const ColorRGBA s_aColors[NUM_COLUMNS] = {
	ColorRGBA(0.30f, 0.82f, 0.42f, 1.0f),
	ColorRGBA(0.95f, 0.76f, 0.18f, 1.0f),
	ColorRGBA(0.35f, 0.68f, 1.0f, 1.0f),
	ColorRGBA(0.96f, 0.38f, 0.12f, 1.0f),
};

int ScoreOf(int Saves, int Freezes)
{
	return Saves * 2 - Freezes;
}

void FormatScore(char *pBuf, int Size, int Score)
{
	if(Score > 0)
		str_format(pBuf, Size, "+%d", Score);
	else
		str_format(pBuf, Size, "%d", Score);
}

float IconWidth(ITextRender *pTextRender, float Size, const char *pIcon)
{
	pTextRender->SetFontPreset(EFontPreset::ICON_FONT);
	const float Width = pTextRender->TextWidth(Size, pIcon);
	pTextRender->SetFontPreset(EFontPreset::DEFAULT_FONT);
	return Width;
}
} // namespace

CActivityHud::SMetrics CActivityHud::Measure(const SLine *pLines, int NumLines, float Scale) const
{
	SMetrics Metrics;
	Metrics.m_FontSize = 6.0f * Scale;
	Metrics.m_IconSize = Metrics.m_FontSize * 0.9f;
	Metrics.m_LineH = Metrics.m_FontSize + 1.5f * Scale;
	Metrics.m_PadX = 3.0f * Scale;
	Metrics.m_PadY = 2.5f * Scale;
	Metrics.m_Gap = 4.0f * Scale;
	Metrics.m_NumColumns = 0;
	if(g_Config.m_BclActivityHudShowJumps)
		Metrics.m_aColumn[Metrics.m_NumColumns++] = 0;
	if(g_Config.m_BclActivityHudShowHits)
		Metrics.m_aColumn[Metrics.m_NumColumns++] = 1;
	if(g_Config.m_BclActivityHudShowSaves)
		Metrics.m_aColumn[Metrics.m_NumColumns++] = 2;
	if(g_Config.m_BclActivityHudShowFreezes)
		Metrics.m_aColumn[Metrics.m_NumColumns++] = 3;
	Metrics.m_ScoreSize = Metrics.m_FontSize * 1.12f;
	Metrics.m_NameColW = TextRender()->TextWidth(Metrics.m_FontSize, "name");
	for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
	{
		const int Column = Metrics.m_aColumn[Index];
		Metrics.m_aStatColW[Index] = std::max(IconWidth(TextRender(), Metrics.m_IconSize, s_apIcons[Column]), TextRender()->TextWidth(Metrics.m_FontSize, "00"));
	}
	Metrics.m_ScoreColW = std::max(TextRender()->TextWidth(Metrics.m_ScoreSize, "±"), TextRender()->TextWidth(Metrics.m_ScoreSize, "-00"));

	for(int i = 0; i < NumLines; i++)
	{
		Metrics.m_NameColW = std::max(Metrics.m_NameColW, TextRender()->TextWidth(Metrics.m_FontSize, pLines[i].m_aName));
		for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
		{
			char aBuf[16];
			str_format(aBuf, sizeof(aBuf), "%d", pLines[i].m_aValue[Metrics.m_aColumn[Index]]);
			Metrics.m_aStatColW[Index] = std::max(Metrics.m_aStatColW[Index], TextRender()->TextWidth(Metrics.m_FontSize, aBuf));
		}
		char aScore[16];
		FormatScore(aScore, sizeof(aScore), ScoreOf(pLines[i].m_aValue[2], pLines[i].m_aValue[3]));
		Metrics.m_ScoreColW = std::max(Metrics.m_ScoreColW, TextRender()->TextWidth(Metrics.m_ScoreSize, aScore));
	}

	float Width = Metrics.m_NameColW;
	for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
		Width += Metrics.m_Gap + Metrics.m_aStatColW[Index];
	Width += Metrics.m_Gap * 1.75f + Metrics.m_ScoreColW;
	Metrics.m_Width = Width + Metrics.m_PadX * 2.0f;
	Metrics.m_Height = Metrics.m_PadY * 2.0f + Metrics.m_LineH * (NumLines + 1);
	return Metrics;
}

void CActivityHud::Reset()
{
	m_Tracking = false;
	for(SPlayer &Player : m_aPlayers)
		Player = {};
}

void CActivityHud::OnReset()
{
	Reset();
}

void CActivityHud::OnMapLoad()
{
	Reset();
}

void CActivityHud::Sample()
{
	m_Tracking = true;
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
		SPlayer &Attacker = m_aPlayers[i];
		const bool NewSwing = Attacker.m_WasActive && Cur.m_Weapon == WEAPON_HAMMER && Cur.m_AttackTick != Attacker.m_PrevAttackTick;
		if(!NewSwing)
			continue;
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
			if(FrozenNow(j))
				m_aPlayers[j].m_LastInteractor = i;
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
		m_aPlayers[Hooked].m_LastInteractor = i;
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		SPlayer &Player = m_aPlayers[i];
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
		{
			Player.m_WasActive = false;
			Player.m_WasFrozen = false;
			Player.m_LastInteractor = -1;
			Player.m_PrevAttackTick = -1;
			continue;
		}

		const CNetObj_Character &Cur = GameClient()->m_aClients[i].m_Snapped;
		const bool Frozen = FrozenNow(i);
		const bool WasActive = Player.m_WasActive;

		if(WasActive && (Cur.m_Jumped & 2) && !(Player.m_PrevJumped & 2))
			Player.m_Jumps++;

		if(WasActive && !Player.m_WasFrozen && Frozen)
			Player.m_Freezes++;
		if(WasActive && Player.m_WasFrozen && !Frozen)
		{
			const int Interactor = Player.m_LastInteractor;
			if(Interactor >= 0 && Interactor < MAX_CLIENTS && Interactor != i)
				m_aPlayers[Interactor].m_Saves++;
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

void CActivityHud::OnNewSnapshot()
{
	if(g_Config.m_BclActivityHud == 0)
	{
		if(m_Tracking)
			Reset();
		return;
	}
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
	Sample();
}

int CActivityHud::BuildLines(SLine *pLines, int Capacity) const
{
	if(Capacity <= 0)
		return 0;

	int ViewId = GameClient()->m_Snap.m_LocalClientId;
	if(GameClient()->m_Snap.m_SpecInfo.m_Active)
		ViewId = GameClient()->m_Snap.m_SpecInfo.m_SpectatorId;
	if(ViewId < 0 || ViewId >= MAX_CLIENTS)
		return 0;
	const int ViewTeam = GameClient()->m_Teams.Team(ViewId);
	if(ViewTeam >= TEAM_SUPER)
		return 0;

	const bool Team0 = ViewTeam <= TEAM_FLOCK;
	const bool ShowTeam0 = Team0 && g_Config.m_BclActivityHudTeam0 != 0;
	const vec2 ViewPos = GameClient()->m_aClients[ViewId].m_RenderPos;

	int Num = 0;
	for(int i = 0; i < MAX_CLIENTS && Num < Capacity; i++)
	{
		if(Team0)
		{
			if(i != ViewId)
			{
				if(!ShowTeam0)
					continue;
				if(GameClient()->m_Teams.Team(i) > TEAM_FLOCK)
					continue;
				if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
					continue;
			}
		}
		else if(GameClient()->m_Teams.Team(i) != ViewTeam)
			continue;
		if(i != ViewId && !GameClient()->m_Snap.m_apPlayerInfos[i])
			continue;
		const char *pName = GameClient()->m_aClients[i].m_aName;
		if(!pName || pName[0] == '\0')
			continue;

		SLine &Line = pLines[Num];
		Line = {};
		Line.m_Id = i;
		str_utf8_truncate(Line.m_aName, sizeof(Line.m_aName), pName, NAME_LIMIT);
		Line.m_aValue[0] = m_aPlayers[i].m_Jumps;
		Line.m_aValue[1] = m_aPlayers[i].m_Hits;
		Line.m_aValue[2] = m_aPlayers[i].m_Saves;
		Line.m_aValue[3] = m_aPlayers[i].m_Freezes;
		Num++;
	}

	const int View = ViewId;
	if(ShowTeam0)
	{
		std::sort(pLines, pLines + Num, [&](const SLine &Left, const SLine &Right) {
			if((Left.m_Id == View) != (Right.m_Id == View))
				return Left.m_Id == View;
			const vec2 DeltaLeft = GameClient()->m_aClients[Left.m_Id].m_RenderPos - ViewPos;
			const vec2 DeltaRight = GameClient()->m_aClients[Right.m_Id].m_RenderPos - ViewPos;
			const float DistLeft = DeltaLeft.x * DeltaLeft.x + DeltaLeft.y * DeltaLeft.y;
			const float DistRight = DeltaRight.x * DeltaRight.x + DeltaRight.y * DeltaRight.y;
			if(DistLeft != DistRight)
				return DistLeft < DistRight;
			return Left.m_Id < Right.m_Id;
		});
	}
	else
	{
		std::sort(pLines, pLines + Num, [View](const SLine &Left, const SLine &Right) {
			if((Left.m_Id == View) != (Right.m_Id == View))
				return Left.m_Id == View;
			return Left.m_Id < Right.m_Id;
		});
	}
	return Num;
}

int CActivityHud::BuildEditorLines(SLine *pLines, int Capacity) const
{
	const int Num = BuildLines(pLines, Capacity);
	if(Num > 0 || Capacity <= 0)
		return Num;
	pLines[0] = {};
	str_copy(pLines[0].m_aName, "name", sizeof(pLines[0].m_aName));
	return 1;
}

CUIRect CActivityHud::MusicRect() const
{
	CUIRect Music = GameClient()->m_MusicPlayer.GetHudEditorRect(false);
	if(Music.w <= 0.0f || Music.h <= 0.0f)
		Music = GameClient()->m_MusicPlayer.GetHudEditorRect(true);
	return Music;
}

void CActivityHud::MigrateLayout(float HudWidth, const CUIRect &Music) const
{
	const auto Layout = HudLayout::Get(HudLayout::MODULE_ACTIVITY_HUD, HudWidth, HudLayout::CANVAS_HEIGHT);
	if(Layout.m_Mode >= HudLayout::POSITION_MODE_ACTIVITY_TL && Layout.m_Mode <= HudLayout::POSITION_MODE_ACTIVITY_BR)
		return;
	if(Music.w <= 0.0f || Music.h <= 0.0f)
		return;
	const float AbsX = Layout.m_X * (HudWidth / HudLayout::CANVAS_WIDTH);
	HudLayout::SetPosition(
		HudLayout::MODULE_ACTIVITY_HUD,
		AbsX - (Music.x + Music.w),
		Layout.m_Y - Music.y,
		HudLayout::POSITION_MODE_ACTIVITY_TL);
}

CUIRect CActivityHud::Place(float Width, float Height) const
{
	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	const CUIRect Music = MusicRect();
	MigrateLayout(HudWidth, Music);
	const auto Layout = HudLayout::Get(HudLayout::MODULE_ACTIVITY_HUD, HudWidth, HudHeight);
	CUIRect Rect;
	Rect.w = Width;
	Rect.h = Height;
	const bool Anchored = Layout.m_Mode >= HudLayout::POSITION_MODE_ACTIVITY_TL && Layout.m_Mode <= HudLayout::POSITION_MODE_ACTIVITY_BR;
	if(!Anchored)
	{
		Rect.x = Layout.m_X * (HudWidth / HudLayout::CANVAS_WIDTH);
		Rect.y = Layout.m_Y;
	}
	else if(Music.w <= 0.0f || Music.h <= 0.0f)
	{
		Rect.x = (HudWidth - Width) * 0.5f;
		Rect.y = 0.0f;
	}
	else
	{
		const float AnchorX = Music.x + Music.w + Layout.m_X;
		const float AnchorY = Music.y + Layout.m_Y;
		switch(Layout.m_Mode)
		{
		case HudLayout::POSITION_MODE_ACTIVITY_TR:
			Rect.x = AnchorX - Width;
			Rect.y = AnchorY;
			break;
		case HudLayout::POSITION_MODE_ACTIVITY_BL:
			Rect.x = AnchorX;
			Rect.y = AnchorY - Height;
			break;
		case HudLayout::POSITION_MODE_ACTIVITY_BR:
			Rect.x = AnchorX - Width;
			Rect.y = AnchorY - Height;
			break;
		default:
			Rect.x = AnchorX;
			Rect.y = AnchorY;
			break;
		}
	}
	Rect.x = std::clamp(Rect.x, 0.0f, std::max(0.0f, HudWidth - Rect.w));
	Rect.y = std::clamp(Rect.y, 0.0f, std::max(0.0f, HudHeight - Rect.h));
	return Rect;
}

void CActivityHud::SetEditorRect(const CUIRect &Rect, int Anchor)
{
	if(Anchor < 0 || Anchor > 3)
		Anchor = 0;
	const auto Mode = (HudLayout::EPositionMode)(HudLayout::POSITION_MODE_ACTIVITY_TL + Anchor);
	const CUIRect Music = MusicRect();
	float CornerX = Rect.x;
	float CornerY = Rect.y;
	if(Anchor == 1 || Anchor == 3)
		CornerX += Rect.w;
	if(Anchor >= 2)
		CornerY += Rect.h;
	float OffsetX = CornerX;
	float OffsetY = CornerY;
	if(Music.w > 0.0f && Music.h > 0.0f)
	{
		OffsetX = CornerX - (Music.x + Music.w);
		OffsetY = CornerY - Music.y;
	}
	HudLayout::SetPosition(HudLayout::MODULE_ACTIVITY_HUD, OffsetX, OffsetY, Mode);
}

CUIRect CActivityHud::GetHudEditorRect() const
{
	SLine aLines[MAX_CLIENTS];
	const int Num = BuildEditorLines(aLines, MAX_CLIENTS);
	if(Num <= 0)
		return {};
	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	const auto Layout = HudLayout::Get(HudLayout::MODULE_ACTIVITY_HUD, HudWidth, HudHeight);
	const float Scale = std::clamp(Layout.m_Scale / 100.0f, 0.25f, 3.0f);
	const SMetrics Metrics = Measure(aLines, Num, Scale);
	return Place(Metrics.m_Width, Metrics.m_Height);
}

void CActivityHud::RenderLines(const SLine *pLines, int Num)
{
	if(Num <= 0)
		return;

	const float HudHeight = HudLayout::CANVAS_HEIGHT;
	const float HudWidth = HudHeight * Graphics()->ScreenAspect();
	const auto Layout = HudLayout::Get(HudLayout::MODULE_ACTIVITY_HUD, HudWidth, HudHeight);
	const float Scale = std::clamp(Layout.m_Scale / 100.0f, 0.25f, 3.0f);
	const SMetrics Metrics = Measure(pLines, Num, Scale);
	const CUIRect Rect = Place(Metrics.m_Width, Metrics.m_Height);

	Graphics()->MapScreenToSize(HudWidth, HudHeight);
	const float ModuleAlpha = HudLayout::AlphaFactor(HudLayout::MODULE_ACTIVITY_HUD);
	const int Corners = HudLayout::BackgroundCorners(IGraphics::CORNER_ALL, Rect.x, Rect.y, Rect.w, Rect.h, HudWidth, HudHeight);
	const float Rounding = std::clamp(4.0f * Scale, 2.0f, 8.0f);
	if(Layout.m_BackgroundEnabled)
		Graphics()->DrawRect(Rect.x, Rect.y, Rect.w, Rect.h, color_cast<ColorRGBA>(ColorHSLA(Layout.m_BackgroundColor, true)).WithMultipliedAlpha(ModuleAlpha), Corners, Rounding);

	const ColorRGBA PreviousText = TextRender()->GetTextColor();
	const ColorRGBA PreviousOutline = TextRender()->GetTextOutlineColor();
	ColorRGBA Outline = PreviousOutline;
	Outline.a *= ModuleAlpha;
	TextRender()->TextOutlineColor(Outline);

	float StatX[NUM_COLUMNS];
	float Cursor = Rect.x + Metrics.m_PadX + Metrics.m_NameColW + Metrics.m_Gap;
	for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
	{
		StatX[Index] = Cursor;
		Cursor += Metrics.m_aStatColW[Index] + Metrics.m_Gap;
	}
	Cursor += Metrics.m_Gap * 0.75f;
	const float ScoreX = Cursor;

	auto DrawCentered = [&](float X, float ColumnW, float Y, float Size, const char *pText) {
		const float Width = TextRender()->TextWidth(Size, pText);
		TextRender()->Text(X + (ColumnW - Width) * 0.5f, Y, Size, pText, -1.0f);
	};
	auto DrawHeavy = [&](float X, float ColumnW, float Y, float Size, const char *pText) {
		const float Width = TextRender()->TextWidth(Size, pText);
		const float DrawX = X + (ColumnW - Width) * 0.5f;
		const float Offset = 0.4f * Scale;
		TextRender()->Text(DrawX + Offset, Y, Size, pText, -1.0f);
		TextRender()->Text(DrawX, Y, Size, pText, -1.0f);
	};

	float Y = Rect.y + Metrics.m_PadY;
	for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
	{
		const int Column = Metrics.m_aColumn[Index];
		ColorRGBA Color = s_aColors[Column];
		Color.a *= ModuleAlpha;
		TextRender()->TextColor(Color);
		TextRender()->SetFontPreset(EFontPreset::ICON_FONT);
		DrawCentered(StatX[Index], Metrics.m_aStatColW[Index], Y + (Metrics.m_LineH - Metrics.m_IconSize) * 0.5f, Metrics.m_IconSize, s_apIcons[Column]);
		TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
	}
	ColorRGBA ScoreColor = TextRender()->DefaultTextColor();
	ScoreColor.a *= ModuleAlpha;
	TextRender()->TextColor(ScoreColor);
	DrawHeavy(ScoreX, Metrics.m_ScoreColW, Y + (Metrics.m_LineH - Metrics.m_ScoreSize) * 0.5f, Metrics.m_ScoreSize, "±");

	ColorRGBA NameColor = TextRender()->DefaultTextColor();
	NameColor.a *= ModuleAlpha;
	for(int i = 0; i < Num; i++)
	{
		Y += Metrics.m_LineH;
		TextRender()->TextColor(NameColor);
		TextRender()->Text(Rect.x + Metrics.m_PadX, Y, Metrics.m_FontSize, pLines[i].m_aName, -1.0f);
		for(int Index = 0; Index < Metrics.m_NumColumns; Index++)
		{
			const int Column = Metrics.m_aColumn[Index];
			char aBuf[16];
			str_format(aBuf, sizeof(aBuf), "%d", pLines[i].m_aValue[Column]);
			ColorRGBA Color = s_aColors[Column];
			Color.a *= ModuleAlpha;
			TextRender()->TextColor(Color);
			DrawCentered(StatX[Index], Metrics.m_aStatColW[Index], Y, Metrics.m_FontSize, aBuf);
		}
		char aScore[16];
		FormatScore(aScore, sizeof(aScore), ScoreOf(pLines[i].m_aValue[2], pLines[i].m_aValue[3]));
		TextRender()->TextColor(ScoreColor);
		DrawHeavy(ScoreX, Metrics.m_ScoreColW, Y + (Metrics.m_FontSize - Metrics.m_ScoreSize) * 0.5f, Metrics.m_ScoreSize, aScore);
	}

	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
	TextRender()->TextColor(PreviousText);
	TextRender()->TextOutlineColor(PreviousOutline);
	Ui()->MapScreen();
}

void CActivityHud::RenderEditorPreview()
{
	if(g_Config.m_BclActivityHud == 0)
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	SLine aLines[MAX_CLIENTS];
	const int Num = BuildEditorLines(aLines, MAX_CLIENTS);
	RenderLines(aLines, Num);
}

void CActivityHud::OnRender()
{
	HudLayout::SetEnabled(HudLayout::MODULE_ACTIVITY_HUD, g_Config.m_BclActivityHud != 0);
	if(g_Config.m_BclActivityHud == 0)
	{
		if(m_Tracking)
			Reset();
		return;
	}
	if(GameClient()->m_HudEditor.IsActive())
		return;
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!GameClient()->m_Snap.m_pGameInfoObj)
		return;
#if defined(CONF_VIDEORECORDER)
	if(!((IVideo::Current() && g_Config.m_ClVideoShowhud) || (!IVideo::Current() && g_Config.m_ClShowhud)))
		return;
#else
	if(!g_Config.m_ClShowhud)
		return;
#endif
	if(g_Config.m_ClFocusMode && g_Config.m_ClFocusModeHideHud)
		return;
	if(GameClient()->m_Scoreboard.IsActive())
		return;
	SLine aLines[MAX_CLIENTS];
	const int Num = BuildLines(aLines, MAX_CLIENTS);
	RenderLines(aLines, Num);
}
