/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_MINIMAP_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_MINIMAP_H

#include <engine/graphics.h>
#include <engine/shared/protocol.h>

#include <game/client/component.h>
#include <game/client/ui_rect.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

class CCollision;

class CMinimap : public CComponent
{
	float m_Width = 0.0f;
	float m_Height = 0.0f;

	const CCollision *m_KindsCollision = nullptr;
	int m_KindsWidth = 0;
	int m_KindsHeight = 0;
	std::vector<uint8_t> m_vKinds;
	std::vector<uint16_t> m_vVisual;
	std::vector<uint16_t> m_vFront;
	std::vector<uint16_t> m_vSwitch;
	std::vector<uint8_t> m_vPlatform;
	std::vector<IGraphics::CQuadItem> m_vTileQuads;
	std::vector<IGraphics::CLineItem> m_vHookLines;
	std::vector<IGraphics::CLineItem> m_vLaserLines;
	bool m_HasSwitchTiles = false;

	bool m_LargeActive = false;
	bool m_FullBindWasPressed = false;
	bool m_ForceFullSize = false;
	vec2 m_PanOffset = vec2(0.0f, 0.0f);
	vec2 m_PrevMouse = vec2(0.0f, 0.0f);
	bool m_Dragging = false;
	int m_FollowClientId = -1;
	int m_FollowArm = -1;
	int m_HoveredPlayerId = -1;
	vec2 m_DragStart = vec2(0.0f, 0.0f);
	bool m_MenuPrevPressed = false;
	int m_ZoomOverride = -1;

	struct SActivityPlayer
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
		int m_Team = 0;
	};

	struct SActivityMarker
	{
		int m_Tile = -1;
		int m_Team = 0;
		int m_ClientId = -1;
		uint8_t m_Kind = 0;
		int m_Count = 0;
	};

	std::vector<SActivityMarker> m_vActivity;
	std::unordered_map<uint64_t, int> m_ActivityIndex;
	std::vector<IGraphics::CFreeformItem> m_vMarkerQuads;
	SActivityPlayer m_aActivity[MAX_CLIENTS]{};
	bool m_ActivityOn = false;
	bool m_MenuExpanded = true;
	bool m_ZoomSliderDrag = false;
	float m_ZoomSliderGrab = 0.0f;
	int m_SpecRestoreId = -1;
	bool m_SpecRestoreArmed = false;
	bool m_CursorOverChrome = false;
	bool m_MapCursorActive = false;
	vec2 m_MapCursor = vec2(0.0f, 0.0f);
	bool m_MarkerHoverValid = false;
	int m_MarkerHoverIndex = -1;
	int m_MarkerHoverKind = 0;
	int m_MarkerHoverClient = -1;
	int m_MarkerHoverCount = 0;
	float m_MarkerHoverDist = 0.0f;
	vec2 m_MarkerHoverPos = vec2(0.0f, 0.0f);

	void UpdateScreen();
	void CloseLarge();
	void RestoreSpecLock();
	void SmallMapRect(float HudW, float HudH, float &X, float &Y, float &Size) const;
	void AdjustLargeZoom(float Steps);
	void SetLargeZoom(int Zoom);
	int ClampLargeZoomOut(int Zoom) const;
	void ResetLargeZoom();
	void RemoveActivity(int Index);
	void ClearMarkers();
	void RebuildActivityIndex();
	bool IsLargeMapBind(int Key) const;
	bool EnsureCache(CCollision *pCollision);
	void DrawEntityQuads(const std::vector<uint16_t> &vCells, IGraphics::CTextureHandle Texture, int StartX, int StartY, int EndX, int EndY, float X, float Y, float W, float H, vec2 WorldMin, float Scale, float Alpha);
	void DrawTiles(float X, float Y, float W, float H, vec2 WorldMin, vec2 WorldMax, float Scale, float Alpha, bool Platforms);
	void RenderFullMapChrome(float MapX, float MapY, float MapW, float MapH, bool AllowClick);
	bool FullMapChromeHit(float HudX, float HudY, float MapX, float MapY, float MapW, float MapH);
	bool HudCursor(float &X, float &Y) const;
	void ClearActivity();
	void ReleaseIdle();
	void NoteActivity(int Tile, int ClientId, int Team, int Kind, int Amount = 1);
	void SampleActivity();
	void DrawActivity(float X, float Y, float W, float H, vec2 WorldMin, float Scale, float Alpha, int LocalId, float CursorX, float CursorY, bool CursorValid);
	void RenderWorldMarkers();
	void RenderHud(bool EditorPreview);

public:
	int Sizeof() const override { return sizeof(*this); }

	void OnConsoleInit() override;
	void OnReset() override;
	void OnMapLoad() override;
	void OnRelease() override;
	void OnNewSnapshot() override;
	void OnRender() override;
	bool OnInput(const IInput::CEvent &Event) override;
	bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;

	bool BlocksGameInput() const { return m_LargeActive && m_ForceFullSize; }
	bool BlocksFollowInput() const { return BlocksGameInput() && m_FollowClientId >= 0; }
	void ReleaseFollowInput();
	void RenderFullOverlay();
	CUIRect GetHudEditorRect() const;
	void RenderEditorPreview();
	bool ConsumesProgress() const;
};

#endif
