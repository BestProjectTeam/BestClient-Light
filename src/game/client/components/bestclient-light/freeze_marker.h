/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_FREEZE_MARKER_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_FREEZE_MARKER_H

#include <base/vmath.h>

#include <engine/client/enums.h>

#include <game/client/component.h>

#include <unordered_map>
#include <vector>

class CFreezeMarker : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnMapLoad() override;
	void OnUpdate() override;
	void OnNewSnapshot() override;
	void OnRender() override;

private:
	struct SMarker
	{
		int m_TileX = 0;
		int m_TileY = 0;
		int m_Count = 0;
	};

	struct SLastTouch
	{
		bool m_Valid = false;
		bool m_Frozen = false;
		bool m_HasPos = false;
		bool m_HasOlder = false;
		vec2 m_Pos{};
		vec2 m_OlderPos{};
	};

	std::vector<SMarker> m_vMarkers;
	std::unordered_map<int, int> m_MarkerByTile;
	SLastTouch m_aLastTouch[NUM_DUMMIES]{};
	bool m_HasData = false;

	void Clear();
	bool KeepLastTouch(int ClientId) const;
	bool IsFrozen(int ClientId) const;
	bool IsFreezeTile(int ClientId, int TileIndex) const;
	int FindFreezeTile(int ClientId, vec2 From, vec2 To) const;
	void SampleTee(int Slot, int ClientId);
	void NoteTouch(int TileX, int TileY, int TileIndex);
};

#endif
