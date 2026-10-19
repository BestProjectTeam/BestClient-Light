/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_CLIENTINDICATOR_VERSION_LABEL_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_CLIENTINDICATOR_VERSION_LABEL_H

#include <base/str.h>

#include <cstring>
#include <string>

inline bool BestClientLightIsVersionProofToken(const char *pToken, int Len)
{
	if(Len != 8)
		return false;
	for(int i = 0; i < Len; ++i)
	{
		const char c = pToken[i];
		const bool Hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		if(!Hex)
			return false;
	}
	return true;
}

inline const char *BestClientLightFindVersionProofSep(const char *pRaw)
{
	if(!pRaw || pRaw[0] == '\0')
		return nullptr;
	const char *pLastSpace = nullptr;
	for(const char *p = pRaw; *p; ++p)
	{
		if(*p == ' ')
			pLastSpace = p;
	}
	if(!pLastSpace || pLastSpace == pRaw)
		return nullptr;
	const char *pToken = pLastSpace + 1;
	if(!BestClientLightIsVersionProofToken(pToken, str_length(pToken)))
		return nullptr;
	return pLastSpace;
}

inline void BestClientLightStripVersionProof(char *pVersion)
{
	if(!pVersion)
		return;
	if(char *pSep = (char *)BestClientLightFindVersionProofSep(pVersion))
		*pSep = '\0';
	int End = str_length(pVersion);
	while(End > 0 && pVersion[End - 1] == ' ')
		pVersion[--End] = '\0';
}

inline std::string BestClientLightStripVersionProofStr(const std::string &Version)
{
	char aBuf[128];
	str_copy(aBuf, Version.c_str(), sizeof(aBuf));
	BestClientLightStripVersionProof(aBuf);
	return aBuf;
}

#endif
