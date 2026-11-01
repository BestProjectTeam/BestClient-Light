/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "damageind.h"

#include <base/color.h>

#include <engine/demo.h>
#include <engine/graphics.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/client/gameclient.h>

CDamageInd::CDamageInd()
{
	m_NumItems = 0;
}

void CDamageInd::Create(vec2 Pos, vec2 Dir, float Alpha)
{
	if(m_NumItems >= MAX_ITEMS)
		return;

	CItem *pItem = &m_aItems[m_NumItems];
	pItem->m_Pos = Pos;
	pItem->m_Dir = -Dir;
	pItem->m_RemainingLife = 0.75f;
	pItem->m_StartAngle = -random_angle();
	pItem->m_Color = ColorRGBA(1.0f, 1.0f, 1.0f, Alpha);
	++m_NumItems;
}

// bestclient-light
void CDamageInd::RenderForMinimap(vec2 ScreenOrigin, vec2 WorldMin, vec2 WorldMax, float Scale, float Alpha) const
{
	if(m_NumItems <= 0 || Alpha <= 0.0f || Scale <= 0.0f || !GameClient()->m_GameSkin.m_aSpriteStars[0].IsValid())
		return;
	if(GameClient()->OptimizerDisableParticles() || (g_Config.m_ClFocusMode && g_Config.m_ClFocusModeHideEffects))
		return;

	float ScaleX = 1.0f;
	float ScaleY = 1.0f;
	Graphics()->GetSpriteScale(SPRITE_STAR1, ScaleX, ScaleY);
	const float Width = 48.0f * ScaleX * Scale;
	const float Height = 48.0f * ScaleY * Scale;
	const float Margin = 80.0f;
	Graphics()->TextureSet(GameClient()->m_GameSkin.m_aSpriteStars[0]);
	Graphics()->QuadsBegin();
	for(int i = 0; i < m_NumItems; ++i)
	{
		const CItem &Item = m_aItems[i];
		if(Item.m_RemainingLife <= 0.0f)
			continue;
		if(Item.m_Pos.x < WorldMin.x - Margin || Item.m_Pos.x > WorldMax.x + Margin || Item.m_Pos.y < WorldMin.y - Margin || Item.m_Pos.y > WorldMax.y + Margin)
			continue;
		const vec2 Pos = mix(Item.m_Pos + Item.m_Dir * 75.0f, Item.m_Pos, std::clamp((Item.m_RemainingLife - 0.60f) / 0.15f, 0.0f, 1.0f));
		const float LifeAlpha = std::min(Item.m_RemainingLife / 0.1f, 1.0f) * Item.m_Color.a * Alpha;
		if(LifeAlpha <= 0.01f)
			continue;
		const vec2 Dot = ScreenOrigin + (Pos - WorldMin) * Scale;
		Graphics()->QuadsSetSubset(0.0f, 0.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(Item.m_StartAngle + Item.m_RemainingLife * 2.0f);
		Graphics()->SetColor(Item.m_Color.r, Item.m_Color.g, Item.m_Color.b, LifeAlpha);
		Graphics()->DrawSprite(Dot.x, Dot.y, Width, Height);
	}
	Graphics()->QuadsEnd();
	Graphics()->QuadsSetRotation(0.0f);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}
// bestclient-light

void CDamageInd::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	static float s_LastLocalTime = LocalTime();
	const float LifeAdjustment = (LocalTime() - s_LastLocalTime) * GameClient()->GetAnimationPlaybackSpeed();
	s_LastLocalTime = LocalTime();

	Graphics()->TextureSet(GameClient()->m_GameSkin.m_aSpriteStars[0]);
	for(int i = 0; i < m_NumItems;)
	{
		m_aItems[i].m_RemainingLife -= LifeAdjustment;
		if(m_aItems[i].m_RemainingLife < 0.0f)
		{
			--m_NumItems;
			m_aItems[i] = m_aItems[m_NumItems];
		}
		else
		{
			// bestclient-light
			if(!GameClient()->OptimizerAllowRenderPos(m_aItems[i].m_Pos))
			{
				i++;
				continue;
			}
			// bestclient-light

			vec2 Pos = mix(m_aItems[i].m_Pos + m_aItems[i].m_Dir * 75.0f, m_aItems[i].m_Pos, std::clamp((m_aItems[i].m_RemainingLife - 0.60f) / 0.15f, 0.0f, 1.0f));
			const float LifeAlpha = m_aItems[i].m_RemainingLife / 0.1f;
			Graphics()->SetColor(m_aItems[i].m_Color.WithMultipliedAlpha(LifeAlpha));
			Graphics()->QuadsSetRotation(m_aItems[i].m_StartAngle + m_aItems[i].m_RemainingLife * 2.0f);
			Graphics()->RenderQuadContainerAsSprite(m_DmgIndQuadContainerIndex, 0, Pos.x, Pos.y);
			i++;
		}
	}

	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);
}

void CDamageInd::OnInit()
{
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);

	m_DmgIndQuadContainerIndex = Graphics()->CreateQuadContainer(false);
	float ScaleX, ScaleY;
	Graphics()->GetSpriteScale(SPRITE_STAR1, ScaleX, ScaleY);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_DmgIndQuadContainerIndex, 48.f * ScaleX, 48.f * ScaleY);
	Graphics()->QuadContainerUpload(m_DmgIndQuadContainerIndex);
}

void CDamageInd::OnReset()
{
	m_NumItems = 0;
}
