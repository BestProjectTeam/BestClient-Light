/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_EGO_LEGIT_FINISHED_MAPS_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_EGO_LEGIT_FINISHED_MAPS_H

#include <game/client/component.h>

#include <memory>
#include <string>
#include <unordered_set>

class IHttpRequest;
class CServerInfo;

class CEgoLegitFinishedMaps : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnUpdate() override;

	bool IsFinishedMap(const CServerInfo *pServer) const;

private:
	enum
	{
		PLAYER_NAME_LENGTH = 64,
		RETRY_DELAY_SECONDS = 30,
	};

	struct SSource
	{
		std::shared_ptr<IHttpRequest> m_pTask;
		int64_t m_NextRetry = 0;
		bool m_Loaded = false;
	};

	std::unordered_set<std::string> m_EgoMaps;
	std::unordered_set<std::string> m_GoresMaps;
	std::unordered_set<std::string> m_DDraceMaps;
	SSource m_Ego;
	SSource m_Legit;
	char m_aPlayerName[PLAYER_NAME_LENGTH] = "";

	bool Enabled() const;
	void ResetSource(SSource *pSource);
	void ClearMaps();
	void StartEgo();
	void StartLegit();
	void ProcessEgo();
	void ProcessLegit();
	bool ShouldStart(const SSource &Source) const;
};

#endif
