/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_VERSION_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_VERSION_H

#define BESTCLIENT_LIGHT_BUILD_DATE "28.05 00:00"
#define BESTCLIENT_LIGHT_VERSIONNR 100
#define BESTCLIENT_LIGHT_VERSION "5.0 closed-beta"

void BestClientLightFormatDisplayVersion(const char *pVersion, char *pOut, int OutSize);
void BestClientLightFormatNetVersion(char *pOut, int OutSize, const char *pGitHash);

#ifndef BESTCLIENT_LIGHT_OFFICIAL_CODE
#define BESTCLIENT_LIGHT_OFFICIAL_CODE ""
#endif

#endif
