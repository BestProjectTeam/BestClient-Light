/* Copyright © 2026 BestProject Team */
#include "ego_legit_finished_maps.h"

#include <base/str.h>
#include <base/time.h>

#include <engine/client.h>
#include <engine/http.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/shared/json.h>

namespace
{
enum class ELegitMode
{
	NONE,
	GORES,
	DDRACE,
};

bool IsEgoServer(const CServerInfo *pServer)
{
	return pServer &&
		(str_comp_nocase(pServer->m_aCommunityId, "ego") == 0 ||
			str_find_nocase(pServer->m_aGameType, "e-gores") != nullptr ||
			str_find_nocase(pServer->m_aGameType, "e_gores") != nullptr ||
			str_find_nocase(pServer->m_aName, "EGO |") != nullptr ||
			str_find_nocase(pServer->m_aName, "eternal-gores") != nullptr ||
			str_find_nocase(pServer->m_aName, "eternal gores") != nullptr);
}

ELegitMode LegitMode(const CServerInfo *pServer)
{
	if(!pServer || (str_comp_nocase(pServer->m_aCommunityId, "legit") != 0 && str_find_nocase(pServer->m_aName, "Legit Network") == nullptr))
		return ELegitMode::NONE;

	if(str_find_nocase(pServer->m_aGameType, "Gores") != nullptr || str_find_nocase(pServer->m_aName, "Gores") != nullptr)
		return ELegitMode::GORES;
	if(str_find_nocase(pServer->m_aGameType, "DDraceNetwork") != nullptr)
		return ELegitMode::DDRACE;
	return ELegitMode::NONE;
}

bool ParseMapArray(const json_value *pRoot, const char *pKey, std::unordered_set<std::string> *pMaps)
{
	const json_value *pArray = json_object_get(pRoot, pKey);
	if(pArray == &json_value_none || pArray->type != json_array)
		return false;

	for(unsigned i = 0; i < pArray->u.array.length; ++i)
	{
		const json_value *pMap = pArray->u.array.values[i];
		if(pMap && pMap->type == json_string && pMap->u.string.ptr[0] != '\0')
			pMaps->emplace(pMap->u.string.ptr);
	}
	return true;
}

bool ParseEgoMaps(const json_value *pRoot, std::unordered_set<std::string> *pFinishedMaps)
{
	if(!pRoot || pRoot == &json_value_none || pRoot->type != json_object || !pFinishedMaps)
		return false;

	const json_value *pRecords = json_object_get(pRoot, "records");
	if(pRecords == &json_value_none || pRecords->type != json_array)
		return false;

	for(unsigned RecordIndex = 0; RecordIndex < pRecords->u.array.length; ++RecordIndex)
	{
		const json_value *pRecord = pRecords->u.array.values[RecordIndex];
		if(!pRecord || pRecord->type != json_object)
			continue;

		const json_value *pMapName = json_object_get(pRecord, "raw_map_name");
		if(pMapName == &json_value_none)
			pMapName = json_object_get(pRecord, "map_name");
		if(pMapName && pMapName != &json_value_none && pMapName->type == json_string && pMapName->u.string.ptr[0] != '\0')
			pFinishedMaps->emplace(pMapName->u.string.ptr);
	}

	return true;
}

bool ParseLegitMaps(const json_value *pRoot, std::unordered_set<std::string> *pGoresMaps, std::unordered_set<std::string> *pDDraceMaps)
{
	if(!pRoot || pRoot == &json_value_none || pRoot->type != json_object)
		return false;
	return ParseMapArray(pRoot, "gores", pGoresMaps) && ParseMapArray(pRoot, "ddrace", pDDraceMaps);
}
}

bool CEgoLegitFinishedMaps::Enabled() const
{
	return g_Config.m_BclShowFinishedMapOnEgoLegit != 0;
}

bool CEgoLegitFinishedMaps::IsFinishedMap(const CServerInfo *pServer) const
{
	if(!Enabled() || !pServer)
		return false;

	if(IsEgoServer(pServer) && m_EgoMaps.contains(pServer->m_aMap))
		return true;

	switch(LegitMode(pServer))
	{
	case ELegitMode::GORES:
		return m_GoresMaps.contains(pServer->m_aMap);
	case ELegitMode::DDRACE:
		return m_DDraceMaps.contains(pServer->m_aMap);
	case ELegitMode::NONE:
		return false;
	}
	return false;
}

void CEgoLegitFinishedMaps::ResetSource(SSource *pSource)
{
	if(pSource->m_pTask)
		pSource->m_pTask->Abort();
	pSource->m_pTask.reset();
}

void CEgoLegitFinishedMaps::ClearMaps()
{
	m_EgoMaps.clear();
	m_GoresMaps.clear();
	m_DDraceMaps.clear();
}

void CEgoLegitFinishedMaps::OnReset()
{
	ResetSource(&m_Ego);
	ResetSource(&m_Legit);
}

bool CEgoLegitFinishedMaps::ShouldStart(const SSource &Source) const
{
	return m_aPlayerName[0] != '\0' && !Source.m_pTask && !Source.m_Loaded && time_get() >= Source.m_NextRetry;
}

void CEgoLegitFinishedMaps::StartEgo()
{
	char aEscapedPlayerName[256];
	EscapeUrl(aEscapedPlayerName, sizeof(aEscapedPlayerName), m_aPlayerName);

	char aUrl[512];
	str_format(aUrl, sizeof(aUrl), "https://eternal-gores.com/api/profiles/by-nick/%s", aEscapedPlayerName);

	m_Ego.m_pTask = HttpGet(aUrl);
	m_Ego.m_pTask->Timeout(CTimeout{8000, 0, 500, 5});
	m_Ego.m_pTask->LogProgress(HTTPLOG::ALL);
	m_Ego.m_pTask->FailOnErrorStatus(false);
	Http()->Run(m_Ego.m_pTask);
}

void CEgoLegitFinishedMaps::StartLegit()
{
	char aEscapedPlayerName[256];
	EscapeUrl(aEscapedPlayerName, sizeof(aEscapedPlayerName), m_aPlayerName);

	char aUrl[512];
	str_format(aUrl, sizeof(aUrl), "https://legit.tw/api_finished_maps.php?name=%s", aEscapedPlayerName);

	m_Legit.m_pTask = HttpGet(aUrl);
	m_Legit.m_pTask->Timeout(CTimeout{8000, 0, 500, 5});
	m_Legit.m_pTask->LogProgress(HTTPLOG::FAILURE);
	m_Legit.m_pTask->FailOnErrorStatus(false);
	Http()->Run(m_Legit.m_pTask);
}

void CEgoLegitFinishedMaps::ProcessEgo()
{
	if(!m_Ego.m_pTask || !m_Ego.m_pTask->Done())
		return;

	bool Success = false;
	if(m_Ego.m_pTask->State() == EHttpState::DONE && m_Ego.m_pTask->StatusCode() == 200)
	{
		json_value *pRoot = m_Ego.m_pTask->ResultJson();
		std::unordered_set<std::string> FinishedMaps;
		Success = ParseEgoMaps(pRoot, &FinishedMaps);
		if(Success)
		{
			m_EgoMaps = std::move(FinishedMaps);
			m_Ego.m_Loaded = true;
		}
		if(pRoot)
			json_value_free(pRoot);
	}

	if(!Success)
		m_Ego.m_NextRetry = time_get() + RETRY_DELAY_SECONDS * time_freq();
	m_Ego.m_pTask.reset();
}

void CEgoLegitFinishedMaps::ProcessLegit()
{
	if(!m_Legit.m_pTask || !m_Legit.m_pTask->Done())
		return;

	bool Success = false;
	if(m_Legit.m_pTask->State() == EHttpState::DONE && m_Legit.m_pTask->StatusCode() == 200)
	{
		json_value *pRoot = m_Legit.m_pTask->ResultJson();
		std::unordered_set<std::string> GoresMaps;
		std::unordered_set<std::string> DDraceMaps;
		Success = ParseLegitMaps(pRoot, &GoresMaps, &DDraceMaps);
		if(Success)
		{
			m_GoresMaps = std::move(GoresMaps);
			m_DDraceMaps = std::move(DDraceMaps);
			m_Legit.m_Loaded = true;
		}
		if(pRoot)
			json_value_free(pRoot);
	}

	if(!Success)
		m_Legit.m_NextRetry = time_get() + RETRY_DELAY_SECONDS * time_freq();
	m_Legit.m_pTask.reset();
}

void CEgoLegitFinishedMaps::OnUpdate()
{
	if(!Enabled())
	{
		ResetSource(&m_Ego);
		ResetSource(&m_Legit);
		m_Ego.m_NextRetry = 0;
		m_Legit.m_NextRetry = 0;
		m_Ego.m_Loaded = false;
		m_Legit.m_Loaded = false;
		ClearMaps();
		m_aPlayerName[0] = '\0';
		return;
	}

	const char *pPlayerName = Client()->PlayerName();
	if(str_comp(m_aPlayerName, pPlayerName) != 0)
	{
		ResetSource(&m_Ego);
		ResetSource(&m_Legit);
		ClearMaps();
		m_Ego.m_Loaded = false;
		m_Legit.m_Loaded = false;
		m_Ego.m_NextRetry = 0;
		m_Legit.m_NextRetry = 0;
		str_copy(m_aPlayerName, pPlayerName, sizeof(m_aPlayerName));
	}

	ProcessEgo();
	ProcessLegit();

	if(ShouldStart(m_Ego))
		StartEgo();
	if(ShouldStart(m_Legit))
		StartLegit();
}
