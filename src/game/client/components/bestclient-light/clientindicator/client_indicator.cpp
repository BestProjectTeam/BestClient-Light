/* Copyright © 2026 BestProject Team */
#include "client_indicator.h"

#include "../subsystem_runtime.h"
#include "../version.h"
#include "protocol.h"
#include "version_label.h"

#include <base/log.h>
#include <base/math.h>
#include <base/mem.h>
#include <base/str.h>
#include <base/time.h>

#include <engine/client.h>
#include <engine/engine.h>
#include <engine/http.h>
#include <engine/serverbrowser.h>
#include <engine/shared/bestclient-light_indicator_protocol.h>
#include <engine/shared/config.h>
#include <engine/shared/jobs.h>
#include <engine/shared/json.h>
#include <engine/shared/network.h>

#include <game/client/gameclient.h>

#include <algorithm>
#include <cstdarg>
#include <cstring>

class CBrowserCacheJob : public IJob
{
	std::shared_ptr<IHttpRequest> m_pBrowserTask;

public:
	CBrowserCache m_LoadedCache;
	bool m_Success = false;

	CBrowserCacheJob(std::shared_ptr<IHttpRequest> pBrowserTask) :
		m_pBrowserTask(std::move(pBrowserTask))
	{
		Abortable(true);
	}

	void Run() override
	{
		if(State() == STATE_ABORTED || !m_pBrowserTask)
			return;
		json_value *pJson = m_pBrowserTask->ResultJson();
		if(State() == STATE_ABORTED)
		{
			if(pJson)
				json_value_free(pJson);
			return;
		}
		if(pJson)
		{
			m_Success = m_LoadedCache.Load(*pJson);
			json_value_free(pJson);
		}
	}
};

namespace
{
	constexpr const char *LOG_SCOPE = "clientindicator-cl";
	constexpr const char *OLD_BC_BROWSER_URL = "http://150.241.70.188:8779/users.json";
	constexpr const char *OLD_BC_TOKEN_URL = "http://150.241.70.188:8779/token.json";
	constexpr const char *NEW_BC_BROWSER_URL = "https://150.241.70.188:8779/users.json";
	constexpr const char *NEW_BC_TOKEN_URL = "https://150.241.70.188:8779/token.json";
	constexpr int PACKET_DUMP_BYTES_PER_LINE = 64;

	void TrimConfigString(char *pValue, int Size)
	{
		if(!pValue || Size <= 0)
			return;

		const int Length = str_length(pValue);
		int Start = 0;
		while(Start < Length && str_isspace(pValue[Start]))
			++Start;

		int End = Length;
		while(End > Start && str_isspace(pValue[End - 1]))
			--End;

		if(Start == 0 && End == Length)
			return;

		const std::string Trimmed(pValue + Start, End - Start);
		str_copy(pValue, Trimmed.c_str(), Size);
	}

	int64_t SlowPacketProcessTicks()
	{
		return time_freq() / 500; // ~2ms
	}

	void NormalizeBestClientLightIndicatorConfig()
	{
		TrimConfigString(g_Config.m_BclClientIndicatorServerAddress, sizeof(g_Config.m_BclClientIndicatorServerAddress));
		TrimConfigString(g_Config.m_BclClientIndicatorBrowserUrl, sizeof(g_Config.m_BclClientIndicatorBrowserUrl));
		TrimConfigString(g_Config.m_BclClientIndicatorTokenUrl, sizeof(g_Config.m_BclClientIndicatorTokenUrl));
		TrimConfigString(g_Config.m_BclClientIndicatorSharedToken, sizeof(g_Config.m_BclClientIndicatorSharedToken));
		TrimConfigString(g_Config.m_BclClientIndicatorSecretKey, sizeof(g_Config.m_BclClientIndicatorSecretKey));

		if(g_Config.m_BclClientIndicatorBrowserUrl[0] == '\0' || str_comp(g_Config.m_BclClientIndicatorBrowserUrl, OLD_BC_BROWSER_URL) == 0)
			str_copy(g_Config.m_BclClientIndicatorBrowserUrl, NEW_BC_BROWSER_URL, sizeof(g_Config.m_BclClientIndicatorBrowserUrl));
		if(g_Config.m_BclClientIndicatorTokenUrl[0] == '\0' || str_comp(g_Config.m_BclClientIndicatorTokenUrl, OLD_BC_TOKEN_URL) == 0)
			str_copy(g_Config.m_BclClientIndicatorTokenUrl, NEW_BC_TOKEN_URL, sizeof(g_Config.m_BclClientIndicatorTokenUrl));
	}

	bool IsBlockedIndicatorAddress(const NETADDR &Addr)
	{
		return net_addr_is_local(&Addr);
	}

	void StripVersionProof(char *pVersion)
	{
		BestClientLightStripVersionProof(pVersion);
	}

	void CopyVersionLabel(const char *pRaw, char *pVersion, int VersionSize)
	{
		str_copy(pVersion, pRaw ? pRaw : "", VersionSize);
		StripVersionProof(pVersion);
	}

	bool HasOfficialCode()
	{
		return BESTCLIENT_LIGHT_OFFICIAL_CODE[0] != '\0';
	}

	bool BuildVersionAnnounceFields(char *pVersionOut, int VersionOutSize, char *pOfficialCodeOut, int OfficialCodeOutSize)
	{
		if(!pVersionOut || VersionOutSize <= 0 || !pOfficialCodeOut || OfficialCodeOutSize <= 0)
			return false;
		pVersionOut[0] = '\0';
		pOfficialCodeOut[0] = '\0';
		if(BESTCLIENT_LIGHT_VERSION[0] == '\0')
			return false;

		str_copy(pVersionOut, BESTCLIENT_LIGHT_VERSION, VersionOutSize);
		str_copy(pOfficialCodeOut, BESTCLIENT_LIGHT_OFFICIAL_CODE, OfficialCodeOutSize);
		return true;
	}

	bool IsValidPeerClientId(int ClientId)
	{
		return ClientId >= 0 && ClientId < MAX_CLIENTS;
	}

	const char *PacketTypeName(int PacketType)
	{
		switch(PacketType)
		{
		case BestClientLightIndicator::PACKET_JOIN:
			return "join";
		case BestClientLightIndicator::PACKET_HEARTBEAT:
			return "heartbeat";
		case BestClientLightIndicator::PACKET_LEAVE:
			return "leave";
		case BestClientLightIndicator::PACKET_PEER_STATE:
			return "peer_state";
		case BestClientLightIndicator::PACKET_PEER_REMOVE:
			return "peer_remove";
		case BestClientLightIndicator::PACKET_PEER_LIST:
			return "peer_list";
		case BestClientLightIndicator::PACKET_DEV_AUTH:
			return "dev_auth";
		case BestClientLightIndicator::PACKET_PEER_DEV_STATE:
			return "peer_dev_state";
		case BestClientLightIndicator::PACKET_PEER_DEV_LIST:
			return "peer_dev_list";
		case BestClientLightIndicator::PACKET_DEV_AUTH_RESULT:
			return "dev_auth_result";
		case BestClientLightIndicator::PACKET_VERSION_ANNOUNCE:
			return "version_announce";
		case BestClientLightIndicator::PACKET_PEER_VERSION_STATE:
			return "peer_version_state";
		case BestClientLightIndicator::PACKET_PEER_EDITION_STATE:
			return "peer_edition_state";
		default:
			return "unknown";
		}
	}

	void DumpUdpPacketBytes(const char *pDirection, const NETADDR &Addr, const void *pData, int DataSize)
	{
		if(!pDirection || !pData || DataSize <= 0)
			return;

		char aAddr[NETADDR_MAXSTRSIZE];
		net_addr_str(&Addr, aAddr, sizeof(aAddr), true);
		log_info(LOG_SCOPE, "%s udp packet bytes=%d addr=%s", pDirection, DataSize, aAddr);

		const auto *pBytes = static_cast<const uint8_t *>(pData);
		for(int Offset = 0; Offset < DataSize; Offset += PACKET_DUMP_BYTES_PER_LINE)
		{
			const int ChunkSize = std::min(PACKET_DUMP_BYTES_PER_LINE, DataSize - Offset);
			char aHex[PACKET_DUMP_BYTES_PER_LINE * 2 + 1];
			str_hex(aHex, sizeof(aHex), pBytes + Offset, ChunkSize);
			log_info(LOG_SCOPE, "%s udp dump offset=%d size=%d hex=%s", pDirection, Offset, ChunkSize, aHex);
		}
	}
}

CClientIndicator::CClientIndicator()
{
	OnReset();
}

void CClientIndicator::OnInit()
{
	NormalizeBestClientLightIndicatorConfig();
	if(m_ClientInstanceId == UUID_ZEROED)
		m_ClientInstanceId = RandomUuid();
	DebugLogF("init server=%s token_url=%s browser_url=%s", g_Config.m_BclClientIndicatorServerAddress, g_Config.m_BclClientIndicatorTokenUrl, g_Config.m_BclClientIndicatorBrowserUrl);
}

void CClientIndicator::OnReset()
{
	ResetPresenceState();
	ResetTokenState();
}

void CClientIndicator::OnStateChange(int NewState, int OldState)
{
	(void)OldState;
	if(NewState == IClient::STATE_OFFLINE)
	{
		StopPresence(true);
		ResetTokenState();
	}
	else if(NewState == IClient::STATE_ONLINE)
	{
		DebugLog("state -> online, refreshing token/browser cache");
		RefreshBrowserCache(false);
		RefreshToken(false);
	}
}

void CClientIndicator::OnShutdown()
{
	StopPresence(true);
	ResetBrowserTask();
	ResetTokenTask();
}

bool CClientIndicator::IsPlayerBestClientLight(int ClientId) const
{
	return ClientId >= 0 && ClientId < MAX_CLIENTS && m_aBestClient[ClientId];
}

bool CClientIndicator::IsPlayerLightEdition(int ClientId) const
{
	return ClientId >= 0 && ClientId < MAX_CLIENTS && m_aLightEdition[ClientId];
}

bool CClientIndicator::IsPlayerDeveloper(int ClientId) const
{
	return ClientId >= 0 && ClientId < MAX_CLIENTS && m_aDeveloper[ClientId];
}

bool CClientIndicator::IsPlayerFakeVersion(int ClientId) const
{
	return ClientId >= 0 && ClientId < MAX_CLIENTS && m_aFake[ClientId];
}

void CClientIndicator::RebuildPlayerFlags()
{
	m_aBestClient.fill(false);
	m_aLightEdition.fill(false);
	m_aDeveloper.fill(false);
	m_aFake.fill(false);
	m_FlagsDirty = false;

	if(Client()->State() != IClient::STATE_ONLINE)
		return;

	const char *pServerAddress = m_aCachedGameServerAddr[0] != '\0' ? m_aCachedGameServerAddr : nullptr;
	char aFallbackAddress[NETADDR_MAXSTRSIZE];
	if(!pServerAddress)
	{
		net_addr_str(&Client()->ServerAddress(), aFallbackAddress, sizeof(aFallbackAddress), true);
		pServerAddress = aFallbackAddress;
	}
	const IServerBrowser::CServerEntry *pCurrentServer = ServerBrowser()->Find(Client()->ServerAddress());

	for(int ClientId = 0; ClientId < MAX_CLIENTS; ++ClientId)
	{
		bool Local = false;
		for(const int LocalId : GameClient()->m_aLocalIds)
		{
			if(LocalId >= 0 && ClientId == LocalId)
			{
				Local = true;
				break;
			}
		}

		const char *pPlayerName = PlayerNameForClient(ClientId);
		bool Browser = false;
		bool BrowserDeveloper = false;
		bool BrowserFake = false;
		bool BrowserLight = false;
		if(pPlayerName[0] != '\0')
			Browser = m_BrowserCache.HasPlayer(pServerAddress, pPlayerName, &BrowserDeveloper, &BrowserFake, &BrowserLight);

		bool ServerBest = false;
		bool ServerDeveloper = false;
		bool ServerFake = false;
		bool ServerLight = false;
		if(!Local && !m_PresenceCache.IsPresent(ClientId) && !Browser && pCurrentServer && pPlayerName[0] != '\0')
		{
			for(const auto &ClientInfo : pCurrentServer->m_Info.m_vClients)
			{
				if(!ClientInfo.m_BestClientLight || str_comp(ClientInfo.m_aName, pPlayerName) != 0)
					continue;
				ServerBest = true;
				ServerDeveloper = ClientInfo.m_BestClientLightDeveloper;
				ServerFake = ClientInfo.m_BestClientLightFake;
				ServerLight = ClientInfo.m_BestClientEditionLight;
				break;
			}
		}
		else if(pCurrentServer && pPlayerName[0] != '\0' && (Browser || m_PresenceCache.IsPresent(ClientId)))
		{
			for(const auto &ClientInfo : pCurrentServer->m_Info.m_vClients)
			{
				if(str_comp(ClientInfo.m_aName, pPlayerName) != 0)
					continue;
				ServerDeveloper = ClientInfo.m_BestClientLightDeveloper;
				ServerFake = ClientInfo.m_BestClientLightFake;
				ServerLight = ClientInfo.m_BestClientEditionLight;
				break;
			}
		}

		const bool Best = Local || m_PresenceCache.IsPresent(ClientId) || Browser || ServerBest;
		const bool Developer = m_DeveloperClientIds.find(ClientId) != m_DeveloperClientIds.end() || BrowserDeveloper || ServerDeveloper;
		if(!Best && !Developer)
			continue;

		m_aBestClient[ClientId] = Best;
		m_aDeveloper[ClientId] = Developer;
		m_aLightEdition[ClientId] = Local || m_LightClientIds.find(ClientId) != m_LightClientIds.end() || BrowserLight || ServerLight;
		if(Local)
			m_aFake[ClientId] = !HasOfficialCode();
		else if(Best)
			m_aFake[ClientId] = m_FakeClientIds.find(ClientId) != m_FakeClientIds.end() || BrowserFake || ServerFake;
	}
}

bool CClientIndicator::GetPlayerVersionLabel(int ClientId, char *pVersion, int VersionSize) const
{
	if(!pVersion || VersionSize <= 0)
		return false;
	pVersion[0] = '\0';

	if(Client()->State() != IClient::STATE_ONLINE)
		return false;

	for(const int LocalId : GameClient()->m_aLocalIds)
	{
		if(LocalId >= 0 && ClientId == LocalId)
		{
			str_copy(pVersion, BESTCLIENT_LIGHT_VERSION, VersionSize);
			return true;
		}
	}

	const char *pPlayerName = PlayerNameForClient(ClientId);
	if(pPlayerName[0] == '\0')
		return false;

	char aCurrentServerAddress[NETADDR_MAXSTRSIZE];
	net_addr_str(&Client()->ServerAddress(), aCurrentServerAddress, sizeof(aCurrentServerAddress), true);
	const auto It = m_ClientVersions.find(ClientId);
	if(It != m_ClientVersions.end() && !It->second.empty())
	{
		CopyVersionLabel(It->second.c_str(), pVersion, VersionSize);
		return true;
	}
	if(m_BrowserCache.GetPlayerVersion(aCurrentServerAddress, pPlayerName, pVersion, VersionSize))
	{
		StripVersionProof(pVersion);
		return true;
	}

	if(IsPlayerBestClientLight(ClientId))
	{
		str_copy(pVersion, "under", VersionSize);
		return true;
	}

	return false;
}

void CClientIndicator::RefreshBrowserCache(bool Force)
{
#if defined(CONF_HEADLESS_CLIENT)
	(void)Force;
	return;
#endif
	NormalizeBestClientLightIndicatorConfig();
	if(g_Config.m_BclClientIndicatorBrowserUrl[0] == '\0')
	{
		DebugLog("browser refresh skipped: browser url is empty");
		return;
	}
	if((m_pBrowserTask && !m_pBrowserTask->Done()) || (m_pBrowserJob && !m_pBrowserJob->Done()))
	{
		if(!Force)
		{
			DebugLog("browser refresh skipped: request already running");
			return;
		}
		DebugLog("browser refresh forcing reset of running request");
		ResetBrowserTask();
	}

	m_pBrowserTask = HttpGet(g_Config.m_BclClientIndicatorBrowserUrl);
	m_pBrowserTask->Timeout(CTimeout{10000, 0, 500, 5});
	m_pBrowserTask->IpResolve(IPRESOLVE::V4);
	m_pBrowserTask->VerifyPeer(false);
	m_pBrowserTask->FailOnErrorStatus(false);
	m_pBrowserTask->LogProgress(HTTPLOG::FAILURE);
	DebugLogF("starting browser request url=%s", g_Config.m_BclClientIndicatorBrowserUrl);
	m_LastBrowserRefreshTick = time_get();
	Http()->Run(m_pBrowserTask);
}

void CClientIndicator::RefreshToken(bool Force)
{
#if defined(CONF_HEADLESS_CLIENT)
	(void)Force;
	return;
#endif
	NormalizeBestClientLightIndicatorConfig();
	if(g_Config.m_BclClientIndicatorTokenUrl[0] == '\0')
	{
		DebugLog("token refresh skipped: token url is empty");
		return;
	}
	if(m_pTokenTask && !m_pTokenTask->Done())
	{
		if(!Force)
		{
			DebugLog("token refresh skipped: request already running");
			return;
		}
		DebugLog("token refresh forcing reset of running request");
		ResetTokenTask();
	}

	m_pTokenTask = HttpGet(g_Config.m_BclClientIndicatorTokenUrl);
	m_pTokenTask->Timeout(CTimeout{10000, 0, 500, 5});
	m_pTokenTask->IpResolve(IPRESOLVE::V4);
	m_pTokenTask->VerifyPeer(false);
	m_pTokenTask->FailOnErrorStatus(false);
	m_pTokenTask->LogProgress(HTTPLOG::FAILURE);
	DebugLogF("starting token request url=%s", g_Config.m_BclClientIndicatorTokenUrl);
	m_LastTokenRefreshTick = time_get();
	Http()->Run(m_pTokenTask);
}

void CClientIndicator::OnUpdate()
{
	const bool PresenceEnabled = IsPresenceEnabled();
	const int64_t Now = time_get();
	const int64_t PerfStart = g_Config.m_DbgClientIndicator >= 2 ? Now : 0;
	const int64_t TokenRefreshInterval = 5 * 60 * time_freq();

	if(!m_pBrowserTask && !m_pBrowserJob && m_LastBrowserRefreshTick == 0)
		RefreshBrowserCache(false);

	if(PresenceEnabled && !m_WasPresenceEnabled)
	{
		RefreshToken(false);
	}
	else if(PresenceEnabled)
	{
		if(!m_pTokenTask && (m_LastTokenRefreshTick == 0 || Now - m_LastTokenRefreshTick >= TokenRefreshInterval))
			RefreshToken(false);
	}
	else
	{
		if(m_WasPresenceEnabled || m_Socket || m_HasServerAddr || m_pTokenTask || m_aWebSharedToken[0] != '\0')
		{
			StopPresence(true);
			ResetTokenState();
		}
		SetPresenceBlockReason("presence update skipped: client offline");
		m_WasPresenceEnabled = false;
		m_aCachedGameServerAddr[0] = '\0';
	}

	if(m_pBrowserJob && m_pBrowserJob->Done())
	{
		if(m_pBrowserJob->State() == IJob::STATE_DONE && m_pBrowserJob->m_Success)
		{
			m_BrowserCache = std::move(m_pBrowserJob->m_LoadedCache);
			DebugLogF("browser cache loaded %llu entries", (unsigned long long)m_BrowserCache.Players().size());
			ApplyBrowserSnapshot();
		}
		else
		{
			DebugLog("browser cache background job failed or was aborted");
		}
		ResetBrowserTask();
	}
	else if(!m_pBrowserJob && m_pBrowserTask && m_pBrowserTask->State() == EHttpState::DONE)
	{
		DebugLogF("browser request done http_status=%d, dispatching background parse job", m_pBrowserTask->StatusCode());
		m_pBrowserJob = std::make_shared<CBrowserCacheJob>(m_pBrowserTask);
		Engine()->AddJob(m_pBrowserJob);
	}
	else if(!m_pBrowserJob && m_pBrowserTask && (m_pBrowserTask->State() == EHttpState::ERROR || m_pBrowserTask->State() == EHttpState::ABORTED))
	{
		DebugLogF("browser request ended with state=%d", (int)m_pBrowserTask->State());
		ResetBrowserTask();
	}

	if(m_pTokenTask && m_pTokenTask->State() == EHttpState::DONE)
	{
		DebugLogF("token request done http_status=%d", m_pTokenTask->StatusCode());
		FinishTokenRefresh();
		ResetTokenTask();
	}
	else if(m_pTokenTask && m_pTokenTask->State() == EHttpState::ERROR)
	{
		DebugLog("token request ended with error");
		char aOldEffectiveToken[sizeof(m_aWebSharedToken)];
		str_copy(aOldEffectiveToken, EffectiveSharedToken(), sizeof(aOldEffectiveToken));
		m_aWebSharedToken[0] = '\0';
		if(str_comp(aOldEffectiveToken, EffectiveSharedToken()) != 0)
			StopPresence(true);
		ResetTokenTask();
	}
	else if(m_pTokenTask && m_pTokenTask->State() == EHttpState::ABORTED)
	{
		DebugLog("token request aborted");
		ResetTokenTask();
	}

	if(PresenceEnabled)
		UpdatePresence();

	const bool ShowIcons = g_Config.m_BclClientIndicatorInNamePlate || g_Config.m_BclClientIndicatorInScoreboard;
	if(PresenceEnabled && ShowIcons && (m_FlagsDirty || CSubsystemTicker::ShouldRunPeriodic(Now, m_LastFlagRebuildTick, time_freq() / 2)))
		RebuildPlayerFlags();

	if(g_Config.m_DbgClientIndicator >= 2)
	{
		m_LastUpdateCostTick = time_get() - PerfStart;
		m_MaxUpdateCostTick = std::max(m_MaxUpdateCostTick, m_LastUpdateCostTick);
		m_TotalUpdateCostTick += m_LastUpdateCostTick;
		++m_UpdateSamples;
		if(m_LastPerfReportTick == 0 || Now - m_LastPerfReportTick >= time_freq())
		{
			DebugLogF("perf last=%.3fms avg=%.3fms max=%.3fms samples=%lld socket=%d",
				m_LastUpdateCostTick * 1000.0 / (double)time_freq(),
				m_UpdateSamples > 0 ? (m_TotalUpdateCostTick * 1000.0 / (double)time_freq()) / (double)m_UpdateSamples : 0.0,
				m_MaxUpdateCostTick * 1000.0 / (double)time_freq(),
				(long long)m_UpdateSamples,
				m_Socket != nullptr ? 1 : 0);
			m_LastPerfReportTick = Now;
			m_TotalUpdateCostTick = 0;
			m_UpdateSamples = 0;
			m_MaxUpdateCostTick = 0;
		}
	}
}

void CClientIndicator::OpenPresenceSocket()
{
	NormalizeBestClientLightIndicatorConfig();
	if(m_Socket)
	{
		DebugLog("presence socket already open");
		return;
	}
	if(g_Config.m_BclClientIndicatorServerAddress[0] == '\0')
	{
		SetPresenceBlockReason("presence socket open skipped: server address is empty");
		return;
	}
	if(!BestClientLightIndicator::ParseAddress(g_Config.m_BclClientIndicatorServerAddress, BestClientLightIndicator::DEFAULT_PORT, m_ServerAddr))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "presence socket open failed: cannot parse server address '%s'", g_Config.m_BclClientIndicatorServerAddress);
		SetPresenceBlockReason(aBuf);
		return;
	}
	if(IsBlockedIndicatorAddress(m_ServerAddr))
	{
		char aServerAddr[NETADDR_MAXSTRSIZE];
		net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "presence socket open blocked: target address %s is local", aServerAddr);
		SetPresenceBlockReason(aBuf);
		return;
	}

	NETADDR Bind = NETADDR_ZEROED;
	Bind.type = NETTYPE_ALL;
	Bind.port = 0;
	m_Socket = net_udp_create(Bind);
	if(!m_Socket)
	{
		SetPresenceBlockReason("presence socket open failed: net_udp_create returned null");
		return;
	}
	net_set_non_blocking(m_Socket);
	m_HasServerAddr = true;
	str_copy(m_aLastPresenceServerAddr, g_Config.m_BclClientIndicatorServerAddress, sizeof(m_aLastPresenceServerAddr));
	char aServerAddr[NETADDR_MAXSTRSIZE];
	net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
	ClearPresenceBlockReason();
	DebugLogF("presence socket opened, udp target=%s", aServerAddr);
}

void CClientIndicator::ClosePresenceSocket()
{
	if(m_Socket)
	{
		DebugLog("closing presence socket");
		net_udp_close(m_Socket);
		m_Socket = nullptr;
	}
	m_HasServerAddr = false;
	m_aLastPresenceServerAddr[0] = '\0';
}

void CClientIndicator::StopPresence(bool SendLeavePackets)
{
	const bool HadPresenceState = m_Socket || !m_RegisteredClientIds.empty() || m_aLastPresenceServerAddr[0] != '\0';
	if(HadPresenceState && SendLeavePackets)
	{
		DebugLog("stopping presence and sending leave packets");
		SendLeaveForAll();
	}
	else if(HadPresenceState)
	{
		DebugLog("stopping presence without leave packets");
	}
	ClosePresenceSocket();
	ResetPresenceState();
}

void CClientIndicator::EnsurePresenceSocket()
{
	NormalizeBestClientLightIndicatorConfig();
	if(Client()->State() != IClient::STATE_ONLINE)
	{
		SetPresenceBlockReason("presence socket skipped: client offline");
		return;
	}

	const bool HadPresenceServer = m_aLastPresenceServerAddr[0] != '\0' || m_Socket || m_HasServerAddr;
	const bool ServerChanged = HadPresenceServer && str_comp(m_aLastPresenceServerAddr, g_Config.m_BclClientIndicatorServerAddress) != 0;
	if(ServerChanged)
	{
		DebugLogF("presence server address changed, resetting state old=%s new=%s", m_aLastPresenceServerAddr, g_Config.m_BclClientIndicatorServerAddress);
		StopPresence(true);
	}

	if(m_Socket || g_Config.m_BclClientIndicatorServerAddress[0] == '\0')
	{
		if(!m_Socket && g_Config.m_BclClientIndicatorServerAddress[0] == '\0')
			SetPresenceBlockReason("presence socket skipped: indicator server address is empty");
		return;
	}

	const int64_t Now = time_get();
	if(m_LastPresenceStartAttempt != 0 && Now - m_LastPresenceStartAttempt < time_freq())
	{
		if(m_LastPresenceBlockReason.empty())
			SetPresenceBlockReason("presence socket skipped: retry throttled");
		return;
	}
	m_LastPresenceStartAttempt = Now;
	OpenPresenceSocket();
}

const char *CClientIndicator::CurrentGameServerAddress()
{
	if(Client()->State() != IClient::STATE_ONLINE)
	{
		m_aLastBlockedGameServerAddr[0] = '\0';
		DebugLog("current game server address unavailable: client offline");
		return "";
	}
	if(IsBlockedIndicatorAddress(Client()->ServerAddress()))
	{
		char aAddr[NETADDR_MAXSTRSIZE];
		net_addr_str(&Client()->ServerAddress(), aAddr, sizeof(aAddr), true);
		if(str_comp(m_aLastBlockedGameServerAddr, aAddr) != 0)
		{
			DebugLogF("current game server address blocked: %s is local", aAddr);
			str_copy(m_aLastBlockedGameServerAddr, aAddr, sizeof(m_aLastBlockedGameServerAddr));
		}
		return "";
	}
	m_aLastBlockedGameServerAddr[0] = '\0';
	char aAddr[NETADDR_MAXSTRSIZE];
	net_addr_str(&Client()->ServerAddress(), aAddr, sizeof(aAddr), true);
	str_copy(m_aLastGameServerAddr, aAddr, sizeof(m_aLastGameServerAddr));
	return m_aLastGameServerAddr;
}

const char *CClientIndicator::PlayerNameForClient(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return "";
	if(GameClient()->m_aClients[ClientId].m_Active && GameClient()->m_aClients[ClientId].m_aName[0] != '\0')
		return GameClient()->m_aClients[ClientId].m_aName;
	if(ClientId == GameClient()->m_aLocalIds[IClient::CONN_MAIN])
		return Client()->PlayerName();
	if(ClientId == GameClient()->m_aLocalIds[IClient::CONN_DUMMY])
		return Client()->DummyName();
	return "";
}

void CClientIndicator::SendPresencePacket(int ClientId, int PacketType)
{
	if(!m_Socket || !m_HasServerAddr || ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;
	const char *pSharedToken = EffectiveSharedToken();
	if(!pSharedToken || pSharedToken[0] == '\0')
		return;

	m_vSendPacket.clear();
	m_vSendPacket.reserve(256);
	BestClientLightIndicator::WriteHeader(m_vSendPacket, (BestClientLightIndicator::EPacketType)PacketType);
	BestClientLightIndicator::WriteUuid(m_vSendPacket, m_ClientInstanceId);
	const CUuid Nonce = RandomUuid();
	BestClientLightIndicator::WriteUuid(m_vSendPacket, Nonce);
	BestClientLightIndicator::WriteU64(m_vSendPacket, (uint64_t)time_timestamp());
	BestClientLightIndicator::WriteString(m_vSendPacket, CurrentGameServerAddress());
	BestClientLightIndicator::WriteString(m_vSendPacket, PlayerNameForClient(ClientId));
	BestClientLightIndicator::WriteS16(m_vSendPacket, (int16_t)ClientId);
	BestClientLightIndicator::AppendProof(m_vSendPacket, pSharedToken);

	if(g_Config.m_DbgClientIndicator >= 2)
		DumpUdpPacketBytes("sent", m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());

	const int Sent = net_udp_send(m_Socket, &m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());
	char aServerAddr[NETADDR_MAXSTRSIZE];
	net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
	DebugLogF("sent %s packet client_id=%d player='%s' game_server=%s indicator_server=%s bytes=%d result=%d",
		PacketTypeName(PacketType), ClientId, PlayerNameForClient(ClientId), CurrentGameServerAddress(), aServerAddr, (int)m_vSendPacket.size(), Sent);
}

void CClientIndicator::SendDevAuthPacket(int ClientId)
{
	NormalizeBestClientLightIndicatorConfig();
	if(!m_Socket || !m_HasServerAddr || ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;
	if(g_Config.m_BclClientIndicatorSecretKey[0] == '\0')
		return;

	m_vSendPacket.clear();
	m_vSendPacket.reserve(256);
	BestClientLightIndicator::WriteHeader(m_vSendPacket, BestClientLightIndicator::PACKET_DEV_AUTH);
	BestClientLightIndicator::WriteUuid(m_vSendPacket, m_ClientInstanceId);
	const CUuid Nonce = RandomUuid();
	BestClientLightIndicator::WriteUuid(m_vSendPacket, Nonce);
	BestClientLightIndicator::WriteU64(m_vSendPacket, (uint64_t)time_timestamp());
	BestClientLightIndicator::WriteString(m_vSendPacket, CurrentGameServerAddress());
	BestClientLightIndicator::WriteString(m_vSendPacket, PlayerNameForClient(ClientId));
	BestClientLightIndicator::WriteS16(m_vSendPacket, (int16_t)ClientId);
	BestClientLightIndicator::AppendHmacSha256(m_vSendPacket, g_Config.m_BclClientIndicatorSecretKey);

	if(g_Config.m_DbgClientIndicator >= 2)
		DumpUdpPacketBytes("sent", m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());

	const int Sent = net_udp_send(m_Socket, &m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());
	char aServerAddr[NETADDR_MAXSTRSIZE];
	net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
	DebugLogF("sent dev_auth packet client_id=%d player='%s' game_server=%s indicator_server=%s bytes=%d result=%d",
		ClientId, PlayerNameForClient(ClientId), CurrentGameServerAddress(), aServerAddr, (int)m_vSendPacket.size(), Sent);
}

void CClientIndicator::SendVersionPacket(int ClientId)
{
	if(!m_Socket || !m_HasServerAddr)
		return;

	const char *pSharedToken = EffectiveSharedToken();
	if(pSharedToken[0] == '\0' || BESTCLIENT_LIGHT_VERSION[0] == '\0')
		return;

	char aVersion[64];
	char aOfficialCode[128];
	if(!BuildVersionAnnounceFields(aVersion, sizeof(aVersion), aOfficialCode, sizeof(aOfficialCode)))
		return;

	m_vSendPacket.clear();
	m_vSendPacket.reserve(256);
	BestClientLightIndicator::WriteHeader(m_vSendPacket, BestClientLightIndicator::PACKET_VERSION_ANNOUNCE);
	BestClientLightIndicator::WriteUuid(m_vSendPacket, m_ClientInstanceId);
	CUuid Nonce = RandomUuid();
	BestClientLightIndicator::WriteUuid(m_vSendPacket, Nonce);
	BestClientLightIndicator::WriteU64(m_vSendPacket, (uint64_t)time_timestamp());
	BestClientLightIndicator::WriteString(m_vSendPacket, CurrentGameServerAddress());
	BestClientLightIndicator::WriteString(m_vSendPacket, PlayerNameForClient(ClientId));
	BestClientLightIndicator::WriteS16(m_vSendPacket, (int16_t)ClientId);
	BestClientLightIndicator::WriteString(m_vSendPacket, aVersion);
	BestClientLightIndicator::WriteString(m_vSendPacket, aOfficialCode);
	BestClientLightIndicator::WriteU8(m_vSendPacket, 1);
	BestClientLightIndicator::AppendProof(m_vSendPacket, pSharedToken);

	if(g_Config.m_DbgClientIndicator >= 2)
		DumpUdpPacketBytes("sent", m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());

	const int Sent = net_udp_send(m_Socket, &m_ServerAddr, m_vSendPacket.data(), (int)m_vSendPacket.size());
	char aServerAddr[NETADDR_MAXSTRSIZE];
	net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
	DebugLogF("sent %s packet client_id=%d version='%s' official_code=%s player='%s' game_server=%s indicator_server=%s bytes=%d result=%d",
		PacketTypeName(BestClientLightIndicator::PACKET_VERSION_ANNOUNCE), ClientId, aVersion, aOfficialCode[0] != '\0' ? "set" : "empty", PlayerNameForClient(ClientId), CurrentGameServerAddress(), aServerAddr, (int)m_vSendPacket.size(), Sent);
}

void CClientIndicator::SendLeaveForAll()
{
	if(!m_Socket || !m_HasServerAddr)
		return;
	for(const int ClientId : m_RegisteredClientIds)
		SendPresencePacket(ClientId, BestClientLightIndicator::PACKET_LEAVE);
}

void CClientIndicator::ProcessIncomingPackets(bool Force)
{
	if(!m_Socket || !m_HasServerAddr)
		return;

	const int64_t StartTick = time_get();
	if(!CSubsystemTicker::ShouldRunPeriodic(StartTick, m_LastPresencePollTick, time_freq() / 20, Force))
		return;
	int ReceivedPackets = 0;
	int ProcessedPackets = 0;

	for(int PacketCount = 0; PacketCount < BestClientLightIndicator::MAX_RECEIVE_PACKETS_PER_TICK; ++PacketCount)
	{
		NETADDR From = NETADDR_ZEROED;
		unsigned char *pRawData = nullptr;
		const int DataSize = net_udp_recv(m_Socket, &From, &pRawData);
		if(DataSize <= 0 || !pRawData)
			break;
		ReceivedPackets++;
		if(g_Config.m_DbgClientIndicator >= 2)
		{
			char aFrom[NETADDR_MAXSTRSIZE];
			net_addr_str(&From, aFrom, sizeof(aFrom), true);
			BestClientLightIndicator::EPacketType Type = (BestClientLightIndicator::EPacketType)0;
			int Offset = 0;
			const bool HasHeader = BestClientLightIndicator::ReadHeader(pRawData, DataSize, Type, Offset);
			char aServerAddr[NETADDR_MAXSTRSIZE];
			net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
			if(HasHeader)
				DebugLogF("received udp packet from=%s bytes=%d type=%d(%s) expected_indicator_server=%s", aFrom, DataSize, (int)Type, PacketTypeName((int)Type), aServerAddr);
			else
				DebugLogF("received udp packet from=%s bytes=%d type=invalid expected_indicator_server=%s", aFrom, DataSize, aServerAddr);

			DumpUdpPacketBytes("recv", From, pRawData, DataSize);
		}

		if(net_addr_comp(&From, &m_ServerAddr) != 0)
		{
			if(g_Config.m_DbgClientIndicator >= 2)
			{
				char aFrom[NETADDR_MAXSTRSIZE];
				char aServerAddr[NETADDR_MAXSTRSIZE];
				net_addr_str(&From, aFrom, sizeof(aFrom), true);
				net_addr_str(&m_ServerAddr, aServerAddr, sizeof(aServerAddr), true);
				DebugLogF("ignoring udp packet from=%s (expected indicator_server=%s)", aFrom, aServerAddr);
			}
			continue;
		}

		ProcessedPackets++;
		BestClientLightIndicator::CPeerState PeerState;
		if(BestClientLightIndicator::ReadPeerStatePacket(pRawData, DataSize, PeerState))
		{
			DebugLogF("received peer_state client_id=%d player='%s' server=%s", PeerState.m_ClientId, PeerState.m_PlayerName.c_str(), PeerState.m_ServerAddress.c_str());
			if(IsValidPeerClientId(PeerState.m_ClientId) && PeerState.m_ServerAddress == m_PresenceCache.ServerAddress())
			{
				m_PresenceCache.SetPresent(PeerState.m_ClientId, true);
			}
			continue;
		}

		if(BestClientLightIndicator::ReadPeerRemovePacket(pRawData, DataSize, PeerState))
		{
			DebugLogF("received peer_remove client_id=%d player='%s' server=%s", PeerState.m_ClientId, PeerState.m_PlayerName.c_str(), PeerState.m_ServerAddress.c_str());
			if(IsValidPeerClientId(PeerState.m_ClientId) && PeerState.m_ServerAddress == m_PresenceCache.ServerAddress())
			{
				m_PresenceCache.SetPresent(PeerState.m_ClientId, false);
				m_DeveloperClientIds.erase(PeerState.m_ClientId);
				m_FakeClientIds.erase(PeerState.m_ClientId);
				m_LightClientIds.erase(PeerState.m_ClientId);
				m_ClientVersions.erase(PeerState.m_ClientId);
			}
			continue;
		}

		BestClientLightIndicator::CPeerList PeerList;
		if(BestClientLightIndicator::ReadPeerListPacket(pRawData, DataSize, PeerList) &&
			PeerList.m_ServerAddress == m_PresenceCache.ServerAddress())
		{
			DebugLogF("received peer_list server=%s count=%llu", PeerList.m_ServerAddress.c_str(), (unsigned long long)PeerList.m_vClientIds.size());
			PeerList.m_vClientIds.erase(std::remove_if(PeerList.m_vClientIds.begin(), PeerList.m_vClientIds.end(),
							    [](int ClientId) { return !IsValidPeerClientId(ClientId); }),
				PeerList.m_vClientIds.end());
			m_PresenceCache.Replace(PeerList.m_vClientIds);
			for(auto It = m_LightClientIds.begin(); It != m_LightClientIds.end();)
			{
				if(!m_PresenceCache.IsPresent(*It))
					It = m_LightClientIds.erase(It);
				else
					++It;
			}
			continue;
		}

		if(BestClientLightIndicator::ReadPeerDevStatePacket(pRawData, DataSize, PeerState))
		{
			DebugLogF("received peer_dev_state client_id=%d developer=%d player='%s' server=%s", PeerState.m_ClientId, PeerState.m_Developer ? 1 : 0, PeerState.m_PlayerName.c_str(), PeerState.m_ServerAddress.c_str());
			if(IsValidPeerClientId(PeerState.m_ClientId) && PeerState.m_ServerAddress == m_PresenceCache.ServerAddress())
			{
				if(PeerState.m_Developer)
					m_DeveloperClientIds.insert(PeerState.m_ClientId);
				else
					m_DeveloperClientIds.erase(PeerState.m_ClientId);
			}
			continue;
		}

		if(BestClientLightIndicator::ReadPeerDevListPacket(pRawData, DataSize, PeerList) &&
			PeerList.m_ServerAddress == m_PresenceCache.ServerAddress())
		{
			DebugLogF("received peer_dev_list server=%s count=%llu", PeerList.m_ServerAddress.c_str(), (unsigned long long)PeerList.m_vClientIds.size());
			m_DeveloperClientIds.clear();
			for(const int ClientId : PeerList.m_vClientIds)
			{
				if(IsValidPeerClientId(ClientId))
					m_DeveloperClientIds.insert(ClientId);
			}
			continue;
		}

		BestClientLightIndicator::CPeerVersionState PeerVersionState;
		if(BestClientLightIndicator::ReadPeerVersionStatePacket(pRawData, DataSize, PeerVersionState))
		{
			DebugLogF("received peer_version_state client_id=%d version='%s' fake=%d player='%s' server=%s", PeerVersionState.m_ClientId, PeerVersionState.m_ClientVersion.c_str(), PeerVersionState.m_Fake ? 1 : 0, PeerVersionState.m_PlayerName.c_str(), PeerVersionState.m_ServerAddress.c_str());
			if(IsValidPeerClientId(PeerVersionState.m_ClientId) && PeerVersionState.m_ServerAddress == m_PresenceCache.ServerAddress())
			{
				m_ClientVersions[PeerVersionState.m_ClientId] = PeerVersionState.m_ClientVersion;
				if(PeerVersionState.m_Fake)
					m_FakeClientIds.insert(PeerVersionState.m_ClientId);
				else
					m_FakeClientIds.erase(PeerVersionState.m_ClientId);
			}
			continue;
		}

		if(BestClientLightIndicator::ReadPeerEditionStatePacket(pRawData, DataSize, PeerState))
		{
			DebugLogF("received peer_edition_state client_id=%d light=%d player='%s' server=%s", PeerState.m_ClientId, PeerState.m_Light ? 1 : 0, PeerState.m_PlayerName.c_str(), PeerState.m_ServerAddress.c_str());
			if(IsValidPeerClientId(PeerState.m_ClientId) && PeerState.m_ServerAddress == m_PresenceCache.ServerAddress())
			{
				if(PeerState.m_Light)
					m_LightClientIds.insert(PeerState.m_ClientId);
				else
					m_LightClientIds.erase(PeerState.m_ClientId);
			}
			continue;
		}

		BestClientLightIndicator::CDevAuthResult DevAuthResult;
		if(BestClientLightIndicator::ReadDevAuthResultPacket(pRawData, DataSize, DevAuthResult) &&
			DevAuthResult.m_ServerAddress == m_PresenceCache.ServerAddress())
		{
			DebugLogF("received dev_auth_result client_id=%d success=%d server=%s", DevAuthResult.m_ClientId, DevAuthResult.m_Success ? 1 : 0, DevAuthResult.m_ServerAddress.c_str());
			if(IsValidPeerClientId(DevAuthResult.m_ClientId))
			{
				if(DevAuthResult.m_Success)
				{
					m_DeveloperClientIds.insert(DevAuthResult.m_ClientId);
				}
				else
					m_DeveloperClientIds.erase(DevAuthResult.m_ClientId);
			}
			continue;
		}

		if(g_Config.m_DbgClientIndicator >= 2)
			DebugLogF("received udp packet from indicator server but did not match known packet formats bytes=%d", DataSize);
	}

	if(ProcessedPackets > 0)
		m_FlagsDirty = true;

	const int64_t EndTick = time_get();
	const int64_t Delta = EndTick - StartTick;
	if(g_Config.m_DbgClientIndicator && Delta > SlowPacketProcessTicks())
	{
		const int64_t Ms = (Delta * 1000) / time_freq();
		DebugLogF("ProcessIncomingPackets slow: %lldms received=%d processed=%d", (long long)Ms, ReceivedPackets, ProcessedPackets);
	}
}

void CClientIndicator::SyncLocalRegistrations(bool Force)
{
	if(!m_Socket || !m_HasServerAddr)
		return;

	const int64_t Now = time_get();
	if(!CSubsystemTicker::ShouldRunPeriodic(Now, m_LastRegistrationSyncTick, time_freq() / 4, Force))
		return;

	std::array<bool, MAX_CLIENTS> aClientActive{};
	for(const int ClientId : GameClient()->m_aLocalIds)
	{
		if(ClientId >= 0 && ClientId < MAX_CLIENTS)
			aClientActive[ClientId] = GameClient()->m_aClients[ClientId].m_Active;
	}
	const auto DesiredClientIds = BestClientLightIndicatorClient::CollectActiveLocalClientIds(GameClient()->m_aLocalIds, aClientActive);

	for(const int ClientId : DesiredClientIds.m_vClientIds)
	{
		if(m_RegisteredClientIds.find(ClientId) == m_RegisteredClientIds.end())
		{
			SendPresencePacket(ClientId, BestClientLightIndicator::PACKET_JOIN);
			SendVersionPacket(ClientId);
			SendDevAuthPacket(ClientId);
			m_RegisteredClientIds.insert(ClientId);
			m_PresenceCache.SetPresent(ClientId, true);
			m_FlagsDirty = true;
			DebugLogF("registered local client_id=%d name='%s'", ClientId, PlayerNameForClient(ClientId));
		}
	}

	for(auto It = m_RegisteredClientIds.begin(); It != m_RegisteredClientIds.end();)
	{
		if(!DesiredClientIds.Contains(*It))
		{
			SendPresencePacket(*It, BestClientLightIndicator::PACKET_LEAVE);
			m_PresenceCache.SetPresent(*It, false);
			m_DeveloperClientIds.erase(*It);
			m_LightClientIds.erase(*It);
			m_FlagsDirty = true;
			DebugLogF("unregistered local client_id=%d", *It);
			It = m_RegisteredClientIds.erase(It);
		}
		else
		{
			++It;
		}
	}
}

void CClientIndicator::UpdatePresence()
{
	if(Client()->State() != IClient::STATE_ONLINE)
	{
		if(m_WasPresenceEnabled || m_Socket)
			StopPresence(true);
		SetPresenceBlockReason("presence update skipped: client offline");
		m_WasPresenceEnabled = false;
		return;
	}

	if(EffectiveSharedToken()[0] == '\0')
	{
		SetPresenceBlockReason("presence update skipped: effective shared token is empty");
		m_WasPresenceEnabled = true;
		return;
	}

	EnsurePresenceSocket();
	if(!m_Socket || !m_HasServerAddr)
	{
		if(m_LastPresenceBlockReason.empty())
			SetPresenceBlockReason("presence update skipped: udp socket is not ready");
		m_WasPresenceEnabled = true;
		return;
	}

	const char *pCurrentGameServer = CurrentGameServerAddress();
	if(pCurrentGameServer[0] == '\0')
	{
		SetPresenceBlockReason("presence update skipped: current game server address is empty");
		if(m_PresenceCache.SetServerAddress(""))
		{
			m_RegisteredClientIds.clear();
			m_DeveloperClientIds.clear();
			m_LightClientIds.clear();
			m_FlagsDirty = true;
			m_LastHeartbeatTick = 0;
		}
		m_WasPresenceEnabled = true;
		return;
	}
	if(m_PresenceCache.SetServerAddress(pCurrentGameServer))
	{
		DebugLogF("presence server changed to game server %s", pCurrentGameServer);
		m_RegisteredClientIds.clear();
		m_DeveloperClientIds.clear();
		m_FakeClientIds.clear();
		m_LightClientIds.clear();
		m_ClientVersions.clear();
		m_FlagsDirty = true;
		m_LastHeartbeatTick = 0;
	}
	str_copy(m_aCachedGameServerAddr, pCurrentGameServer, sizeof(m_aCachedGameServerAddr));
	ClearPresenceBlockReason();

	const int64_t Now = time_get();
	if(m_LastPollTick == 0 || Now - m_LastPollTick >= time_freq() / 10) // 10 Hz (~every 5 ticks)
	{
		m_LastPollTick = Now;
		SyncLocalRegistrations(false);
		ProcessIncomingPackets(false);
	}
	if(m_LastHeartbeatTick == 0 || Now - m_LastHeartbeatTick > time_freq() * 5)
	{
		const bool SendDevAuth = g_Config.m_BclClientIndicatorSecretKey[0] != '\0';
		for(const int ClientId : m_RegisteredClientIds)
		{
			SendPresencePacket(ClientId, BestClientLightIndicator::PACKET_HEARTBEAT);
			SendVersionPacket(ClientId);
			if(SendDevAuth)
				SendDevAuthPacket(ClientId);
		}
		m_LastHeartbeatTick = Now;
		DebugLogF("heartbeat tick sent for %llu local clients", (unsigned long long)m_RegisteredClientIds.size());
	}

	m_WasPresenceEnabled = true;
}

void CClientIndicator::FinishBrowserCacheRefresh()
{
	if(!m_pBrowserTask)
		return;
	json_value *pJson = m_pBrowserTask->ResultJson();
	if(!pJson)
	{
		DebugLog("browser request completed but JSON parsing returned null");
		return;
	}
	m_BrowserCache.Load(*pJson);
	json_value_free(pJson);
	DebugLogF("browser cache loaded %llu entries", (unsigned long long)m_BrowserCache.Players().size());
	ApplyBrowserSnapshot();
}

void CClientIndicator::ResetBrowserTask()
{
	if(m_pBrowserJob)
	{
		m_pBrowserJob->Abort();
		m_pBrowserJob = nullptr;
	}
	if(m_pBrowserTask)
	{
		m_pBrowserTask->Abort();
		m_pBrowserTask = nullptr;
	}
}

void CClientIndicator::FinishTokenRefresh()
{
	if(!m_pTokenTask)
		return;
	json_value *pJson = m_pTokenTask->ResultJson();
	char aOldEffectiveToken[sizeof(m_aWebSharedToken)];
	str_copy(aOldEffectiveToken, EffectiveSharedToken(), sizeof(aOldEffectiveToken));
	m_aWebSharedToken[0] = '\0';

	if(pJson)
	{
		const char *pToken = nullptr;
		if(pJson->type == json_object)
		{
			const json_value &Token = (*pJson)["token"];
			if(Token.type == json_string)
				pToken = Token.u.string.ptr;
		}

		if(pToken && pToken[0] != '\0')
			str_copy(m_aWebSharedToken, pToken, sizeof(m_aWebSharedToken));

		json_value_free(pJson);
	}
	else
	{
		DebugLog("token request completed but JSON parsing returned null");
	}

	if(m_aWebSharedToken[0] != '\0')
		DebugLogF("token refresh succeeded, web token length=%d", str_length(m_aWebSharedToken));
	else
		DebugLogF("token refresh produced empty web token, fallback token length=%d", str_length(g_Config.m_BclClientIndicatorSharedToken));

	if(str_comp(aOldEffectiveToken, EffectiveSharedToken()) != 0)
	{
		DebugLog("effective token changed, restarting presence state");
		StopPresence(true);
	}
}

void CClientIndicator::ResetTokenTask()
{
	if(m_pTokenTask)
	{
		m_pTokenTask->Abort();
		m_pTokenTask = nullptr;
	}
}

void CClientIndicator::ApplyBrowserSnapshot()
{
	ServerBrowser()->SetBestClientLightPlayers(m_BrowserCache.Players());
	m_FlagsDirty = true;
}

void CClientIndicator::ResetPresenceState()
{
	m_LastHeartbeatTick = 0;
	m_LastPollTick = 0;
	m_LastPresenceStartAttempt = 0;
	m_WasPresenceEnabled = false;
	m_RegisteredClientIds.clear();
	m_DeveloperClientIds.clear();
	m_FakeClientIds.clear();
	m_LightClientIds.clear();
	m_ClientVersions.clear();
	m_aBestClient.fill(false);
	m_aLightEdition.fill(false);
	m_aDeveloper.fill(false);
	m_aFake.fill(false);
	m_FlagsDirty = true;
	m_PresenceCache.Clear();
	m_aLastGameServerAddr[0] = '\0';
	m_aLastBlockedGameServerAddr[0] = '\0';
	m_aCachedGameServerAddr[0] = '\0';
	m_LastPresenceBlockReason.clear();
}

void CClientIndicator::ResetTokenState()
{
	ResetTokenTask();
	m_aWebSharedToken[0] = '\0';
}

void CClientIndicator::ReapplyBrowserSnapshot()
{
	ApplyBrowserSnapshot();
}

bool CClientIndicator::IsBrowserCacheRefreshing() const
{
	return (m_pBrowserTask && !m_pBrowserTask->Done()) || (m_pBrowserJob && !m_pBrowserJob->Done());
}

bool CClientIndicator::IsPresenceEnabled() const
{
	return Client()->State() == IClient::STATE_ONLINE;
}

const char *CClientIndicator::EffectiveSharedToken() const
{
	if(m_aWebSharedToken[0] != '\0')
		return m_aWebSharedToken;
	return g_Config.m_BclClientIndicatorSharedToken;
}

void CClientIndicator::DebugLog(const char *pText) const
{
	if(!g_Config.m_DbgClientIndicator)
		return;
	log_info(LOG_SCOPE, "%s", pText);
}

void CClientIndicator::DebugLogF(const char *pFormat, ...) const
{
	if(!g_Config.m_DbgClientIndicator)
		return;
	char aBuf[1024];
	va_list Args;
	va_start(Args, pFormat);
	str_format_v(aBuf, sizeof(aBuf), pFormat, Args);
	va_end(Args);
	log_info(LOG_SCOPE, "%s", aBuf);
}

void CClientIndicator::SetPresenceBlockReason(const char *pReason)
{
	if(m_LastPresenceBlockReason == pReason)
		return;
	m_LastPresenceBlockReason = pReason;
	DebugLog(pReason);
}

void CClientIndicator::ClearPresenceBlockReason()
{
	m_LastPresenceBlockReason.clear();
}
