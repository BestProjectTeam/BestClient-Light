/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_PROFILE_ASSETS_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_PROFILE_ASSETS_H

#include <game/client/ui.h>

class CMenus;
class CProfile;

void BestClientLightApplyProfileAssets(class CGameClient *pGameClient, const CProfile &Profile);
void BestClientLightFillProfileAssets(CProfile &Profile);
void BestClientLightDoProfileAssetsDropDown(CUi *pUi, CMenus *pMenus, CUIRect *pRect);

#endif
