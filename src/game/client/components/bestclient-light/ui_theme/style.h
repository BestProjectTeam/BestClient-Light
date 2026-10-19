/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_UI_THEME_STYLE_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_UI_THEME_STYLE_H

#include <base/color.h>
#include <base/math.h>
#include <base/str.h>
#include <base/time.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/ui_rect.h>

#include <cmath>
#include <vector>

namespace BestClientLightUiTheme
{

enum
{
	UI_THEME_ELEM_TEXT = 0,
	UI_THEME_ELEM_CHECKBOX,
	UI_THEME_ELEM_SCROLLBAR,
	UI_THEME_ELEM_BUTTON,
	UI_THEME_ELEM_DROP,
	UI_THEME_ELEM_EDIT,
	UI_THEME_ELEM_BLOCK,
	UI_THEME_ELEM_PANEL,
	UI_THEME_ELEM_NAV,
	UI_THEME_ELEM_BADGE,
	UI_THEME_ELEM_BIND,
	UI_THEME_ELEM_NUM,
};

inline bool &MenuThemeScopeRef()
{
	static bool s_Active = false;
	return s_Active;
}

inline bool IsMenuThemeScope()
{
	return MenuThemeScopeRef();
}

struct CMenuThemeScope
{
	bool m_Prev;
	explicit CMenuThemeScope(bool Enable = true)
	{
		m_Prev = MenuThemeScopeRef();
		MenuThemeScopeRef() = Enable;
	}
	~CMenuThemeScope()
	{
		MenuThemeScopeRef() = m_Prev;
	}
	CMenuThemeScope(const CMenuThemeScope &) = delete;
	CMenuThemeScope &operator=(const CMenuThemeScope &) = delete;
};

inline bool IsNewStyle()
{
	return g_Config.m_BclMenuUiStyle != 0;
}

inline bool IsNewCheckbox()
{
	return IsNewStyle() && g_Config.m_BclMenuUiNewCheckbox != 0;
}

inline bool IsNewScrollbar()
{
	return IsNewStyle() && g_Config.m_BclMenuUiNewScrollbar != 0;
}

inline bool IsNewTabOption()
{
	return IsNewStyle() && g_Config.m_BclMenuUiNewTabOption != 0;
}

inline bool IsSettingsNavOnLeft()
{
	return IsNewTabOption() && g_Config.m_BclMenuUiTabSide == 0;
}

inline bool IsNewColorPicker()
{
	return IsNewStyle() && g_Config.m_BclMenuUiNewColorPicker != 0;
}

inline bool IsUiThemes()
{
	return g_Config.m_BclUiThemes != 0 && IsMenuThemeScope();
}

inline bool IsCustomUiColor()
{
	return IsUiThemes();
}

inline bool IsCustomUiBackground()
{
	return g_Config.m_BclUiThemes != 0 && g_Config.m_BclCustomUiBackground != 0;
}

inline bool CatOn(int Flag)
{
	return IsCustomUiColor() && Flag != 0;
}

inline int TextAnimateSpeed() { return g_Config.m_BclCustomUiTextAnimateSpeed; }
inline int CheckboxAnimateSpeed() { return g_Config.m_BclCustomUiCheckboxAnimateSpeed; }
inline int ScrollbarAnimateSpeed() { return g_Config.m_BclCustomUiScrollbarAnimateSpeed; }
inline int ButtonAnimateSpeed() { return g_Config.m_BclCustomUiButtonAnimateSpeed; }
inline int DropdownAnimateSpeed() { return g_Config.m_BclCustomUiDropdownAnimateSpeed; }
inline int EditBoxAnimateSpeed() { return g_Config.m_BclCustomUiEditBoxAnimateSpeed; }
inline int BlockAnimateSpeed() { return g_Config.m_BclCustomUiBlockAnimateSpeed; }
inline int PanelAnimateSpeed() { return g_Config.m_BclCustomUiMainPanelAnimateSpeed; }
inline int NavAnimateSpeed() { return g_Config.m_BclCustomUiNavTabAnimateSpeed; }
inline int BadgeAnimateSpeed() { return g_Config.m_BclCustomUiBadgeAnimateSpeed; }
inline int BindAnimateSpeed() { return g_Config.m_BclCustomUiBindAnimateSpeed; }

inline int ThemeBlockOpacity()
{
	return IsCustomUiColor() ? g_Config.m_BclCustomUiBlockOpacity : 25;
}

inline int ThemePanelOpacity()
{
	return g_Config.m_BclCustomUiMainPanelOpacity;
}

inline int ThemeNavOpacity()
{
	return g_Config.m_BclCustomUiNavTabOpacity;
}

inline bool IsCustomUiText() { return CatOn(g_Config.m_BclCustomUiText); }
inline bool IsCustomUiCheckbox() { return CatOn(g_Config.m_BclCustomUiCheckbox); }
inline bool IsCustomUiScrollbar() { return CatOn(g_Config.m_BclCustomUiScrollbar); }
inline bool IsCustomUiButton() { return CatOn(g_Config.m_BclCustomUiButton); }
inline bool IsCustomUiDropdown() { return CatOn(g_Config.m_BclCustomUiDropdown); }
inline bool IsCustomUiEditBox() { return CatOn(g_Config.m_BclCustomUiEditBox); }
inline bool IsCustomUiBlock() { return CatOn(g_Config.m_BclCustomUiBlock); }
inline bool IsCustomUiMainPanel() { return CatOn(g_Config.m_BclCustomUiMainPanel); }
inline bool IsCustomUiNavTab() { return CatOn(g_Config.m_BclCustomUiNavTab); }
inline bool IsCustomUiBadge() { return CatOn(g_Config.m_BclCustomUiBadge); }
inline bool IsCustomUiBind() { return CatOn(g_Config.m_BclCustomUiBind); }

inline bool TextGrad() { return CatOn(g_Config.m_BclCustomUiTextGradient); }
inline bool CheckboxGrad() { return CatOn(g_Config.m_BclCustomUiCheckboxGradient); }
inline bool ScrollbarGrad() { return CatOn(g_Config.m_BclCustomUiScrollbarGradient); }
inline bool ButtonGrad() { return CatOn(g_Config.m_BclCustomUiButtonGradient); }
inline bool DropdownGrad() { return CatOn(g_Config.m_BclCustomUiDropdownGradient); }
inline bool EditBoxGrad() { return CatOn(g_Config.m_BclCustomUiEditBoxGradient); }
inline bool BlockGrad() { return CatOn(g_Config.m_BclCustomUiBlockGradient); }
inline bool PanelGrad() { return CatOn(g_Config.m_BclCustomUiMainPanelGradient); }
inline bool NavGrad() { return CatOn(g_Config.m_BclCustomUiNavTabGradient); }
inline bool BadgeGrad() { return CatOn(g_Config.m_BclCustomUiBadgeGradient); }
inline bool BindGrad() { return CatOn(g_Config.m_BclCustomUiBindGradient); }

inline bool TextAccent() { return CatOn(g_Config.m_BclCustomUiTextUseAccent); }
inline bool CheckboxAccent() { return CatOn(g_Config.m_BclCustomUiCheckboxUseAccent); }
inline bool ScrollbarAccent() { return CatOn(g_Config.m_BclCustomUiScrollbarUseAccent); }
inline bool ButtonAccent() { return CatOn(g_Config.m_BclCustomUiButtonUseAccent); }
inline bool DropdownAccent() { return CatOn(g_Config.m_BclCustomUiDropdownUseAccent); }
inline bool EditBoxAccent() { return CatOn(g_Config.m_BclCustomUiEditBoxUseAccent); }
inline bool BlockAccent() { return CatOn(g_Config.m_BclCustomUiBlockUseAccent); }
inline bool PanelAccent() { return CatOn(g_Config.m_BclCustomUiMainPanelUseAccent); }
inline bool NavAccent() { return CatOn(g_Config.m_BclCustomUiNavTabUseAccent); }
inline bool BadgeAccent() { return CatOn(g_Config.m_BclCustomUiBadgeUseAccent); }
inline bool BindAccent() { return CatOn(g_Config.m_BclCustomUiBindUseAccent); }

inline unsigned TextColor() { return g_Config.m_BclCustomUiTextColor; }
inline unsigned TextGradColor() { return g_Config.m_BclCustomUiTextGradientColor; }
inline unsigned TextAccentColor() { return g_Config.m_BclCustomUiTextAccentColor; }
inline unsigned CheckboxColor() { return g_Config.m_BclCustomUiCheckboxColor; }
inline unsigned CheckboxGradColor() { return g_Config.m_BclCustomUiCheckboxGradientColor; }
inline unsigned CheckboxAccentColor() { return g_Config.m_BclCustomUiCheckboxAccentColor; }
inline unsigned ScrollbarColor() { return g_Config.m_BclCustomUiScrollbarColor; }
inline unsigned ScrollbarGradColor() { return g_Config.m_BclCustomUiScrollbarGradientColor; }
inline unsigned ScrollbarAccentColor() { return g_Config.m_BclCustomUiScrollbarAccentColor; }
inline unsigned ButtonColor() { return g_Config.m_BclCustomUiButtonColor; }
inline unsigned ButtonGradColor() { return g_Config.m_BclCustomUiButtonGradientColor; }
inline unsigned ButtonAccentColor() { return g_Config.m_BclCustomUiButtonAccentColor; }
inline unsigned DropdownColor() { return g_Config.m_BclCustomUiDropdownColor; }
inline unsigned DropdownGradColor() { return g_Config.m_BclCustomUiDropdownGradientColor; }
inline unsigned DropdownAccentColor() { return g_Config.m_BclCustomUiDropdownAccentColor; }
inline unsigned EditBoxColor() { return g_Config.m_BclCustomUiEditBoxColor; }
inline unsigned EditBoxGradColor() { return g_Config.m_BclCustomUiEditBoxGradientColor; }
inline unsigned EditBoxAccentColor() { return g_Config.m_BclCustomUiEditBoxAccentColor; }
inline unsigned BlockColor() { return g_Config.m_BclCustomUiBlockColor; }
inline unsigned BlockGradColor() { return g_Config.m_BclCustomUiBlockGradientColor; }
inline unsigned BlockAccentColor() { return g_Config.m_BclCustomUiBlockAccentColor; }
inline unsigned PanelColor() { return g_Config.m_BclCustomUiMainPanelColor; }
inline unsigned PanelGradColor() { return g_Config.m_BclCustomUiMainPanelGradientColor; }
inline unsigned PanelAccentColor() { return g_Config.m_BclCustomUiMainPanelAccentColor; }
inline unsigned NavColor() { return g_Config.m_BclCustomUiNavTabColor; }
inline unsigned NavGradColor() { return g_Config.m_BclCustomUiNavTabGradientColor; }
inline unsigned NavAccentColor() { return g_Config.m_BclCustomUiNavTabAccentColor; }
inline unsigned BadgeColor() { return g_Config.m_BclCustomUiBadgeColor; }
inline unsigned BadgeGradColor() { return g_Config.m_BclCustomUiBadgeGradientColor; }
inline unsigned BadgeAccentColor() { return g_Config.m_BclCustomUiBadgeAccentColor; }
inline unsigned BindColor() { return g_Config.m_BclCustomUiBindColor; }
inline unsigned BindGradColor() { return g_Config.m_BclCustomUiBindGradientColor; }
inline unsigned BindAccentColor() { return g_Config.m_BclCustomUiBindAccentColor; }

inline bool IsCustomUiTextGradient()
{
	return IsCustomUiText() && TextGrad() && !TextAccent();
}

inline bool IsCustomUiBadgeGradient()
{
	return IsCustomUiBadge() && BadgeGrad();
}

inline unsigned PickChromeColor(unsigned BaseColor, unsigned AccentColor, bool AccentState, bool UseAccent)
{
	return (UseAccent && AccentState) ? AccentColor : BaseColor;
}

inline float UiGradientPhase(int Speed)
{
	if(Speed <= 0)
		return 0.0f;
	const double T = (double)time_get() / (double)time_freq();
	return (float)std::fmod(T * (Speed / 100.0), 1.0);
}

inline ColorRGBA SampleTwoStopColor(unsigned Color1, unsigned Color2, float Position)
{
	const ColorRGBA A = color_cast<ColorRGBA>(ColorHSLA(Color1, true));
	const ColorRGBA B = color_cast<ColorRGBA>(ColorHSLA(Color2, true));
	float Wrapped = std::fmod(Position, 1.0f);
	if(Wrapped < 0.0f)
		Wrapped += 1.0f;
	const float Scaled = Wrapped * 2.0f;
	const int Index = (int)Scaled % 2;
	const float LocalT = Scaled - std::floor(Scaled);
	const ColorRGBA &From = Index == 0 ? A : B;
	const ColorRGBA &To = Index == 0 ? B : A;
	return ColorRGBA(
		From.r + LocalT * (To.r - From.r),
		From.g + LocalT * (To.g - From.g),
		From.b + LocalT * (To.b - From.b),
		From.a + LocalT * (To.a - From.a));
}

inline void SampleWaveCorners(unsigned Color1, unsigned Color2, float Alpha, float Lum, ColorRGBA &TL, ColorRGBA &TR, ColorRGBA &BL, ColorRGBA &BR, int AnimateSpeed, bool Vertical = false)
{
	const float Phase = UiGradientPhase(AnimateSpeed);
	auto At = [&](float U, float V) {
		const ColorRGBA C = SampleTwoStopColor(Color1, Color2, (Vertical ? V : U) - Phase);
		return ColorRGBA(C.r * Lum, C.g * Lum, C.b * Lum, Alpha * C.a);
	};
	TL = At(0.0f, 0.0f);
	TR = At(1.0f, 0.0f);
	BL = At(0.0f, 1.0f);
	BR = At(1.0f, 1.0f);
}

inline ColorRGBA TintAchromatic(ColorRGBA Color, unsigned ConfigColor, bool Enabled)
{
	if(!Enabled)
		return Color;
	if(absolute(Color.r - Color.g) > 0.02f || absolute(Color.g - Color.b) > 0.02f)
		return Color;
	const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(ConfigColor, true));
	const float Lum = Color.r;
	return ColorRGBA(C.r * Lum, C.g * Lum, C.b * Lum, Color.a * C.a);
}

inline void DrawChromeRect(const CUIRect *pRect, ColorRGBA BaseTint, unsigned CfgBase, unsigned CfgGrad, bool UseGrad, unsigned CfgAccent, bool AccentState, bool UseAccent, bool Enabled, int Corners, float Rounding, int AnimateSpeed, bool VerticalGrad = false)
{
	if(!Enabled)
	{
		pRect->Draw(BaseTint, Corners, Rounding);
		return;
	}
	if(UseAccent && AccentState)
	{
		pRect->Draw(TintAchromatic(BaseTint, CfgAccent, true), Corners, Rounding);
		return;
	}
	if(UseGrad)
	{
		float Lum = 1.0f;
		if(absolute(BaseTint.r - BaseTint.g) <= 0.02f && absolute(BaseTint.g - BaseTint.b) <= 0.02f)
			Lum = BaseTint.r;
		ColorRGBA TL, TR, BL, BR;
		SampleWaveCorners(CfgBase, CfgGrad, BaseTint.a, Lum, TL, TR, BL, BR, AnimateSpeed, VerticalGrad);
		pRect->Draw4(TL, TR, BL, BR, Corners, Rounding);
		return;
	}
	pRect->Draw(TintAchromatic(BaseTint, CfgBase, true), Corners, Rounding);
}

inline ColorRGBA UiText(ColorRGBA Color = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f), bool AccentState = false)
{
	if(IsCustomUiTextGradient() && !AccentState)
		return Color;
	return TintAchromatic(Color, PickChromeColor(TextColor(), TextAccentColor(), AccentState, TextAccent()), IsCustomUiText());
}

inline std::vector<STextColorSplit> BuildUiTextGradientSplits(const char *pText)
{
	std::vector<STextColorSplit> vSplits;
	if(pText == nullptr || !IsCustomUiTextGradient())
		return vSplits;

	size_t Size, Count;
	str_utf8_stats(pText, str_length(pText) + 1, SIZE_MAX, &Size, &Count);
	if(Count == 0)
		return vSplits;

	const float Phase = UiGradientPhase(TextAnimateSpeed());
	const unsigned C1 = TextColor();
	const unsigned C2 = TextGradColor();
	vSplits.reserve(Count);
	const char *pStr = pText;
	for(size_t i = 0; i < Count; i++)
	{
		const int ByteOffset = (int)(pStr - pText);
		const char *pPrev = pStr;
		str_utf8_decode(&pStr);
		const int ByteLen = (int)(pStr - pPrev);
		const float T = Count == 1 ? 0.5f : (float)i / (float)(Count - 1);
		const float Shifted = std::fmod(T - Phase + 1.0f, 1.0f);
		vSplits.emplace_back(ByteOffset, ByteLen, SampleTwoStopColor(C1, C2, Shifted));
	}
	return vSplits;
}

inline void DrawUiCheckbox(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding)
{
	DrawChromeRect(pRect, Color, CheckboxColor(), CheckboxGradColor(), CheckboxGrad(), CheckboxAccentColor(), AccentState, CheckboxAccent(), IsCustomUiCheckbox(), Corners, Rounding, CheckboxAnimateSpeed());
}

inline ColorRGBA UiScrollbar(ColorRGBA Color, bool AccentState = false)
{
	return TintAchromatic(Color, PickChromeColor(ScrollbarColor(), ScrollbarAccentColor(), AccentState, ScrollbarAccent()), IsCustomUiScrollbar());
}

inline void DrawUiScrollbar(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding, bool VerticalGrad = false)
{
	DrawChromeRect(pRect, Color, ScrollbarColor(), ScrollbarGradColor(), ScrollbarGrad(), ScrollbarAccentColor(), AccentState, ScrollbarAccent(), IsCustomUiScrollbar(), Corners, Rounding, ScrollbarAnimateSpeed(), VerticalGrad);
}

inline ColorRGBA UiButton(float Alpha = 1.0f, bool AccentState = false)
{
	return TintAchromatic(ColorRGBA(1.0f, 1.0f, 1.0f, Alpha), PickChromeColor(ButtonColor(), ButtonAccentColor(), AccentState, ButtonAccent()), IsCustomUiButton());
}

inline ColorRGBA UiButton(ColorRGBA Color, bool AccentState = false)
{
	return TintAchromatic(Color, PickChromeColor(ButtonColor(), ButtonAccentColor(), AccentState, ButtonAccent()), IsCustomUiButton());
}

inline void DrawUiButton(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding)
{
	DrawChromeRect(pRect, Color, ButtonColor(), ButtonGradColor(), ButtonGrad(), ButtonAccentColor(), AccentState, ButtonAccent(), IsCustomUiButton(), Corners, Rounding, ButtonAnimateSpeed());
}

inline ColorRGBA UiDropdown(float Alpha = 1.0f, bool AccentState = false)
{
	return TintAchromatic(ColorRGBA(1.0f, 1.0f, 1.0f, Alpha), PickChromeColor(DropdownColor(), DropdownAccentColor(), AccentState, DropdownAccent()), IsCustomUiDropdown());
}

inline ColorRGBA UiDropdown(ColorRGBA Color, bool AccentState = false)
{
	return TintAchromatic(Color, PickChromeColor(DropdownColor(), DropdownAccentColor(), AccentState, DropdownAccent()), IsCustomUiDropdown());
}

inline void DrawUiDropdown(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding)
{
	DrawChromeRect(pRect, Color, DropdownColor(), DropdownGradColor(), DropdownGrad(), DropdownAccentColor(), AccentState, DropdownAccent(), IsCustomUiDropdown(), Corners, Rounding, DropdownAnimateSpeed());
}

inline void DrawUiEditBox(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding)
{
	DrawChromeRect(pRect, Color, EditBoxColor(), EditBoxGradColor(), EditBoxGrad(), EditBoxAccentColor(), AccentState, EditBoxAccent(), IsCustomUiEditBox(), Corners, Rounding, EditBoxAnimateSpeed());
}

inline ColorRGBA UiBlockColor()
{
	const float Alpha = ThemeBlockOpacity() / 100.0f;
	if(IsCustomUiBlock() && (!BlockGrad() || BlockAccent()))
	{
		const unsigned ConfigColor = PickChromeColor(BlockColor(), BlockAccentColor(), false, BlockAccent());
		const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(ConfigColor, true));
		return ColorRGBA(C.r, C.g, C.b, Alpha * C.a);
	}
	if(!IsCustomUiColor())
		return ColorRGBA(0.0f, 0.0f, 0.0f, 0.25f);
	return ColorRGBA(0.0f, 0.0f, 0.0f, Alpha);
}

inline void DrawUiBlock(const CUIRect *pRect, int Corners = IGraphics::CORNER_ALL, float Rounding = 10.0f)
{
	const float Alpha = ThemeBlockOpacity() / 100.0f;
	if(IsCustomUiBlock() && BlockGrad() && !BlockAccent())
	{
		ColorRGBA TL, TR, BL, BR;
		SampleWaveCorners(BlockColor(), BlockGradColor(), Alpha, 1.0f, TL, TR, BL, BR, BlockAnimateSpeed());
		pRect->Draw4(TL, TR, BL, BR, Corners, Rounding);
		return;
	}
	pRect->Draw(UiBlockColor(), Corners, Rounding);
}

inline ColorRGBA UiMainPanel(ColorRGBA Base, bool AccentState = false)
{
	if(!IsCustomUiColor())
		return Base;
	const float OpacityScale = ThemePanelOpacity() / 50.0f;
	const bool UseAccent = PanelAccent();
	if(IsCustomUiMainPanel())
	{
		if(!(UseAccent && AccentState) && (absolute(Base.r - Base.g) > 0.02f || absolute(Base.g - Base.b) > 0.02f))
			return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
		if(PanelGrad() && !(UseAccent && AccentState))
			return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
		const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(PickChromeColor(PanelColor(), PanelAccentColor(), AccentState, UseAccent), true));
		return ColorRGBA(C.r, C.g, C.b, Base.a * C.a * OpacityScale);
	}
	return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
}

inline void DrawUiMainPanel(const CUIRect *pRect, ColorRGBA Base, bool AccentState, int Corners, float Rounding)
{
	if(IsCustomUiMainPanel() && AccentState && PanelAccent())
	{
		pRect->Draw(UiMainPanel(Base, true), Corners, Rounding);
		return;
	}
	if(IsCustomUiMainPanel() && PanelGrad() && !(PanelAccent() && AccentState))
	{
		const float OpacityScale = ThemePanelOpacity() / 50.0f;
		ColorRGBA TL, TR, BL, BR;
		SampleWaveCorners(PanelColor(), PanelGradColor(), Base.a * OpacityScale, 1.0f, TL, TR, BL, BR, PanelAnimateSpeed());
		pRect->Draw4(TL, TR, BL, BR, Corners, Rounding);
		return;
	}
	pRect->Draw(UiMainPanel(Base, AccentState), Corners, Rounding);
}

inline ColorRGBA UiNavTab(ColorRGBA Base, bool AccentState = false)
{
	if(!IsCustomUiColor())
		return Base;
	const float OpacityScale = ThemeNavOpacity() / 50.0f;
	const bool UseAccent = NavAccent();
	if(IsCustomUiNavTab())
	{
		if(!(UseAccent && AccentState) && (absolute(Base.r - Base.g) > 0.02f || absolute(Base.g - Base.b) > 0.02f))
			return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
		if(NavGrad() && !(UseAccent && AccentState))
			return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
		const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(PickChromeColor(NavColor(), NavAccentColor(), AccentState, UseAccent), true));
		return ColorRGBA(C.r, C.g, C.b, Base.a * C.a * OpacityScale);
	}
	return ColorRGBA(Base.r, Base.g, Base.b, Base.a * OpacityScale);
}

inline void DrawUiNavTab(const CUIRect *pRect, ColorRGBA Base, bool AccentState, int Corners, float Rounding)
{
	if(IsCustomUiNavTab() && AccentState && NavAccent())
	{
		pRect->Draw(UiNavTab(Base, true), Corners, Rounding);
		return;
	}
	if(IsCustomUiNavTab() && NavGrad() && !(NavAccent() && AccentState))
	{
		const float OpacityScale = ThemeNavOpacity() / 50.0f;
		ColorRGBA TL, TR, BL, BR;
		SampleWaveCorners(NavColor(), NavGradColor(), Base.a * OpacityScale, 1.0f, TL, TR, BL, BR, NavAnimateSpeed());
		pRect->Draw4(TL, TR, BL, BR, Corners, Rounding);
		return;
	}
	pRect->Draw(UiNavTab(Base, AccentState), Corners, Rounding);
}

inline void DrawUiBadge(const CUIRect *pRect, float LeftAlpha, float RightAlpha, float Rounding, bool AccentState = false)
{
	if(!IsCustomUiBadge())
		return;
	const unsigned Base = PickChromeColor(BadgeColor(), BadgeAccentColor(), AccentState, BadgeAccent());
	if(IsCustomUiBadgeGradient() && !(AccentState && BadgeAccent()))
	{
		ColorRGBA TL, TR, BL, BR;
		SampleWaveCorners(Base, BadgeGradColor(), 1.0f, 1.0f, TL, TR, BL, BR, BadgeAnimateSpeed());
		TL.a *= LeftAlpha;
		TR.a *= RightAlpha;
		BL.a *= LeftAlpha;
		BR.a *= RightAlpha;
		pRect->Draw4(TL, TR, BL, BR, IGraphics::CORNER_ALL, Rounding);
		return;
	}
	const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(Base, true));
	pRect->Draw4(
		ColorRGBA(C.r, C.g, C.b, LeftAlpha * C.a),
		ColorRGBA(C.r, C.g, C.b, RightAlpha * C.a),
		ColorRGBA(C.r, C.g, C.b, LeftAlpha * C.a),
		ColorRGBA(C.r, C.g, C.b, RightAlpha * C.a),
		IGraphics::CORNER_ALL, Rounding);
}

inline void DrawUiBind(const CUIRect *pRect, ColorRGBA Color, bool AccentState, int Corners, float Rounding)
{
	DrawChromeRect(pRect, Color, BindColor(), BindGradColor(), BindGrad(), BindAccentColor(), AccentState, BindAccent(), IsCustomUiBind(), Corners, Rounding, BindAnimateSpeed());
}

inline unsigned DarkenConfigColor(unsigned ConfigColor, float Factor = 0.65f)
{
	const ColorRGBA C = color_cast<ColorRGBA>(ColorHSLA(ConfigColor, true));
	return color_cast<ColorHSLA>(ColorRGBA(C.r * Factor, C.g * Factor, C.b * Factor, C.a)).Pack(true);
}

} // namespace BestClientLightUiTheme

class IStorage;
namespace BestClientLightUiTheme
{
inline constexpr const char *FILE_NAME = "ui_theme.cfg";
void AppendExport(char *pBuf, int Size, bool SkipDefaults);
void SaveFile(IStorage *pStorage);
void LoadFile(IStorage *pStorage);
} // namespace BestClientLightUiTheme

#endif
