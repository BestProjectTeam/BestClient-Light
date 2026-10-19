/* Copyright © 2026 BestProject Team */
#include <algorithm>

#include <engine/font_icons.h>
#include <engine/shared/config.h>

#include <game/client/bc_menu_badges.h>
#include <game/client/bc_ui_animations.h>
#include <game/client/components/bestclient-light/fx/fgf_weather.h>
#include <game/client/components/bestclient-light/settings_search.h>
#include <game/client/components/bestclient-light/ui_theme/style.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

void CMenus::RenderSettingsBestClientLightFx(CUIRect &Column)
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
}
