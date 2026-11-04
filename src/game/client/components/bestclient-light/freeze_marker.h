/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_FREEZE_MARKER_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_FREEZE_MARKER_H

#include <base/vmath.h>

#include <engine/client/enums.h>
#include <engine/textrender.h>

#include <game/client/component.h>

#include <unordered_map>
#include <vector>

class CFreezeMarker : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnMapLoad() override;
	void OnWindowResize() override;
	void OnUpdate() override;
	void OnNewSnapshot() override;
	void OnRender() override;

private:
	struct SMarker
	{
		int m_TileX = 0;
		int m_TileY = 0;
		int m_Count = 0;
		int m_BuiltCount = -1;
		int m_BuiltScreenW = -1;
		int m_BuiltScreenH = -1;
		float m_BuiltZoom = -1.0f;
		float m_TextH = 0.0f;
		STextContainerIndex m_TextContainer;
	};

	struct SLastTouch
	{
		bool m_Valid = false;
		bool m_Frozen = false;
	};

	std::vector<SMarker> m_vMarkers;
	std::unordered_map<int, int> m_MarkerByTile;
	SLastTouch m_aLastTouch[NUM_DUMMIES]{};
	bool m_HasData = false;

	void Clear();
	void ReleaseCountText();
	void EnsureCountText(SMarker &Marker);
	bool KeepLastTouch(int ClientId) const;
	bool IsFrozen(int ClientId, int CurTick) const;
	int TileAt(vec2 Pos) const;
	void SampleTee(int Slot, int ClientId);
	void NoteTouch(int TileX, int TileY, int TileIndex);
};

#endif
