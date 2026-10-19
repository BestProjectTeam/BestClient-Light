/* Copyright © 2026 BestProject Team */
#include "inputs.h"

#include <base/math.h>
#include <engine/shared/config.h>

#include <algorithm>
#include <cmath>

namespace
{
	float FastModeOffsetTicks()
	{
		if(g_Config.m_BclInputs != BC_INPUTS_FAST || g_Config.m_TcFastInputAmount <= 0)
			return 0.0f;
		return g_Config.m_TcFastInputAmount / 20.0f;
	}

	float BestModeOffsetTicks()
	{
		if(g_Config.m_BclInputs != BC_INPUTS_BEST || g_Config.m_BclBestInputAmount <= 0)
			return 0.0f;

		float Offset = g_Config.m_BclBestInputAmount / 100.0f;

		if(g_Config.m_BclBestInputSmoothing > 0)
			Offset *= 1.0f - (g_Config.m_BclBestInputSmoothing / 200.0f);

		if(g_Config.m_BclBestInputLatencyComp > 0)
			Offset *= 1.0f + (g_Config.m_BclBestInputLatencyComp / 100.0f);

		return Offset;
	}

	float SaikoModeOffsetTicks()
	{
		if(g_Config.m_BclInputs != BC_INPUTS_SAIKO || g_Config.m_BclSaikoInputAmount <= 0)
			return 0.0f;
		return g_Config.m_BclSaikoInputAmount / 100.0f;
	}

	float DeltaModeOffsetTicks()
	{
		if(g_Config.m_BclInputs != BC_INPUTS_DELTA || g_Config.m_BclDeltaInputAmount <= 0)
			return 0.0f;
		return g_Config.m_BclDeltaInputAmount / 100.0f;
	}

	float FModeOffsetTicks()
	{
		if(g_Config.m_BclInputs != BC_INPUTS_F || g_Config.m_BclFInputAmount <= 0)
			return 0.0f;
		return g_Config.m_BclFInputAmount / 1000.0f;
	}
} // namespace

float BclInputs::EffectiveOffsetTicks()
{
	switch(g_Config.m_BclInputs)
	{
	case BC_INPUTS_FAST: return FastModeOffsetTicks();
	case BC_INPUTS_BEST: return BestModeOffsetTicks();
	case BC_INPUTS_SAIKO: return SaikoModeOffsetTicks();
	case BC_INPUTS_DELTA: return DeltaModeOffsetTicks();
	case BC_INPUTS_F: return FModeOffsetTicks();
	default: return 0.0f;
	}
}

int BclInputs::PredictionTicks(float OffsetTicks)
{
	if(OffsetTicks <= 0.0f)
		return 0;
	return (int)std::ceil(OffsetTicks);
}

int BclInputs::PredictionTicksOthers(float OffsetTicks)
{
	return PredictionTicks(OffsetTicks);
}

void BclInputs::ApplyOffset(float OffsetTicks, int &Tick, float &Intra)
{
	if(OffsetTicks <= 0.0f)
		return;

	const int WholeTicks = (int)OffsetTicks;
	const float OffsetIntra = OffsetTicks - WholeTicks;

	const float CombinedIntra = Intra + OffsetIntra;
	const int CarryOverTicks = (int)CombinedIntra;

	Tick += WholeTicks + CarryOverTicks;
	Intra = CombinedIntra - CarryOverTicks;
}

float BclInputs::BestInterpolationAmount(float Fraction, float DeltaLength, bool Enable)
{
	if(!Enable)
		return Fraction;

	const float T = std::clamp(Fraction, 0.0f, 1.0f);
	const float T2 = T * T;
	const float CubicT = 3.0f * T2 - 2.0f * T2 * T;
	switch(std::clamp(g_Config.m_BclBestInputInterpolation, 1, 3))
	{
	case 2:
		return CubicT;
	case 3:
		return mix(T, CubicT, std::clamp(DeltaLength / 1000.0f, 0.0f, 1.0f));
	default:
		return T;
	}
}

vec2 BclInputs::BestInterpolate(vec2 PrevPos, vec2 CurPos, float Fraction, bool Enable)
{
	return mix(PrevPos, CurPos, BestInterpolationAmount(Fraction, length(CurPos - PrevPos), Enable));
}

float BclInputs::BestInterpolate(float PrevPos, float CurPos, float Fraction, bool Enable)
{
	return mix(PrevPos, CurPos, BestInterpolationAmount(Fraction, absolute(CurPos - PrevPos), Enable));
}

bool BclInputs::FastOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_FAST && g_Config.m_TcFastInputOthers != 0;
}

bool BclInputs::BestOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_BEST && g_Config.m_BclBestInputOthers != 0;
}

bool BclInputs::SaikoOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_SAIKO && g_Config.m_BclSaikoInputOthers != 0;
}

bool BclInputs::DeltaOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_DELTA && g_Config.m_BclDeltaInputOthers != 0;
}

bool BclInputs::FOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_F && g_Config.m_BclFInputOthers != 0;
}

bool BclInputs::CloudOthers()
{
	return g_Config.m_BclInputs == BC_INPUTS_CLOUD && g_Config.m_BclCloudInputOthers != 0;
}

bool BclInputs::AnyOthers()
{
	return FastOthers() || BestOthers() || SaikoOthers() || DeltaOthers() || FOthers() || CloudOthers();
}

bool BclInputs::ImmediateOthers()
{
	return BestOthers() || SaikoOthers() || DeltaOthers();
}
