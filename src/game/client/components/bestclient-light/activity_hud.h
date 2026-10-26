/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_ACTIVITY_HUD_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_ACTIVITY_HUD_H

#include <engine/shared/protocol.h>

#include <game/client/component.h>
#include <game/client/ui_rect.h>

class CActivityHud : public CComponent
{
	struct SPlayer
	{
		int m_Jumps = 0;
		int m_Hits = 0;
		int m_Saves = 0;
		int m_Freezes = 0;
		int m_LastInteractor = -1;
		int m_PrevAttackTick = -1;
		int m_PrevJumped = 0;
		bool m_WasActive = false;
		bool m_WasFrozen = false;
	};

	struct SLine
	{
		int m_Id = -1;
		char m_aName[48] = "";
		int m_aValue[4] = {0, 0, 0, 0};
	};

	struct SMetrics
	{
		float m_FontSize;
		float m_IconSize;
		float m_LineH;
		float m_PadX;
		float m_PadY;
		float m_Gap;
		float m_NameColW;
		int m_aColumn[4];
		int m_NumColumns;
		float m_aStatColW[4];
		float m_ScoreColW;
		float m_ScoreSize;
		float m_Width;
		float m_Height;
	};

	SPlayer m_aPlayers[MAX_CLIENTS]{};
	bool m_Tracking = false;

	void Sample();
	int BuildLines(SLine *pLines, int Capacity) const;
	int BuildEditorLines(SLine *pLines, int Capacity) const;
	SMetrics Measure(const SLine *pLines, int NumLines, float Scale) const;
	CUIRect MusicRect() const;
	void MigrateLayout(float HudWidth, const CUIRect &Music) const;
	CUIRect Place(float Width, float Height) const;
	void RenderLines(const SLine *pLines, int Num);

public:
	int Sizeof() const override { return sizeof(*this); }

	void Reset();
	void SetEditorRect(const CUIRect &Rect, int Anchor);
	CUIRect GetHudEditorRect() const;
	void RenderEditorPreview();

	void OnReset() override;
	void OnMapLoad() override;
	void OnNewSnapshot() override;
	void OnRender() override;
};

#endif
