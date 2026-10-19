#include <engine/discord.h>

class CDiscordStub : public IDiscord
{
	void Update(bool Enabled) override {}
	void ClearGameInfo() override {}
	// bestclient-light
	void SetGameInfo(const CServerInfo &ServerInfo, const char *pMapName, const char *pPlayerName, const char *pSkinName, bool ShowMap, bool Registered) override {}
	void UpdateServerInfo(const CServerInfo &ServerInfo, const char *pMapName, const char *pPlayerName, const char *pSkinName) override {}
	// bestclient-light
	void UpdatePlayerCount(int Count) override {}
};

IDiscord *CreateDiscord()
{
	// bestclient-light
	IDiscord *pDiscord = CreateBestClientLightDiscord();
	// bestclient-light
	if(pDiscord)
	{
		return pDiscord;
	}
	return new CDiscordStub();
}
