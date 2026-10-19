/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_UI_THEME_STYLE_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_UI_THEME_STYLE_H

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/ui_rect.h>

namespace BestClientLightUiTheme
{

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

inline void DrawUiBlock(const CUIRect *pRect, int Corners = IGraphics::CORNER_ALL, float Rounding = 10.0f)
{
	pRect->Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.25f), Corners, Rounding);
}

} // namespace BestClientLightUiTheme

#endif
