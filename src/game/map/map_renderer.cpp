#include "map_renderer.h"

#include <base/dbg.h>
#include <base/log.h>

#include <engine/graphics.h>

#include <game/map/envelope_manager.h>

#include <algorithm>
#include <cmath>

const int LAYER_DEFAULT_TILESET = -1;

void CMapRenderer::Clear()
{
	for(auto &pLayer : m_vpRenderLayers)
		pLayer->Unload();
	m_vpRenderLayers.clear();
}

void CMapRenderer::Load(ERenderType Type, CLayers *pLayers, IMapImages *pMapImages, const IEnvelopeEval *pEnvelopeEval, std::optional<FCallbackMapRendererInit> CallbackMapRendererInitOptional)
{
	Clear();

	std::shared_ptr<CEnvelopeManager> pEnvelopeManager = std::make_shared<CEnvelopeManager>(pEnvelopeEval, pLayers->Map());
	bool PassedGameLayer = false;

	for(int GroupId = 0; GroupId < pLayers->NumGroups(); GroupId++)
	{
		CMapItemGroup *pGroup = pLayers->GetGroup(GroupId);
		std::unique_ptr<CRenderLayer> pRenderLayerGroup = std::make_unique<CRenderLayerGroup>(GroupId, pGroup);

		std::optional<FCallbackLayerInit> CallbackLayerInitOptional;
		if(CallbackMapRendererInitOptional.has_value())
		{
			CallbackLayerInitOptional = [&](int LayerGroupId, int LayerId) {
				(*CallbackMapRendererInitOptional)(LayerGroupId, pLayers->NumGroups(), LayerId, pGroup->m_NumLayers);
			};
		}

		pRenderLayerGroup->OnInit(Graphics(), TextRender(), RenderMap(), pEnvelopeManager, pLayers->Map(), pMapImages, CallbackLayerInitOptional);
		if(!pRenderLayerGroup->IsValid())
		{
			log_error("map_renderer", "error group was null, group number = %d, total groups = %d", GroupId, pLayers->NumGroups());
			log_error("map_renderer", "this is here to prevent a crash but the source of this is unknown, please report this for it to get fixed");
			log_error("map_renderer", "we need mapname and crc and the map that caused this if possible, and anymore info you think is relevant");
			continue;
		}

		for(int LayerId = 0; LayerId < pGroup->m_NumLayers; LayerId++)
		{
			CMapItemLayer *pLayer = pLayers->GetLayer(pGroup->m_StartLayer + LayerId);
			int LayerType = GetLayerType(pLayer);
			PassedGameLayer |= LayerType == LAYER_GAME;

			if(Type == ERenderType::RENDERTYPE_BACKGROUND_FORCE || Type == ERenderType::RENDERTYPE_BACKGROUND)
			{
				if(PassedGameLayer)
					return;
			}
			else if(Type == ERenderType::RENDERTYPE_FOREGROUND)
			{
				if(!PassedGameLayer)
					continue;
			}

			if(pRenderLayerGroup)
				m_vpRenderLayers.push_back(std::move(pRenderLayerGroup));

			std::unique_ptr<CRenderLayer> pRenderLayer;

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTileLayer = (CMapItemLayerTilemap *)pLayer;

				switch(LayerType)
				{
				case LAYER_DEFAULT_TILESET:
					pRenderLayer = std::make_unique<CRenderLayerTile>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_GAME:
					pRenderLayer = std::make_unique<CRenderLayerEntityGame>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_FRONT:
					pRenderLayer = std::make_unique<CRenderLayerEntityFront>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_TELE:
					pRenderLayer = std::make_unique<CRenderLayerEntityTele>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_SPEEDUP:
					pRenderLayer = std::make_unique<CRenderLayerEntitySpeedup>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_SWITCH:
					pRenderLayer = std::make_unique<CRenderLayerEntitySwitch>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				case LAYER_TUNE:
					pRenderLayer = std::make_unique<CRenderLayerEntityTune>(
						GroupId,
						LayerId,
						pLayer->m_Flags,
						pTileLayer);
					break;
				default:
					dbg_assert_failed("Unknown LayerType %d", LayerType);
				}
			}
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				CMapItemLayerQuads *pQLayer = (CMapItemLayerQuads *)pLayer;

				pRenderLayer = std::make_unique<CRenderLayerQuads>(
					GroupId,
					LayerId,
					pLayer->m_Flags,
					pQLayer);
			}

			// just ignore invalid layers from rendering
			if(pRenderLayer)
			{
				pRenderLayer->OnInit(Graphics(), TextRender(), RenderMap(), pEnvelopeManager, pLayers->Map(), pMapImages, CallbackLayerInitOptional);
				if(pRenderLayer->IsValid())
				{
					pRenderLayer->Init();
					m_vpRenderLayers.push_back(std::move(pRenderLayer));
				}
			}
		}
	}
}

void CMapRenderer::Render(const CRenderLayerParams &Params)
{
	CScreenRect ScreenRect = Graphics()->GetScreen();

	bool DoRenderGroup = true;
	for(auto &pRenderLayer : m_vpRenderLayers)
	{
		if(pRenderLayer->IsGroup())
			DoRenderGroup = pRenderLayer->DoRender(Params);

		if(!DoRenderGroup)
			continue;

		if(pRenderLayer->DoRender(Params))
			pRenderLayer->Render(Params);
	}

	// Reset clip from last group
	Graphics()->ClipDisable();

	// don't reset screen on background
	if(Params.m_RenderType != ERenderType::RENDERTYPE_BACKGROUND && Params.m_RenderType != ERenderType::RENDERTYPE_BACKGROUND_FORCE)
	{
		// reset the screen like it was before
		Graphics()->MapScreen(ScreenRect);
	}
	else
	{
		// reset the screen to the default interface
		Graphics()->MapScreenToInterface(Params.m_Center.x, Params.m_Center.y, Params.m_Zoom);
	}
}

// bestclient-light
void CMapRenderer::RenderFlatDesignTiles(vec2 WorldMin, vec2 WorldMax, int ViewportX, int ViewportY, int ViewportW, int ViewportH, const CRenderLayerParams &Params)
{
	const CScreenRect PreviousScreen = Graphics()->GetScreen();
	const float SpanX = WorldMax.x - WorldMin.x;
	const float SpanY = WorldMax.y - WorldMin.y;
	if(SpanX <= 0.0f || SpanY <= 0.0f || ViewportW <= 0 || ViewportH <= 0)
		return;

	float BaseW = 1.0f;
	float BaseH = 1.0f;
	const float Aspect = SpanX / SpanY;
	Graphics()->CalcScreenParams(Aspect, 1.0f, &BaseW, &BaseH);
	const float Zoom = BaseW > 0.0f ? SpanX / BaseW : 1.0f;
	const vec2 Center = (WorldMin + WorldMax) * 0.5f;
	const float GameAspect = Graphics()->ScreenAspect() > 0.0f ? Graphics()->ScreenAspect() : Aspect;
	const float CameraZoom = Params.m_Zoom > 0.0f ? Params.m_Zoom : 1.0f;
	const bool TileBuffer = Graphics()->IsTileBufferingEnabled();

	bool Pushed = false;
	bool WorldAligned = true;
	for(auto &pRenderLayer : m_vpRenderLayers)
	{
		if(pRenderLayer->IsGroup())
		{
			if(Pushed)
			{
				Graphics()->PopPreviewViewport();
				Pushed = false;
			}
			CRenderLayerGroup *pGroup = static_cast<CRenderLayerGroup *>(pRenderLayer.get());
			vec2 VisibleMin;
			vec2 VisibleMax;
			if(!pGroup->FlatVisibleWorld(WorldMin, WorldMax, VisibleMin, VisibleMax))
				continue;
			const float Fx0 = (VisibleMin.x - WorldMin.x) / SpanX;
			const float Fy0 = (VisibleMin.y - WorldMin.y) / SpanY;
			const float Fx1 = (VisibleMax.x - WorldMin.x) / SpanX;
			const float Fy1 = (VisibleMax.y - WorldMin.y) / SpanY;
			const int SubX = std::clamp(ViewportX + (int)std::floor(Fx0 * (float)ViewportW), ViewportX, ViewportX + ViewportW);
			const int SubY = std::clamp(ViewportY + (int)std::floor(Fy0 * (float)ViewportH), ViewportY, ViewportY + ViewportH);
			const int SubR = std::clamp(ViewportX + (int)std::ceil(Fx1 * (float)ViewportW), ViewportX, ViewportX + ViewportW);
			const int SubB = std::clamp(ViewportY + (int)std::ceil(Fy1 * (float)ViewportH), ViewportY, ViewportY + ViewportH);
			if(SubR <= SubX || SubB <= SubY)
				continue;
			WorldAligned = pGroup->FlatWorldAligned();
			Graphics()->ClipDisable();
			Graphics()->PushPreviewViewport(SubX, SubY, SubR - SubX, SubB - SubY);
			Pushed = true;
			if(WorldAligned)
			{
				const CScreenRect GroupScreen = pGroup->FlatScreen(Center, Zoom, Aspect);
				const float GroupW = GroupScreen.Width();
				const float GroupH = GroupScreen.Height();
				Graphics()->MapScreen(CScreenRect(GroupScreen.m_TopLeft.x + Fx0 * GroupW, GroupScreen.m_TopLeft.y + Fy0 * GroupH, (Fx1 - Fx0) * GroupW, (Fy1 - Fy0) * GroupH));
			}
			else
				Graphics()->MapScreen(pGroup->FlatBackdropScreen(Params.m_Center, CameraZoom, GameAspect));
			continue;
		}
		if(!Pushed)
			continue;
		if(pRenderLayer->IsDesignTileLayer())
		{
			if(!TileBuffer)
				continue;
		}
		else if(!pRenderLayer->IsMapArtLayer())
			continue;
		else if(!WorldAligned)
		{
			CRenderLayerQuads *pQuads = static_cast<CRenderLayerQuads *>(pRenderLayer.get());
			const CScreenRect Backdrop = Graphics()->GetScreen();
			CScreenRect ClipScreen(0.0f, 0.0f, 0.0f, 0.0f);
			bool Remap = false;
			if(pQuads->FlatClipScreen(ClipScreen))
			{
				const float ClipW = ClipScreen.Width();
				const float ClipH = ClipScreen.Height();
				const bool ScreenSized = ClipW >= 400.0f && ClipH >= 250.0f && ClipW <= 20000.0f && ClipH <= 20000.0f;
				if(ScreenSized)
				{
					const float ClipX = ClipScreen.m_TopLeft.x + ClipW * 0.5f;
					const float ClipY = ClipScreen.m_TopLeft.y + ClipH * 0.5f;
					const bool CenterOutside = ClipX < Backdrop.m_TopLeft.x || ClipX > Backdrop.m_BottomRight.x || ClipY < Backdrop.m_TopLeft.y || ClipY > Backdrop.m_BottomRight.y;
					Remap = CenterOutside || !pRenderLayer->DoRender(Params);
				}
			}
			if(Remap)
				Graphics()->MapScreen(ClipScreen);
			if(pRenderLayer->DoRender(Params))
				pRenderLayer->Render(Params);
			if(Remap)
				Graphics()->MapScreen(Backdrop);
			continue;
		}
		if(pRenderLayer->DoRender(Params))
			pRenderLayer->Render(Params);
	}

	if(Pushed)
		Graphics()->PopPreviewViewport();
	Graphics()->ClipDisable();
	Graphics()->MapScreen(PreviousScreen);
}
// bestclient-light

int CMapRenderer::GetLayerType(const CMapItemLayer *pLayer) const
{
	if(pLayer->m_Type != LAYERTYPE_TILES)
		return LAYER_DEFAULT_TILESET;

	// Physics layers must be determined by their flags instead of by comparing them with the
	// layers of CLayers, which only knows the last physics layer of each type, as design tiles
	// layers use the data index which is neither used nor validated for physics layers.
	const int Flags = reinterpret_cast<const CMapItemLayerTilemap *>(pLayer)->m_Flags;
	if(Flags & TILESLAYERFLAG_GAME)
		return LAYER_GAME;
	else if(Flags & TILESLAYERFLAG_FRONT)
		return LAYER_FRONT;
	else if(Flags & TILESLAYERFLAG_SWITCH)
		return LAYER_SWITCH;
	else if(Flags & TILESLAYERFLAG_TELE)
		return LAYER_TELE;
	else if(Flags & TILESLAYERFLAG_SPEEDUP)
		return LAYER_SPEEDUP;
	else if(Flags & TILESLAYERFLAG_TUNE)
		return LAYER_TUNE;
	return LAYER_DEFAULT_TILESET;
}
