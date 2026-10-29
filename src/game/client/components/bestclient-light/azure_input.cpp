/* Copyright © 2026 BestProject Team */
#include "azure_input.h"

#include <base/math.h>

#include <engine/shared/config.h>

#include <game/client/components/controls.h>
#include <game/client/gameclient.h>

#include <cmath>

bool CAzureInput::IsActive() const
{
	return g_Config.m_BclInputs == BC_INPUTS_AZURE && Amount() > 0.0f;
}

float CAzureInput::Amount() const
{
	return g_Config.m_BclAzureInputAmount / 100.0f;
}

bool CAzureInput::Others() const
{
	return IsActive() && g_Config.m_BclAzureInputOthers != 0;
}

int CAzureInput::SelfTickOffset() const
{
	if(Amount() <= 0.0f)
		return 0;
	return (int)std::ceil(Amount() + 1.0f);
}

int CAzureInput::OthersTickOffset() const
{
	if(!Others() || Amount() <= 0.0f)
		return 0;
	return (int)std::ceil(Amount());
}

CNetObj_PlayerInput &CAzureInput::Input(int Dummy)
{
	return m_aInput[Dummy];
}

const CNetObj_PlayerInput &CAzureInput::Input(int Dummy) const
{
	return m_aInput[Dummy];
}

bool CAzureInput::CheckNewInput(const CControls &Controls)
{
	bool NewInput[2] = {};
	for(int Dummy = 0; Dummy < NUM_DUMMIES; Dummy++)
	{
		CNetObj_PlayerInput NextInput = Controls.m_aInputData[Dummy];
		if(Dummy == g_Config.m_ClDummy)
		{
			const bool LeftPressed = Controls.m_aInputDirectionLeft[Dummy] != 0;
			const bool RightPressed = Controls.m_aInputDirectionRight[Dummy] != 0;
			NextInput.m_Direction = GameClient()->m_SnapTap.ResolveMovementDirection(Dummy, LeftPressed, RightPressed, false);
			if(NextInput.m_Direction == 0 && Controls.m_aInputDebounce[Dummy].m_Direction)
				NextInput.m_Direction = Controls.m_aInputDebounce[Dummy].m_Direction;
		}

		if(m_aInput[Dummy].m_Direction != NextInput.m_Direction)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_Hook != NextInput.m_Hook)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_Fire != NextInput.m_Fire)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_Jump != NextInput.m_Jump)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_NextWeapon != NextInput.m_NextWeapon)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_PrevWeapon != NextInput.m_PrevWeapon)
			NewInput[Dummy] = true;
		if(m_aInput[Dummy].m_WantedWeapon != NextInput.m_WantedWeapon)
			NewInput[Dummy] = true;

		if(Dummy == g_Config.m_ClDummy && g_Config.m_ClSubTickAiming)
		{
			NextInput.m_TargetX = (int)Controls.m_aMousePos[Dummy].x;
			NextInput.m_TargetY = (int)Controls.m_aMousePos[Dummy].y;
		}

		m_aInput[Dummy] = NextInput;
	}

	return NewInput[0] || NewInput[1];
}

void CAzureInput::ApplyOffset(float OffsetTicks, int &Tick, float &Intra) const
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

void CAzureInput::ApplySampleOffset(const CGameClient &GameClient, int ClientId, int &Tick, float &Intra) const
{
	const bool Local = GameClient.IsFastInputLocalClient(ClientId);
	const int Ticks = Local ? SelfTickOffset() : OthersTickOffset();
	if(Local || Ticks > 0)
		ApplyOffset(Amount(), Tick, Intra);
}

bool CAzureInput::TryGetPredPos(const CGameClient &GameClient, int ClientId, int Tick, float Intra, int MaxTick, vec2 &OutPos) const
{
	if(!IsActive() || Tick <= 0)
		return false;
	if(GameClient.m_aClients[ClientId].m_aPredTick[(Tick - 1) % 200] < GameClient.Client()->PrevGameTick(g_Config.m_ClDummy) ||
		GameClient.m_aClients[ClientId].m_aPredTick[Tick % 200] > MaxTick)
		return false;

	OutPos = mix(GameClient.m_aClients[ClientId].m_aPredPos[(Tick - 1) % 200], GameClient.m_aClients[ClientId].m_aPredPos[Tick % 200], Intra);
	return true;
}

vec2 CAzureInput::FastInputPos(const CGameClient &GameClient, int ClientId) const
{
	int PredTick = GameClient.Client()->PredGameTick(g_Config.m_ClDummy);
	float PredIntra = GameClient.Client()->PredIntraGameTick(g_Config.m_ClDummy);
	vec2 Pos = mix(GameClient.m_aClients[ClientId].m_PrevPredicted.m_Pos, GameClient.m_aClients[ClientId].m_Predicted.m_Pos, PredIntra);
	const bool Local = GameClient.IsFastInputLocalClient(ClientId);
	const int FastInputTicks = Local ? SelfTickOffset() : OthersTickOffset();
	if(!(Local || FastInputTicks > 0))
		return Pos;

	ApplyOffset(Amount(), PredTick, PredIntra);
	TryGetPredPos(GameClient, ClientId, PredTick, PredIntra, GameClient.Client()->PredGameTick(g_Config.m_ClDummy) + FastInputTicks, Pos);
	return Pos;
}
