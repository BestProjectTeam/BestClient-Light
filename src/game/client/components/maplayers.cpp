/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "maplayers.h"

#include <engine/map.h>

#include <game/client/gameclient.h>
#include <game/localization.h>

CMapLayers::CMapLayers(ERenderType Type, bool OnlineOnly)
{
	m_Type = Type;
	m_OnlineOnly = OnlineOnly;

	// static parameters for ingame rendering
	m_Params.m_RenderType = m_Type;
	m_Params.m_RenderInvalidTiles = false;
	m_Params.m_TileAndQuadBuffering = true;
	m_Params.m_RenderTileBorder = true;
}

void CMapLayers::OnInit()
{
	m_pLayers = Layers();
	m_pImages = &GameClient()->m_MapImages;
	m_MapRenderer.Clear();
}

CCamera *CMapLayers::GetCurCamera()
{
	return &GameClient()->m_Camera;
}

void CMapLayers::OnMapLoad()
{
	char aCaption[64 + MAX_MAP_LENGTH];
	str_format(aCaption, sizeof(aCaption), "%s: %s", Localize("Loading map"), m_pLayers->Map()->BaseName());

	FCallbackMapRendererInit ProgressBarCallback = [&](int GroupId, int NumGroups, int LayerId, int NumLayers) {
		GameClient()->m_Menus.RenderLoadingDirect(aCaption, Localize("Initializing layers"), std::make_optional((GroupId + (float)LayerId / NumLayers) / (float)NumGroups));
	};

	// can't do that in CMapLayers::OnInit, because some of this interfaces are not available yet
	m_MapRenderer.OnInit(Graphics(), TextRender(), RenderMap());

	m_EnvEvaluator = CEnvelopeState(m_pLayers->Map(), m_OnlineOnly);
	m_EnvEvaluator.OnInterfacesInit(GameClient());
	m_MapRenderer.Load(m_Type, m_pLayers, m_pImages, &m_EnvEvaluator, ProgressBarCallback);
}

void CMapLayers::OnRender()
{
	if(m_OnlineOnly && Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	// dynamic parameters for ingame rendering
	m_Params.m_EntityOverlayVal = m_Type == RENDERTYPE_FULL_DESIGN ? 0 : g_Config.m_ClOverlayEntities;
	m_Params.m_Center = GetCurCamera()->m_Center;
	m_Params.m_Zoom = GetCurCamera()->m_Zoom;
	m_Params.m_RenderText = g_Config.m_ClTextEntities;
	m_Params.m_DebugRenderGroupClips = g_Config.m_DbgRenderGroupClips;
	m_Params.m_DebugRenderQuadClips = g_Config.m_DbgRenderQuadClips;
	m_Params.m_DebugRenderClusterClips = g_Config.m_DbgRenderClusterClips;
	m_Params.m_DebugRenderTileClips = g_Config.m_DbgRenderTileClips;

	// bestclient-light
	m_Params.m_FpsFogEnabled = GameClient()->OptimizerFpsFogEnabled();
	m_Params.m_FpsFogCullMapTiles = GameClient()->m_Optimizer.CullMapTiles();
	m_Params.m_FpsFogHalfW = GameClient()->m_Optimizer.FogHalfW();
	m_Params.m_FpsFogHalfH = GameClient()->m_Optimizer.FogHalfH();
	m_Params.m_DisableMapQuads = GameClient()->m_Optimizer.DisableQuads();
	// bestclient-light

	m_MapRenderer.Render(m_Params);
}

// bestclient-light
void CMapLayers::RenderFlatDesignTiles(vec2 WorldMin, vec2 WorldMax, int ViewportX, int ViewportY, int ViewportW, int ViewportH, float Alpha)
{
	if(Alpha <= 0.0f || ViewportW <= 0 || ViewportH <= 0 || !Graphics()->IsTileBufferingEnabled())
		return;
	CRenderLayerParams Params = m_Params;
	Params.m_EntityOverlayVal = 0;
	Params.m_RenderTileBorder = false;
	Params.m_TileAndQuadBuffering = true;
	Params.m_FpsFogEnabled = false;
	Params.m_FpsFogCullMapTiles = false;
	Params.m_DisableMapQuads = false;
	Params.m_ExtraAlpha = Alpha;
	Params.m_DebugRenderGroupClips = false;
	Params.m_DebugRenderQuadClips = false;
	Params.m_DebugRenderClusterClips = false;
	Params.m_DebugRenderTileClips = false;
	Graphics()->BlendNormal();
	Graphics()->WrapNormal();
	m_MapRenderer.RenderFlatDesignTiles(WorldMin, WorldMax, ViewportX, ViewportY, ViewportW, ViewportH, Params);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}
// bestclient-light
