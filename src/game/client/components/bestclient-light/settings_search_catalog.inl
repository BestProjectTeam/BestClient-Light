	{"Corner indicator", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclCornerIndicator, nullptr},
	{"Only in team", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorOnlyInTeam, &g_Config.m_BclCornerIndicator},
	{"Show alive teammates", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorShowAlive, &g_Config.m_BclCornerIndicator},
	{"Show frozen teammates", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorShowFrozen, &g_Config.m_BclCornerIndicator},
	{"Indicate last teammate", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorIndicateLastTeammate, &g_Config.m_BclCornerIndicator},
	{"Scale by distance", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorProximity, &g_Config.m_BclCornerIndicator},
	{"Ninja skin when frozen", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorNinjaFrozen, &g_Config.m_BclCornerIndicator},
	{"Alive size", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorSizeAlive, &g_Config.m_BclCornerIndicator},
	{"Frozen size", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorSizeFrozen, &g_Config.m_BclCornerIndicator},
	{"Last teammate size", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorLastTeammateSize, &g_Config.m_BclCornerIndicator},
	{"Use custom colors", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorCustomColors, &g_Config.m_BclCornerIndicator},
	{"Alive color", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorColorAlive, &g_Config.m_BclCornerIndicatorCustomColors},
	{"Frozen color", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorColorFrozen, &g_Config.m_BclCornerIndicatorCustomColors},
	{"Last teammate color", SETTINGS_NAV_LEAF_BC_VISUALS, "Corner indicator", &g_Config.m_BclCornerIndicatorLastTeammateColor, &g_Config.m_BclCornerIndicatorCustomColors},

	{"Chat Bubbles", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclChatBubbles, nullptr},
	{"Show chat bubbles above you", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubblesSelf, &g_Config.m_BclChatBubbles},
	{"Show chat bubbles in demo", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubblesDemo, &g_Config.m_BclChatBubbles},
	{"Chat bubble size", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleSize, &g_Config.m_BclChatBubbles},
	{"Show for", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleShowTime, &g_Config.m_BclChatBubbles},
	{"Fade in", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleFadeIn, &g_Config.m_BclChatBubbles},
	{"Fade out", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleFadeOut, &g_Config.m_BclChatBubbles},
	{"Stack animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleAnimation, &g_Config.m_BclChatBubbles},
	{"Corner rounding", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleRounding, &g_Config.m_BclChatBubbles},
	{"Custom colors", SETTINGS_NAV_LEAF_BC_VISUALS, "Chat Bubbles", &g_Config.m_BclChatBubbleCustomColors, &g_Config.m_BclChatBubbles},

	{"Media Background", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclMenuMediaBackground, nullptr},
	{"Enable to game background", SETTINGS_NAV_LEAF_BC_VISUALS, "Media Background", &g_Config.m_BclGameMediaBackground, nullptr},
	{"Map offset", SETTINGS_NAV_LEAF_BC_VISUALS, "Media Background", &g_Config.m_BclGameMediaBackgroundOffset, &g_Config.m_BclGameMediaBackground},

	{"Cursor Trail", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclCursorTrail, nullptr},
	{"Trail size", SETTINGS_NAV_LEAF_BC_VISUALS, "Cursor Trail", &g_Config.m_BclCursorTrailTrailSize, &g_Config.m_BclCursorTrail},
	{"Number of frames", SETTINGS_NAV_LEAF_BC_VISUALS, "Cursor Trail", &g_Config.m_BclCursorTrailNumberOfFrames, &g_Config.m_BclCursorTrail},
	{"Opacity", SETTINGS_NAV_LEAF_BC_VISUALS, "Cursor Trail", &g_Config.m_BclCursorTrailOpacity, &g_Config.m_BclCursorTrail},
	{"Sampling FPS", SETTINGS_NAV_LEAF_BC_VISUALS, "Cursor Trail", &g_Config.m_BclCursorTrailSamplingFps, &g_Config.m_BclCursorTrail},
	{"Disable movement", SETTINGS_NAV_LEAF_BC_VISUALS, "Cursor Trail", &g_Config.m_BclCursorTrailDisableMovement, &g_Config.m_BclCursorTrail},

	{"Motion Blur", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclMotionBlur, nullptr},
	{"Blend strength", SETTINGS_NAV_LEAF_BC_VISUALS, "Motion Blur", &g_Config.m_BclMotionBlurStrength, &g_Config.m_BclMotionBlur},

	{"Animations", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclAnimations, nullptr},
	{"Settings animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclModuleUiRevealAnimation, &g_Config.m_BclAnimations},
	{"Settings (ms)", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclModuleUiRevealAnimationMs, &g_Config.m_BclAnimations},
	{"Chat message animations", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatAnimation, &g_Config.m_BclAnimations},
	{"Chat message (ms)", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatAnimationMs, &g_Config.m_BclAnimations},
	{"Chat open animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatOpenAnimation, &g_Config.m_BclAnimations},
	{"Chat open (ms)", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatOpenAnimationMs, &g_Config.m_BclAnimations},
	{"Chat typing animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatTypingAnimation, &g_Config.m_BclAnimations},
	{"Chat typing (ms)", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclChatTypingAnimationMs, &g_Config.m_BclAnimations},
	{"Killfeed animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclKillfeedAnimation, &g_Config.m_BclAnimations},
	{"Killfeed (ms)", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclKillfeedAnimationMs, &g_Config.m_BclAnimations},
	{"Main menu animation", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclMainMenuAnimation, &g_Config.m_BclAnimations},
	{"Main menu speed", SETTINGS_NAV_LEAF_BC_VISUALS, "Animations", &g_Config.m_BclMainMenuAnimationSpeed, &g_Config.m_BclAnimations},

	{"Music Player", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclMusicPlayer, nullptr},
	{"Show song lyrics", SETTINGS_NAV_LEAF_BC_VISUALS, "Music Player", &g_Config.m_BclMusicPlayerShowLyrics, &g_Config.m_BclMusicPlayer},
	{"Show current time", SETTINGS_NAV_LEAF_BC_VISUALS, "Music Player", &g_Config.m_BclMusicPlayerShowCurrentTime, &g_Config.m_BclMusicPlayer},
	{"Text scale", SETTINGS_NAV_LEAF_BC_VISUALS, "Music Player", &g_Config.m_BclMusicPlayerTextScale, &g_Config.m_BclMusicPlayer},
	{"Columns", SETTINGS_NAV_LEAF_BC_VISUALS, "Music Player", &g_Config.m_BclMusicPlayerVisualizerColumns, &g_Config.m_BclMusicPlayer},
	{"Rounding", SETTINGS_NAV_LEAF_BC_VISUALS, "Music Player", &g_Config.m_BclMusicPlayerVisualizerRounding, &g_Config.m_BclMusicPlayer},

	{"Keystrokes", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclKeystrokesKeyboard, nullptr},

	{"Aspect Ratio", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclCustomAspectRatioMode, nullptr},
	{"Cursor as without stretch", SETTINGS_NAV_LEAF_BC_VISUALS, "Aspect Ratio", &g_Config.m_BclCustomAspectRatioCursorNative, nullptr},

	{"Weather", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals", &g_Config.m_BclAtmosphere, nullptr},
	{"Size", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherSnowSize, &g_Config.m_BclAtmosphere},
	{"Speed", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherSnowSpeed, &g_Config.m_BclAtmosphere},
	{"Size", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherRainSize, &g_Config.m_BclAtmosphere},
	{"Speed", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherRainSpeed, &g_Config.m_BclAtmosphere},
	{"Tilt", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherRainTilt, &g_Config.m_BclAtmosphere},
	{"Size", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherFireflySize, &g_Config.m_BclAtmosphere},
	{"Speed", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherFireflySpeed, &g_Config.m_BclAtmosphere},
	{"Size", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherDustSize, &g_Config.m_BclAtmosphere},
	{"Speed", SETTINGS_NAV_LEAF_BC_VISUALS, "Weather", &g_Config.m_BclWeatherDustSpeed, &g_Config.m_BclAtmosphere},

	{"Show coordinates above tees", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowTeeCoords, nullptr},
	{"Show real hitbox", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowRealHitbox, nullptr},
	{"Real hitbox dot color", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowRealHitboxColor, &g_Config.m_BclShowRealHitbox},
	{"Show player below indicator", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowhudDummyCoordIndicator, nullptr},
	{"Show current checkpoint", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowCorrectCheckpoint, nullptr},
	{"Show points in tab", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowPointsInTab, nullptr},
	{"Show spectator nicknames", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowSpectatorNames, nullptr},
	{"Show vote percentage", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclShowVotePercentage, nullptr},
	{"Display max FPS", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclDisplayMaxFps, nullptr},
	{"discord rpc (restart requires)", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclDiscordRPC, nullptr},
	{"Bind/Emote wheel scale", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclWheelScale, nullptr},
	{"Scoreboard scale", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_BclScoreboardScale, nullptr},
	{"UI scale", SETTINGS_NAV_LEAF_BC_VISUALS, "Visuals QoL", &g_Config.m_UiScale, nullptr},

	{"Better UI", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiStyle, nullptr},
	{"Enable better UI", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiStyle, nullptr},
	{"Better checkbox", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiNewCheckbox, &g_Config.m_BclMenuUiStyle},
	{"Better scrollbar", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiNewScrollbar, &g_Config.m_BclMenuUiStyle},
	{"Better tab option", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiNewTabOption, &g_Config.m_BclMenuUiStyle},
	{"Left", SETTINGS_NAV_LEAF_BC_VISUALS, "Better tab option", &g_Config.m_BclMenuUiTabSide, &g_Config.m_BclMenuUiNewTabOption},
	{"Right", SETTINGS_NAV_LEAF_BC_VISUALS, "Better tab option", &g_Config.m_BclMenuUiTabSide, &g_Config.m_BclMenuUiNewTabOption},
	{"Better color picker", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiNewColorPicker, &g_Config.m_BclMenuUiStyle},
	{"Better font", SETTINGS_NAV_LEAF_BC_VISUALS, "Better UI", &g_Config.m_BclMenuUiBetterFont, &g_Config.m_BclMenuUiStyle},

	{"Inputs", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclInputs, nullptr},
	{"Sub-Tick aiming", SETTINGS_NAV_LEAF_BC_TOOLS, "Inputs", &g_Config.m_ClSubTickAiming, nullptr},
	{"Auto margin", SETTINGS_NAV_LEAF_BC_TOOLS, "Inputs", &g_Config.m_BclAutoMargin, nullptr},

	{"Snap Tap", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclSnapTap, nullptr},
	{"Delay", SETTINGS_NAV_LEAF_BC_TOOLS, "Snap Tap", &g_Config.m_BclSnapTapDelay, &g_Config.m_BclSnapTap},

	{"Gores mode", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclGoresMode, nullptr},
	{"Disable if you have shotgun, grenade or laser", SETTINGS_NAV_LEAF_BC_TOOLS, "Gores mode", &g_Config.m_BclGoresModeDisableIfWeapons, &g_Config.m_BclGoresMode},

	{"Optimizer", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclOptimizer, nullptr},
	{"FPS fog", SETTINGS_NAV_LEAF_BC_TOOLS, "Optimizer", &g_Config.m_BclOptimizerFpsFog, &g_Config.m_BclOptimizer},

	{"Fast Actions", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclFastActions, nullptr},

	{"Finish Prediction", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclFinishPrediction, nullptr},
	{"Show time", SETTINGS_NAV_LEAF_BC_TOOLS, "Finish Prediction", &g_Config.m_BclFinishPredictionShowTime, &g_Config.m_BclFinishPrediction},
	{"Show percentage", SETTINGS_NAV_LEAF_BC_TOOLS, "Finish Prediction", &g_Config.m_BclFinishPredictionShowPercentage, &g_Config.m_BclFinishPrediction},
	{"Show always", SETTINGS_NAV_LEAF_BC_TOOLS, "Finish Prediction", &g_Config.m_BclFinishPredictionShowAlways, &g_Config.m_BclFinishPrediction},

	{"Edge Info", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_RiEdgeInfoCords, nullptr},
	{"Show edge info about jumps", SETTINGS_NAV_LEAF_BC_TOOLS, "Edge Info", &g_Config.m_RiEdgeInfoJump, nullptr},

	{"High DDNet Process Priority", SETTINGS_NAV_LEAF_BC_TOOLS, "Performance", &g_Config.m_BclHighProcessPriority, nullptr},
	{"Lower Discords Process Priority", SETTINGS_NAV_LEAF_BC_TOOLS, "Performance", &g_Config.m_BclDiscordNormalProcessPriority, nullptr},

	{"Automatic update", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclAutoUpdate, nullptr},
	{"Cinematic camera", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclCinematicCamera, nullptr},
	{"Extend zoom (0.5 steps)", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclExtendZoom, nullptr},
	{"Better spectate", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclBetterSpectate, nullptr},
	{"Notify when moved in spec", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclSpecMovedNotify, nullptr},
	{"Lock team automatically after joining", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclAutoTeamLock, nullptr},
	{"Confirm before quitting", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_BclConfirmQuit, nullptr},
	{"Moving/ego tiles prediction", SETTINGS_NAV_LEAF_BC_TOOLS, "Gameplay QoL", &g_Config.m_TcEgoTilesAntiLag, nullptr},

	{"Rollback Demo", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_ClReplays, nullptr},
	{"Rollback length", SETTINGS_NAV_LEAF_BC_TOOLS, "Rollback Demo", &g_Config.m_ClReplayLength, &g_Config.m_ClReplays},

	{"Focus Mode", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_ClFocusMode, nullptr},

	{"Client Indicator", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclClientIndicatorInNamePlate, nullptr},
	{"Show indicator in scoreboard", SETTINGS_NAV_LEAF_BC_TOOLS, "Client Indicator", &g_Config.m_BclClientIndicatorInScoreboard, nullptr},

	{"Save unsent messages", SETTINGS_NAV_LEAF_BC_TOOLS, "Chat QoL", &g_Config.m_ClChatReset, nullptr},
	{"Silent typing", SETTINGS_NAV_LEAF_BC_TOOLS, "Chat QoL", &g_Config.m_BclSilentTyping, nullptr},
	{"Commands in other layout", SETTINGS_NAV_LEAF_BC_TOOLS, "Chat QoL", &g_Config.m_BclChatAltCommandLayout, nullptr},
	{"Reply/copy message menu", SETTINGS_NAV_LEAF_BC_TOOLS, "Chat QoL", &g_Config.m_BclChatMessageActions, nullptr},
	{"Notify about saves on map", SETTINGS_NAV_LEAF_BC_TOOLS, "Chat QoL", &g_Config.m_BclNotifySavesOnMap, nullptr},

	{"Twitch Chat", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclTwitchChatNick, nullptr},

	{"Browser Utils", SETTINGS_NAV_LEAF_BC_TOOLS, "Tools", &g_Config.m_BclAutoServerListRefresh, nullptr},
	{"Short EGO/KoG server name", SETTINGS_NAV_LEAF_BC_TOOLS, "Browser Utils", &g_Config.m_BclUseShortKogServerName, nullptr},
	{"Show finished map on EGO/LEGIT-NET", SETTINGS_NAV_LEAF_BC_TOOLS, "Browser Utils", &g_Config.m_BclShowFinishedMapOnEgoLegit, nullptr},

	{"Show ping colored circle in nameplates", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcNameplatePingCircle, nullptr},
	{"Show country flags in nameplates", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcNameplateCountry, nullptr},
	{"Show skin names in nameplate", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcNameplateSkins, nullptr},
	{"Freeze stars", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_ClFreezeStars, nullptr},
	{"Colored frozen tee skins", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcColorFreeze, nullptr},
	{"Show katan on frozen players", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcFrozenKatana, nullptr},
	{"Tiny Tees", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcTinyTees, nullptr},
	{"Ingame tee scale", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcTeeScale, nullptr},
	{"White feet", SETTINGS_NAV_LEAF_TC_SETTINGS, "Visual", &g_Config.m_TcWhiteFeet, nullptr},

	{"Anti Latency Tools", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcRemoveAnti, nullptr},
	{"Dont predict other players if you are frozen", SETTINGS_NAV_LEAF_TC_SETTINGS, "Anti Latency Tools", &g_Config.m_TcUnpredOthersInFreeze, nullptr},
	{"Adjust your prediction margin while frozen", SETTINGS_NAV_LEAF_TC_SETTINGS, "Anti Latency Tools", &g_Config.m_TcPredMarginInFreeze, nullptr},

	{"Anti Ping Smoothing", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcAntiPingImproved, nullptr},

	{"Voting", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcAutoVoteWhenFar, nullptr},

	{"Auto Reply", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcAutoReplyMuted, nullptr},
	{"Auto reply when tabbed out", SETTINGS_NAV_LEAF_TC_SETTINGS, "Auto Reply", &g_Config.m_TcAutoReplyMinimized, nullptr},

	{"Player Indicator", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcPlayerIndicator, nullptr},

	{"Pet", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcPetShow, nullptr},
	{"Pet size", SETTINGS_NAV_LEAF_TC_SETTINGS, "Pet", &g_Config.m_TcPetSize, &g_Config.m_TcPetShow},
	{"Pet alpha", SETTINGS_NAV_LEAF_TC_SETTINGS, "Pet", &g_Config.m_TcPetAlpha, &g_Config.m_TcPetShow},

	{"Frozen Tee Display", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcShowFrozenHud, nullptr},

	{"Tile Outlines", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcOutline, nullptr},

	{"Ghost Tools", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcShowOthersGhosts, nullptr},

	{"Rainbow", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcRainbowTees, nullptr},
	{"Rainbow weapons", SETTINGS_NAV_LEAF_TC_SETTINGS, "Rainbow", &g_Config.m_TcRainbowWeapon, nullptr},
	{"Rainbow hook", SETTINGS_NAV_LEAF_TC_SETTINGS, "Rainbow", &g_Config.m_TcRainbowHook, nullptr},

	{"Tee Trails", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcTeeTrail, nullptr},
	{"Trail width", SETTINGS_NAV_LEAF_TC_SETTINGS, "Tee Trails", &g_Config.m_TcTeeTrailWidth, &g_Config.m_TcTeeTrail},
	{"Trail length", SETTINGS_NAV_LEAF_TC_SETTINGS, "Tee Trails", &g_Config.m_TcTeeTrailLength, &g_Config.m_TcTeeTrail},
	{"Trail alpha", SETTINGS_NAV_LEAF_TC_SETTINGS, "Tee Trails", &g_Config.m_TcTeeTrailAlpha, &g_Config.m_TcTeeTrail},

	{"Finish Name", SETTINGS_NAV_LEAF_TC_SETTINGS, "Settings", &g_Config.m_TcChangeNameNearFinish, nullptr},

	{"Show mini vote HUD", SETTINGS_NAV_LEAF_TC_SETTINGS, "HUD", &g_Config.m_TcMiniVoteHud, nullptr},
	{"Show position and angle (mini debug)", SETTINGS_NAV_LEAF_TC_SETTINGS, "HUD", &g_Config.m_TcMiniDebug, nullptr},
	{"Show your cursor when in free spectate", SETTINGS_NAV_LEAF_TC_SETTINGS, "HUD", &g_Config.m_TcRenderCursorSpec, nullptr},
	{"Show when you are the last alive", SETTINGS_NAV_LEAF_TC_SETTINGS, "HUD", &g_Config.m_TcNotifyWhenLast, nullptr},
	{"Show screen center lines", SETTINGS_NAV_LEAF_TC_SETTINGS, "HUD", &g_Config.m_TcShowCenter, nullptr},

	{"Status Bar", SETTINGS_NAV_LEAF_TC_STATUS_BAR, "TClient", &g_Config.m_TcStatusBar, nullptr},
	{"Status bar height", SETTINGS_NAV_LEAF_TC_STATUS_BAR, "Status Bar", &g_Config.m_TcStatusBarHeight, nullptr},
	{"Status bar alpha", SETTINGS_NAV_LEAF_TC_STATUS_BAR, "Status Bar", &g_Config.m_TcStatusBarAlpha, nullptr},
