/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_FGF_WEATHER_H
#define GAME_CLIENT_COMPONENTS_FGF_WEATHER_H
#include <base/color.h>
#include <base/vmath.h>

#include <engine/graphics.h>

#include <game/client/component.h>
#include "fgf_glow.h"

#include <vector>

enum
{
	FGF_WEATHER_SNOW = 0,
	FGF_WEATHER_RAIN,
	FGF_WEATHER_FIREFLIES,
	FGF_WEATHER_DUST,
};

class CFgfWeather : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
	void OnReset() override;
	void OnRender() override;

private:
	static constexpr int MAX_DROPS = 1024;
	static constexpr int MAX_SPLASHES = 256;

	struct CDrop
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Life;
		float m_LifeSpan;
		float m_Size;
		float m_Phase;
		bool m_Settled;
	};

	struct CSplash
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Life;
		float m_LifeSpan;
		float m_Size;
	};

	IGraphics::CTextureHandle m_GlowTexture;

	int m_Kind = FGF_WEATHER_SNOW;
	bool m_WasActive = false;
	CDrop m_aDrops[MAX_DROPS];
	int m_NumDrops = 0;
	CSplash m_aSplashes[MAX_SPLASHES];
	int m_NumSplashes = 0;
	float m_SpawnAccumulator = 0.0f;
	float m_LastTime = 0.0f;

	int m_MaskOriginX = 0;
	int m_MaskOriginY = 0;
	int m_MaskWidth = 0;
	int m_MaskHeight = 0;
	std::vector<int> m_AirTiles;
	bool m_MaskReady = false;
	bool m_ScanActive = false;
	int m_ScanOriginX = 0;
	int m_ScanOriginY = 0;
	int m_ScanWidth = 0;
	int m_ScanHeight = 0;
	int m_ScanRow = 0;
	std::vector<int> m_ScanAir;

	bool TileOpen(int TileX, int TileY) const;
	void AppendOpenRow(int OriginX, int OriginY, int Width, int LocalY, std::vector<int> &Air) const;
	void ShiftAir(int OriginX, int OriginY, int Width, int Height, int Dx, int Dy);
	void BeginAirScan(int OriginX, int OriginY, int Width, int Height);
	void StepAirScan();
	CDrop *NewDrop();
	void Splash(vec2 Pos);
	void BuildMask(const CScreenRect &View);
	bool SolidAt(vec2 Pos) const;
	bool RandomAirPoint(const CScreenRect &View, vec2 &Pos) const;
	void Spawn(const CScreenRect &View, float Passed, float Time);
	void Update(const CScreenRect &View, float Passed, float Time);
	void Draw(float Time);
	void AmbientDust();
};
#endif
