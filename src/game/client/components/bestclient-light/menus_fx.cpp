/* Copyright © 2026 BestProject Team */
#include <algorithm>

#include <engine/font_icons.h>
#include <engine/shared/config.h>

#include <game/client/bc_menu_badges.h>
#include <game/client/bc_ui_animations.h>
#include <game/client/components/bestclient-light/fx/fgf_weather.h>
#include <game/client/components/bestclient-light/fx/hookgrip.h>
#include <game/client/components/bestclient-light/settings_search.h>
#include <game/client/components/bestclient-light/ui_theme/style.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

void CMenus::RenderSettingsBestClientLightFx(CUIRect &Column, bool HookColumn)
{
	const float LineSize = 20.0f;
	const float MarginSmall = 5.0f;
	const float HeadlineFontSize = 20.0f;
	const float MarginBetweenViews = 30.0f;
	const float BlockPadding = MarginBetweenViews * 0.6666f;
	const float Row = MarginSmall + LineSize;

	const auto RowCheck = [&](CUIRect &View, int *pValue, const char *pText) {
		View.HSplitTop(MarginSmall, nullptr, &View);
		CUIRect Line;
		View.HSplitTop(LineSize, &Line, &View);
		DoButton_CheckBoxAutoVMarginAndSet(pValue, pText, pValue, &Line, LineSize);
	};
	const auto RowSlider = [&](CUIRect &View, int *pValue, const char *pText, int Min, int Max, const char *pSuffix) {
		View.HSplitTop(MarginSmall, nullptr, &View);
		CUIRect Line;
		View.HSplitTop(LineSize, &Line, &View);
		Ui()->DoScrollbarOption(pValue, pValue, &Line, pText, Min, Max, &CUi::ms_LinearScrollbarScale, CUi::SCROLLBAR_OPTION_DELAYUPDATE, pSuffix);
	};
	const auto RowColor = [&](CUIRect &View, CButtonContainer *pResetId, const char *pText, unsigned *pColor, unsigned DefaultColor) {
		View.HSplitTop(MarginSmall, nullptr, &View);
		DoLine_ColorPicker(pResetId, LineSize, 13.0f, 0.0f, &View, pText, pColor, color_cast<ColorRGBA>(ColorHSLA(DefaultColor)), false);
	};
	const auto RowDropDown = [&](CUIRect &View, const char *pLabel, int &Value, const char **pItems, int Num, CUi::SDropDownState &State, CScrollRegion &Scroll) {
		View.HSplitTop(MarginSmall, nullptr, &View);
		CUIRect Line, Label, Select;
		View.HSplitTop(LineSize, &Line, &View);
		Line.VSplitLeft(110.0f, &Label, &Select);
		Ui()->DoLabel(&Label, pLabel, 14.0f, TEXTALIGN_ML);
		State.m_SelectionPopupContext.m_pScrollRegion = &Scroll;
		Value = Ui()->DoDropDown(&Select, Value, pItems, Num, State);
	};
	const auto DrawFxTitle = [&](CUIRect Title, const char *pText, CButtonContainer *pAuthor) {
		CUIRect AuthorBadge;
		Title.VSplitRight(6.0f, &Title, nullptr);
		BcMenuBadges::DrawAuthor(Graphics(), Ui(), TextRender(), &Title, 4.0f, &AuthorBadge);
		Ui()->DoButtonLogic(pAuthor, 0, &AuthorBadge, BUTTONFLAG_NONE);
		GameClient()->m_Tooltips.DoToolTip(pAuthor, &AuthorBadge, "recidivismnational");
		GameClient()->m_Tooltips.SetFadeTime(pAuthor, 0.0f);
		Ui()->DoLabel(&Title, pText, HeadlineFontSize, TEXTALIGN_ML);
	};

	if(HookColumn)
	{
		int StyleSettingRows = 0;
		switch(g_Config.m_BclHookStyle)
		{
		case FGF_HOOK_STYLE_WHIRLWIND: StyleSettingRows = 9; break;
		case FGF_HOOK_STYLE_BLACK_HOLE:
		case FGF_HOOK_STYLE_MAGIC:
		case FGF_HOOK_STYLE_FIRE:
		case FGF_HOOK_STYLE_RAINBOW: StyleSettingRows = 4; break;
		case FGF_HOOK_STYLE_LIGHTNING:
		case FGF_HOOK_STYLE_TENTACLE: StyleSettingRows = 5; break;
		default: break;
		}
		const bool StyleOn = g_Config.m_BclHookStyle != FGF_HOOK_STYLE_OFF;
		int DetailRows = 0;
		if(StyleOn)
		{
			DetailRows = 4;
			if(StyleSettingRows > 0)
				DetailRows += StyleSettingRows;
		}
		const float ExpandedTarget = (1.0f + DetailRows) * Row;
		const float BlockHeight = LineSize + ExpandedTarget;
		CUIRect Block;
		Column.HSplitTop(BlockHeight, &Block, &Column);
		CUIRect BlockBg = Block;
		BlockBg.w += BlockPadding;
		BlockBg.h += BlockPadding;
		BlockBg.x -= BlockPadding * 0.5f;
		BlockBg.y -= BlockPadding * 0.5f;
		BestClientLightUiTheme::DrawUiBlock(&BlockBg, IGraphics::CORNER_ALL, 10.0f);
		BestClientLightSettingsSearch::DrawBlockHighlight(&g_Config.m_BclHookStyle, &BlockBg);

		CUIRect Label;
		Block.HSplitTop(LineSize, &Label, &Block);
		CUIRect Title, ResetButton;
		Label.VSplitRight(LineSize + 8.0f, &Title, &ResetButton);
		static CButtonContainer s_Reset;
		if(Ui()->DoButton_FontIcon(&s_Reset, FontIcon::ARROW_ROTATE_LEFT, 0, &ResetButton, BUTTONFLAG_LEFT))
		{
			g_Config.m_BclHookStyle = DefaultConfig::BclHookStyle;
			g_Config.m_BclHookRope = DefaultConfig::BclHookRope;
			g_Config.m_BclHookTip = DefaultConfig::BclHookTip;
			g_Config.m_BclHookChargeTime = DefaultConfig::BclHookChargeTime;
			g_Config.m_BclHookSize = DefaultConfig::BclHookSize;
			g_Config.m_BclHookWindColor = DefaultConfig::BclHookWindColor;
			g_Config.m_BclHookWindBurst = DefaultConfig::BclHookWindBurst;
			g_Config.m_BclHookWindEye = DefaultConfig::BclHookWindEye;
			g_Config.m_BclHookWindDensity = DefaultConfig::BclHookWindDensity;
			g_Config.m_BclHookWindSpin = DefaultConfig::BclHookWindSpin;
			g_Config.m_BclHookWindThickness = DefaultConfig::BclHookWindThickness;
			g_Config.m_BclHookWindOpacity = DefaultConfig::BclHookWindOpacity;
			g_Config.m_BclHookWindLeafAmount = DefaultConfig::BclHookWindLeafAmount;
			g_Config.m_BclHookWindDust = DefaultConfig::BclHookWindDust;
			g_Config.m_BclHookHoleColor = DefaultConfig::BclHookHoleColor;
			g_Config.m_BclHookHoleDisk = DefaultConfig::BclHookHoleDisk;
			g_Config.m_BclHookHoleStreaks = DefaultConfig::BclHookHoleStreaks;
			g_Config.m_BclHookHoleSpin = DefaultConfig::BclHookHoleSpin;
			g_Config.m_BclHookMagicColor = DefaultConfig::BclHookMagicColor;
			g_Config.m_BclHookMagicRunes = DefaultConfig::BclHookMagicRunes;
			g_Config.m_BclHookMagicSpin = DefaultConfig::BclHookMagicSpin;
			g_Config.m_BclHookMagicSparkles = DefaultConfig::BclHookMagicSparkles;
			g_Config.m_BclHookRainbowSpeed = DefaultConfig::BclHookRainbowSpeed;
			g_Config.m_BclHookRainbowGlitter = DefaultConfig::BclHookRainbowGlitter;
			g_Config.m_BclHookRainbowRays = DefaultConfig::BclHookRainbowRays;
			g_Config.m_BclHookRainbowWidth = DefaultConfig::BclHookRainbowWidth;
			g_Config.m_BclHookFireColor = DefaultConfig::BclHookFireColor;
			g_Config.m_BclHookFireSmoke = DefaultConfig::BclHookFireSmoke;
			g_Config.m_BclHookFireIntensity = DefaultConfig::BclHookFireIntensity;
			g_Config.m_BclHookFireEmbers = DefaultConfig::BclHookFireEmbers;
			g_Config.m_BclHookBoltColor = DefaultConfig::BclHookBoltColor;
			g_Config.m_BclHookBoltJitter = DefaultConfig::BclHookBoltJitter;
			g_Config.m_BclHookBoltBranches = DefaultConfig::BclHookBoltBranches;
			g_Config.m_BclHookBoltSparks = DefaultConfig::BclHookBoltSparks;
			g_Config.m_BclHookBoltArc = DefaultConfig::BclHookBoltArc;
			g_Config.m_BclHookTentacleColor = DefaultConfig::BclHookTentacleColor;
			g_Config.m_BclHookTentacleWaves = DefaultConfig::BclHookTentacleWaves;
			g_Config.m_BclHookTentacleThickness = DefaultConfig::BclHookTentacleThickness;
			g_Config.m_BclHookTentacleSuckers = DefaultConfig::BclHookTentacleSuckers;
			g_Config.m_BclHookTentacleSlime = DefaultConfig::BclHookTentacleSlime;
		}
		GameClient()->m_Tooltips.DoToolTip(&s_Reset, &ResetButton, BcLocalize("Reset to defaults"));
		static CButtonContainer s_HooksAuthor;
		DrawFxTitle(Title, BcLocalize("Custom hooks"), &s_HooksAuthor);

		CUIRect Visible = Block;
		Visible.h = ExpandedTarget;
		Ui()->ClipEnable(&Visible);
		static CUi::SDropDownState s_StyleState;
		static CScrollRegion s_StyleScroll;
		const char *apStyles[] = {
			BcLocalize("None"),
			BcLocalize("Whirlwind"),
			BcLocalize("Black hole"),
			BcLocalize("Magic"),
			BcLocalize("Fire"),
			BcLocalize("Troll"),
			BcLocalize("Rainbow"),
			BcLocalize("Lightning"),
			BcLocalize("Tentacle"),
		};
		RowDropDown(Block, BcLocalize("Hook"), g_Config.m_BclHookStyle, apStyles, (int)std::size(apStyles), s_StyleState, s_StyleScroll);

		if(StyleOn)
		{
			RowCheck(Block, &g_Config.m_BclHookRope, BcLocalize("Style the hook"));
			RowCheck(Block, &g_Config.m_BclHookTip, BcLocalize("Style the tip"));
			RowSlider(Block, &g_Config.m_BclHookChargeTime, BcLocalize("Grow time"), 200, 4000, "ms");
			RowSlider(Block, &g_Config.m_BclHookSize, BcLocalize("Size"), 50, 200, "%");

			if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_WHIRLWIND)
			{
				RowCheck(Block, &g_Config.m_BclHookWindBurst, BcLocalize("Grab gust"));
				RowCheck(Block, &g_Config.m_BclHookWindEye, BcLocalize("Eye"));
				RowSlider(Block, &g_Config.m_BclHookWindDensity, BcLocalize("Wind lines"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookWindSpin, BcLocalize("Spin speed"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookWindThickness, BcLocalize("Line thickness"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookWindOpacity, BcLocalize("Wind opacity"), 10, 100, "%");
				RowSlider(Block, &g_Config.m_BclHookWindLeafAmount, BcLocalize("Leaves"), 0, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookWindDust, BcLocalize("Dust"), 0, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Wind color"), &g_Config.m_BclHookWindColor, DefaultConfig::BclHookWindColor);
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_BLACK_HOLE)
			{
				RowCheck(Block, &g_Config.m_BclHookHoleDisk, BcLocalize("Disk"));
				RowSlider(Block, &g_Config.m_BclHookHoleStreaks, BcLocalize("Matter"), 0, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookHoleSpin, BcLocalize("Spin"), 25, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Disk color"), &g_Config.m_BclHookHoleColor, DefaultConfig::BclHookHoleColor);
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_MAGIC)
			{
				RowCheck(Block, &g_Config.m_BclHookMagicRunes, BcLocalize("Runes"));
				RowSlider(Block, &g_Config.m_BclHookMagicSpin, BcLocalize("Spin"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookMagicSparkles, BcLocalize("Sparkles"), 0, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Spell color"), &g_Config.m_BclHookMagicColor, DefaultConfig::BclHookMagicColor);
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_FIRE)
			{
				RowCheck(Block, &g_Config.m_BclHookFireSmoke, BcLocalize("Smoke"));
				RowSlider(Block, &g_Config.m_BclHookFireIntensity, BcLocalize("Flames"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookFireEmbers, BcLocalize("Embers"), 0, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Fire color"), &g_Config.m_BclHookFireColor, DefaultConfig::BclHookFireColor);
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_RAINBOW)
			{
				RowCheck(Block, &g_Config.m_BclHookRainbowRays, BcLocalize("Rays"));
				RowSlider(Block, &g_Config.m_BclHookRainbowSpeed, BcLocalize("Flow"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookRainbowGlitter, BcLocalize("Glitter"), 0, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookRainbowWidth, BcLocalize("Ribbon width"), 50, 250, "%");
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_LIGHTNING)
			{
				RowCheck(Block, &g_Config.m_BclHookBoltArc, BcLocalize("Wall arcs"));
				RowSlider(Block, &g_Config.m_BclHookBoltJitter, BcLocalize("Jitter"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookBoltBranches, BcLocalize("Forks"), 0, 6, "");
				RowSlider(Block, &g_Config.m_BclHookBoltSparks, BcLocalize("Sparks"), 0, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Lightning color"), &g_Config.m_BclHookBoltColor, DefaultConfig::BclHookBoltColor);
			}
			else if(g_Config.m_BclHookStyle == FGF_HOOK_STYLE_TENTACLE)
			{
				RowCheck(Block, &g_Config.m_BclHookTentacleSuckers, BcLocalize("Suckers"));
				RowSlider(Block, &g_Config.m_BclHookTentacleWaves, BcLocalize("Writhing"), 25, 300, "%");
				RowSlider(Block, &g_Config.m_BclHookTentacleThickness, BcLocalize("Thickness"), 50, 250, "%");
				RowSlider(Block, &g_Config.m_BclHookTentacleSlime, BcLocalize("Slime"), 0, 300, "%");
				static CButtonContainer s_Color;
				RowColor(Block, &s_Color, BcLocalize("Flesh color"), &g_Config.m_BclHookTentacleColor, DefaultConfig::BclHookTentacleColor);
			}
		}
		Ui()->ClipDisable();

		Column.HSplitTop(MarginBetweenViews, nullptr, &Column);

		const float HoldTarget = 3.0f * Row;
		const float HoldHeight = BC_MODULE_REVEAL(g_Config.m_BclGatheredEnergy != 0, HoldTarget, Client()->RenderFrameTime());
		const float EffectRows = 5.0f + (g_Config.m_BclAirJump != 0 ? 1.0f : 0.0f) + (g_Config.m_BclHammerSlash != 0 ? 1.0f : 0.0f);
		const float EffectsBlockHeight = LineSize + EffectRows * Row + HoldHeight;
		CUIRect EffectsBlock;
		Column.HSplitTop(EffectsBlockHeight, &EffectsBlock, &Column);
		CUIRect EffectsBg = EffectsBlock;
		EffectsBg.w += BlockPadding;
		EffectsBg.h += BlockPadding;
		EffectsBg.x -= BlockPadding * 0.5f;
		EffectsBg.y -= BlockPadding * 0.5f;
		BestClientLightUiTheme::DrawUiBlock(&EffectsBg, IGraphics::CORNER_ALL, 10.0f);
		BestClientLightSettingsSearch::DrawBlockHighlight(&g_Config.m_BclHookTether, &EffectsBg);

		CUIRect EffectsLabel;
		EffectsBlock.HSplitTop(LineSize, &EffectsLabel, &EffectsBlock);
		CUIRect EffectsTitle, EffectsResetButton;
		EffectsLabel.VSplitRight(LineSize + 8.0f, &EffectsTitle, &EffectsResetButton);
		static CButtonContainer s_EffectsReset;
		if(Ui()->DoButton_FontIcon(&s_EffectsReset, FontIcon::ARROW_ROTATE_LEFT, 0, &EffectsResetButton, BUTTONFLAG_LEFT))
		{
			g_Config.m_BclHookTether = DefaultConfig::BclHookTether;
			g_Config.m_BclHookLassoBox = DefaultConfig::BclHookLassoBox;
			g_Config.m_BclAirJump = DefaultConfig::BclAirJump;
			g_Config.m_BclAirJumpHideParticles = DefaultConfig::BclAirJumpHideParticles;
			g_Config.m_BclHammerSlash = DefaultConfig::BclHammerSlash;
			g_Config.m_BclHammerHideParticles = DefaultConfig::BclHammerHideParticles;
			g_Config.m_BclGatheredEnergyAura = DefaultConfig::BclGatheredEnergyAura;
			g_Config.m_BclGatheredEnergyBurst = DefaultConfig::BclGatheredEnergyBurst;
			g_Config.m_BclGatheredEnergyAmount = DefaultConfig::BclGatheredEnergyAmount;
		}
		GameClient()->m_Tooltips.DoToolTip(&s_EffectsReset, &EffectsResetButton, BcLocalize("Reset to defaults"));
		static CButtonContainer s_EffectsAuthor;
		DrawFxTitle(EffectsTitle, BcLocalize("Hook effects"), &s_EffectsAuthor);
		RowCheck(EffectsBlock, &g_Config.m_BclAirJump, BcLocalize("Air jump ring"));
		if(g_Config.m_BclAirJump)
			RowCheck(EffectsBlock, &g_Config.m_BclAirJumpHideParticles, BcLocalize("Disable jump particles"));
		RowCheck(EffectsBlock, &g_Config.m_BclHammerSlash, BcLocalize("Hammer slash"));
		if(g_Config.m_BclHammerSlash)
			RowCheck(EffectsBlock, &g_Config.m_BclHammerHideParticles, BcLocalize("Disable hammer particles"));
		RowCheck(EffectsBlock, &g_Config.m_BclHookTether, BcLocalize("Hook lasso"));
		RowCheck(EffectsBlock, &g_Config.m_BclHookLassoBox, BcLocalize("Hook box"));
		RowCheck(EffectsBlock, &g_Config.m_BclGatheredEnergy, BcLocalize("Hook hold"));
		if(HoldHeight > 0.5f)
		{
			CUIRect VisibleEffects = EffectsBlock;
			VisibleEffects.h = HoldHeight;
			Ui()->ClipEnable(&VisibleEffects);
			RowCheck(EffectsBlock, &g_Config.m_BclGatheredEnergyAura, BcLocalize("Hold aura"));
			RowCheck(EffectsBlock, &g_Config.m_BclGatheredEnergyBurst, BcLocalize("Hold burst"));
			RowSlider(EffectsBlock, &g_Config.m_BclGatheredEnergyAmount, BcLocalize("Energy motes"), 25, 300, "%");
			Ui()->ClipDisable();
		}

		Column.HSplitTop(MarginBetweenViews, nullptr, &Column);

		{
			const bool Expanded = g_Config.m_BclAtmosphere != 0;
			float WeatherExpandedRows = 5.0f;
			if(g_Config.m_BclWeather == FGF_WEATHER_SNOW || g_Config.m_BclWeather == FGF_WEATHER_RAIN)
				WeatherExpandedRows = 6.0f;
			const float WeatherExpandedTarget = WeatherExpandedRows * Row;
			const float ExpandedHeight = BC_MODULE_REVEAL(Expanded, WeatherExpandedTarget, Client()->RenderFrameTime());
			const float WeatherBlockHeight = LineSize + MarginSmall + LineSize + ExpandedHeight;
			CUIRect WeatherBlock;
			Column.HSplitTop(WeatherBlockHeight, &WeatherBlock, &Column);
			CUIRect WeatherBg = WeatherBlock;
			WeatherBg.w += BlockPadding;
			WeatherBg.h += BlockPadding;
			WeatherBg.x -= BlockPadding * 0.5f;
			WeatherBg.y -= BlockPadding * 0.5f;
			BestClientLightUiTheme::DrawUiBlock(&WeatherBg, IGraphics::CORNER_ALL, 10.0f);
			BestClientLightSettingsSearch::DrawBlockHighlight(&g_Config.m_BclAtmosphere, &WeatherBg);

			CUIRect WeatherLabel;
			WeatherBlock.HSplitTop(LineSize, &WeatherLabel, &WeatherBlock);
			CUIRect WeatherTitle, WeatherResetButton;
			WeatherLabel.VSplitRight(LineSize + 8.0f, &WeatherTitle, &WeatherResetButton);
			static CButtonContainer s_WeatherReset;
			if(Ui()->DoButton_FontIcon(&s_WeatherReset, FontIcon::ARROW_ROTATE_LEFT, 0, &WeatherResetButton, BUTTONFLAG_LEFT))
			{
				g_Config.m_BclWeather = DefaultConfig::BclWeather;
				g_Config.m_BclWeatherAmount = DefaultConfig::BclWeatherAmount;
				g_Config.m_BclWeatherSnowSize = DefaultConfig::BclWeatherSnowSize;
				g_Config.m_BclWeatherSnowSpeed = DefaultConfig::BclWeatherSnowSpeed;
				g_Config.m_BclWeatherSnowColor = DefaultConfig::BclWeatherSnowColor;
				g_Config.m_BclWeatherSnowflakes = DefaultConfig::BclWeatherSnowflakes;
				g_Config.m_BclWeatherRainSize = DefaultConfig::BclWeatherRainSize;
				g_Config.m_BclWeatherRainSpeed = DefaultConfig::BclWeatherRainSpeed;
				g_Config.m_BclWeatherRainTilt = DefaultConfig::BclWeatherRainTilt;
				g_Config.m_BclWeatherRainColor = DefaultConfig::BclWeatherRainColor;
				g_Config.m_BclWeatherFireflySize = DefaultConfig::BclWeatherFireflySize;
				g_Config.m_BclWeatherFireflySpeed = DefaultConfig::BclWeatherFireflySpeed;
				g_Config.m_BclWeatherFireflyColor = DefaultConfig::BclWeatherFireflyColor;
				g_Config.m_BclAmbientDust = DefaultConfig::BclAmbientDust;
				g_Config.m_BclAmbientDustAmount = DefaultConfig::BclAmbientDustAmount;
				g_Config.m_BclAmbientDustColor = DefaultConfig::BclAmbientDustColor;
				g_Config.m_BclWeatherDustSize = DefaultConfig::BclWeatherDustSize;
				g_Config.m_BclWeatherDustSpeed = DefaultConfig::BclWeatherDustSpeed;
			}
			GameClient()->m_Tooltips.DoToolTip(&s_WeatherReset, &WeatherResetButton, BcLocalize("Reset to defaults"));
			static CButtonContainer s_WeatherAuthor;
			DrawFxTitle(WeatherTitle, BcLocalize("Weather"), &s_WeatherAuthor);
			WeatherBlock.HSplitTop(MarginSmall, nullptr, &WeatherBlock);
			CUIRect Enable;
			WeatherBlock.HSplitTop(LineSize, &Enable, &WeatherBlock);
			DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_BclAtmosphere, BcLocalize("Enable"), &g_Config.m_BclAtmosphere, &Enable, LineSize);
			if(ExpandedHeight > 0.5f)
			{
				CUIRect WeatherVisible = WeatherBlock;
				WeatherVisible.h = ExpandedHeight;
				Ui()->ClipEnable(&WeatherVisible);
				static CUi::SDropDownState s_WeatherState;
				static CScrollRegion s_WeatherScroll;
				const char *apWeather[] = {
					BcLocalize("Snow"),
					BcLocalize("Rain"),
					BcLocalize("Fireflies"),
					BcLocalize("Dust"),
				};
				g_Config.m_BclWeather = std::clamp(g_Config.m_BclWeather, 0, (int)FGF_WEATHER_DUST);
				RowDropDown(WeatherBlock, BcLocalize("Weather"), g_Config.m_BclWeather, apWeather, (int)std::size(apWeather), s_WeatherState, s_WeatherScroll);
				if(g_Config.m_BclWeather == FGF_WEATHER_DUST)
				{
					RowSlider(WeatherBlock, &g_Config.m_BclAmbientDustAmount, BcLocalize("Amount"), 25, 200, "%");
					RowSlider(WeatherBlock, &g_Config.m_BclWeatherDustSize, BcLocalize("Size"), 25, 300, "%");
					RowSlider(WeatherBlock, &g_Config.m_BclWeatherDustSpeed, BcLocalize("Speed"), 25, 300, "%");
					static CButtonContainer s_DustColor;
					RowColor(WeatherBlock, &s_DustColor, BcLocalize("Dust color"), &g_Config.m_BclAmbientDustColor, DefaultConfig::BclAmbientDustColor);
				}
				else
				{
					RowSlider(WeatherBlock, &g_Config.m_BclWeatherAmount, BcLocalize("Amount"), 25, 300, "%");
					if(g_Config.m_BclWeather == FGF_WEATHER_RAIN)
					{
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherRainSize, BcLocalize("Size"), 25, 300, "%");
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherRainSpeed, BcLocalize("Speed"), 25, 300, "%");
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherRainTilt, BcLocalize("Tilt"), -75, 75, "°");
						static CButtonContainer s_RainColor;
						RowColor(WeatherBlock, &s_RainColor, BcLocalize("Rain color"), &g_Config.m_BclWeatherRainColor, DefaultConfig::BclWeatherRainColor);
					}
					else if(g_Config.m_BclWeather == FGF_WEATHER_FIREFLIES)
					{
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherFireflySize, BcLocalize("Size"), 25, 300, "%");
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherFireflySpeed, BcLocalize("Speed"), 25, 300, "%");
						static CButtonContainer s_FireflyColor;
						RowColor(WeatherBlock, &s_FireflyColor, BcLocalize("Firefly color"), &g_Config.m_BclWeatherFireflyColor, DefaultConfig::BclWeatherFireflyColor);
					}
					else
					{
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherSnowSize, BcLocalize("Size"), 25, 300, "%");
						RowSlider(WeatherBlock, &g_Config.m_BclWeatherSnowSpeed, BcLocalize("Speed"), 25, 300, "%");
						static CUi::SDropDownState s_SnowState;
						static CScrollRegion s_SnowScroll;
						const char *apSnow[] = {
							BcLocalize("Dots"),
							BcLocalize("Snowflakes"),
						};
						g_Config.m_BclWeatherSnowflakes = std::clamp(g_Config.m_BclWeatherSnowflakes, 0, 1);
						RowDropDown(WeatherBlock, BcLocalize("Snow"), g_Config.m_BclWeatherSnowflakes, apSnow, (int)std::size(apSnow), s_SnowState, s_SnowScroll);
						static CButtonContainer s_SnowColor;
						RowColor(WeatherBlock, &s_SnowColor, BcLocalize("Snow color"), &g_Config.m_BclWeatherSnowColor, DefaultConfig::BclWeatherSnowColor);
					}
				}
				Ui()->ClipDisable();
			}
		}

		Column.HSplitTop(MarginBetweenViews, nullptr, &Column);

	{
		const bool Expanded = g_Config.m_BclFreezePull != 0;
		const bool ShowColdColor = g_Config.m_BclFreezePullHookColor == 0;
		const float PullExpandedTarget = (5.0f + (ShowColdColor ? 1.0f : 0.0f)) * Row;
		const float ExpandedHeight = BC_MODULE_REVEAL(Expanded, PullExpandedTarget, Client()->RenderFrameTime());
		const float PullBlockHeight = LineSize + MarginSmall + LineSize + ExpandedHeight;
		CUIRect PullBlock;
		Column.HSplitTop(PullBlockHeight, &PullBlock, &Column);
		CUIRect PullBg = PullBlock;
		PullBg.w += BlockPadding;
		PullBg.h += BlockPadding;
		PullBg.x -= BlockPadding * 0.5f;
		PullBg.y -= BlockPadding * 0.5f;
		BestClientLightUiTheme::DrawUiBlock(&PullBg, IGraphics::CORNER_ALL, 10.0f);
		BestClientLightSettingsSearch::DrawBlockHighlight(&g_Config.m_BclFreezePull, &PullBg);

		CUIRect PullLabel;
		PullBlock.HSplitTop(LineSize, &PullLabel, &PullBlock);
		CUIRect PullTitle, PullResetButton;
		PullLabel.VSplitRight(LineSize + 8.0f, &PullTitle, &PullResetButton);
		static CButtonContainer s_PullReset;
		if(Ui()->DoButton_FontIcon(&s_PullReset, FontIcon::ARROW_ROTATE_LEFT, 0, &PullResetButton, BUTTONFLAG_LEFT))
		{
			g_Config.m_BclFreezePullShield = DefaultConfig::BclFreezePullShield;
			g_Config.m_BclFreezePullRange = DefaultConfig::BclFreezePullRange;
			g_Config.m_BclFreezePullStrands = DefaultConfig::BclFreezePullStrands;
			g_Config.m_BclFreezePullSparks = DefaultConfig::BclFreezePullSparks;
			g_Config.m_BclFreezePullColor = DefaultConfig::BclFreezePullColor;
			g_Config.m_BclFreezePullHookColor = DefaultConfig::BclFreezePullHookColor;
		}
		GameClient()->m_Tooltips.DoToolTip(&s_PullReset, &PullResetButton, BcLocalize("Reset to defaults"));
		static CButtonContainer s_PullAuthor;
		DrawFxTitle(PullTitle, BcLocalize("Freeze lifesteal"), &s_PullAuthor);
		PullBlock.HSplitTop(MarginSmall, nullptr, &PullBlock);
		CUIRect Enable;
		PullBlock.HSplitTop(LineSize, &Enable, &PullBlock);
		DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_BclFreezePull, BcLocalize("Enable"), &g_Config.m_BclFreezePull, &Enable, LineSize);
		if(ExpandedHeight > 0.5f)
		{
			CUIRect PullVisible = PullBlock;
			PullVisible.h = ExpandedHeight;
			Ui()->ClipEnable(&PullVisible);
			RowCheck(PullBlock, &g_Config.m_BclFreezePullShield, BcLocalize("Shield"));
			RowSlider(PullBlock, &g_Config.m_BclFreezePullRange, BcLocalize("Reach"), 60, 260, "");
			RowSlider(PullBlock, &g_Config.m_BclFreezePullStrands, BcLocalize("Amount"), 1, 8, "");
			RowSlider(PullBlock, &g_Config.m_BclFreezePullSparks, BcLocalize("Frost"), 0, 300, "%");
			RowCheck(PullBlock, &g_Config.m_BclFreezePullHookColor, BcLocalize("Custom hook color"));
			if(ShowColdColor)
			{
				static CButtonContainer s_Color;
				RowColor(PullBlock, &s_Color, BcLocalize("Cold color"), &g_Config.m_BclFreezePullColor, DefaultConfig::BclFreezePullColor);
			}
			Ui()->ClipDisable();
		}
	}

	Column.HSplitTop(MarginBetweenViews, nullptr, &Column);

	{
		const bool Expanded = g_Config.m_BclFreezeLightning != 0;
		const float BoltExpandedTarget = 7.0f * Row;
		const float ExpandedHeight = BC_MODULE_REVEAL(Expanded, BoltExpandedTarget, Client()->RenderFrameTime());
		const float BoltBlockHeight = LineSize + MarginSmall + LineSize + ExpandedHeight;
		CUIRect BoltBlock;
		Column.HSplitTop(BoltBlockHeight, &BoltBlock, &Column);
		CUIRect BoltBg = BoltBlock;
		BoltBg.w += BlockPadding;
		BoltBg.h += BlockPadding;
		BoltBg.x -= BlockPadding * 0.5f;
		BoltBg.y -= BlockPadding * 0.5f;
		BestClientLightUiTheme::DrawUiBlock(&BoltBg, IGraphics::CORNER_ALL, 10.0f);
		BestClientLightSettingsSearch::DrawBlockHighlight(&g_Config.m_BclFreezeLightning, &BoltBg);

		CUIRect BoltLabel;
		BoltBlock.HSplitTop(LineSize, &BoltLabel, &BoltBlock);
		CUIRect BoltTitle, BoltResetButton;
		BoltLabel.VSplitRight(LineSize + 8.0f, &BoltTitle, &BoltResetButton);
		static CButtonContainer s_BoltReset;
		if(Ui()->DoButton_FontIcon(&s_BoltReset, FontIcon::ARROW_ROTATE_LEFT, 0, &BoltResetButton, BUTTONFLAG_LEFT))
		{
			g_Config.m_BclFreezeLightningImpact = DefaultConfig::BclFreezeLightningImpact;
			g_Config.m_BclFreezeLightningHeight = DefaultConfig::BclFreezeLightningHeight;
			g_Config.m_BclFreezeLightningThickness = DefaultConfig::BclFreezeLightningThickness;
			g_Config.m_BclFreezeLightningBranches = DefaultConfig::BclFreezeLightningBranches;
			g_Config.m_BclFreezeLightningShake = DefaultConfig::BclFreezeLightningShake;
			g_Config.m_BclFreezeLightningColor = DefaultConfig::BclFreezeLightningColor;
		}
		GameClient()->m_Tooltips.DoToolTip(&s_BoltReset, &BoltResetButton, BcLocalize("Reset to defaults"));
		static CButtonContainer s_LightningAuthor;
		DrawFxTitle(BoltTitle, BcLocalize("Freeze lightning"), &s_LightningAuthor);
		BoltBlock.HSplitTop(MarginSmall, nullptr, &BoltBlock);
		CUIRect Enable;
		BoltBlock.HSplitTop(LineSize, &Enable, &BoltBlock);
		DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_BclFreezeLightning, BcLocalize("Enable"), &g_Config.m_BclFreezeLightning, &Enable, LineSize);

		if(ExpandedHeight > 0.5f)
		{
			CUIRect BoltVisible = BoltBlock;
			BoltVisible.h = ExpandedHeight;
			Ui()->ClipEnable(&BoltVisible);
			RowCheck(BoltBlock, &g_Config.m_BclFreezeLightningImpact, BcLocalize("Impact"));
			RowSlider(BoltBlock, &g_Config.m_BclFreezeLightningHeight, BcLocalize("Height"), 100, 900, "");
			RowSlider(BoltBlock, &g_Config.m_BclFreezeLightningThickness, BcLocalize("Thickness"), 50, 200, "%");
			RowSlider(BoltBlock, &g_Config.m_BclFreezeLightningBranches, BcLocalize("Branches"), 0, 8, "");
			RowSlider(BoltBlock, &g_Config.m_BclFreezeLightningShake, BcLocalize("Shake"), 0, 100, "%");
			static CButtonContainer s_Color;
			RowColor(BoltBlock, &s_Color, BcLocalize("Color"), &g_Config.m_BclFreezeLightningColor, DefaultConfig::BclFreezeLightningColor);
			BoltBlock.HSplitTop(MarginSmall, nullptr, &BoltBlock);
			CUIRect Preview;
			BoltBlock.HSplitTop(LineSize, &Preview, &BoltBlock);
			if(GameClient()->m_Snap.m_pLocalCharacter != nullptr)
			{
				static CButtonContainer s_Strike;
				if(DoButton_Menu(&s_Strike, BcLocalize("Strike me"), 0, &Preview))
				{
					GameClient()->m_Lightning.Strike(GameClient()->m_LocalCharacterPos, 1.0f, true);
					if(g_Config.m_BclFreezeLightningShake > 0)
						GameClient()->m_Lightning.AddShake(g_Config.m_BclFreezeLightningShake / 100.0f * 14.0f);
				}
			}
			else
				Ui()->DoLabel(&Preview, BcLocalize("Join a server to preview the lightning"), 13.0f, TEXTALIGN_ML);
			Ui()->ClipDisable();
		}
	}

		Column.HSplitTop(MarginBetweenViews, nullptr, &Column);
	}
}
