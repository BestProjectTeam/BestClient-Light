#ifndef GAME_MAP_MAP_RENDERER_H
#define GAME_MAP_MAP_RENDERER_H

#include <engine/map.h>

#include <game/layers.h>
#include <game/map/render_component.h>
#include <game/map/render_layer.h>

typedef std::function<void(int GroupId, int NumGroups, int LayerId, int NumLayers)> FCallbackMapRendererInit;

class CMapRenderer : public CRenderComponent
{
public:
	CMapRenderer() = default;

	void Clear();
	void Load(ERenderType Type, CLayers *pLayers, IMapImages *pMapImages, const IEnvelopeEval *pEnvelopeEval, std::optional<FCallbackMapRendererInit> CallbackMapRendererInitOptional);
	void Render(const CRenderLayerParams &Params);
	// bestclient-light
	void RenderFlatDesignTiles(vec2 WorldMin, vec2 WorldMax, int ViewportX, int ViewportY, int ViewportW, int ViewportH, const CRenderLayerParams &Params);
	// bestclient-light

private:
	int GetLayerType(const CMapItemLayer *pLayer) const;

	std::vector<std::unique_ptr<CRenderLayer>> m_vpRenderLayers;
};

#endif
