// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

// Frozen skin
MACRO_CONFIG_STR(BclFrozenSkin, bcl_frozen_skin, 24, "", CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_INSENSITIVE, "Skin to show on frozen players, empty keeps the default behavior")
MACRO_CONFIG_INT(BclFrozenSkinDarken, bcl_frozen_skin_darken, 0, 0, 70, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Darken the frozen skin override (0 - normal, 70 - max darkened)")

// SettingsTabs
MACRO_CONFIG_INT(BclBestClientLightSettingsTabs, bcl_bestclient_light_settings_tabs, 0, 0, 65536, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bit flags to disable BestClient Light settings tabs") // bestclient-light

// Chat QoL
MACRO_CONFIG_INT(BclSilentTyping, bcl_silent_typing, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide PLAYERFLAG_CHATTING from others while typing")
MACRO_CONFIG_INT(BclChatAltCommandLayout, bcl_chat_alt_command_layout, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Interpret slash chat commands typed in the Russian keyboard layout")
MACRO_CONFIG_INT(BclChatMessageActions, bcl_chat_message_actions, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable reply/copy message menu on chat messages")
MACRO_CONFIG_INT(BclNotifySavesOnMap, bcl_notify_saves_on_map, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Notify in chat when the current map has team saves")

// Chat Filter
MACRO_CONFIG_INT(BclChatOnlyTagsAndWhispers, bcl_chat_only_tags_and_whispers, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show only highlighted and private chat messages")

// Translate
MACRO_CONFIG_STR(BclTranslateIncomingSource, bcl_translate_incoming_source, 16, "auto", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Source language for incoming chat translation")
MACRO_CONFIG_STR(BclTranslateIncomingIgnoreLanguages, bcl_translate_incoming_ignore_languages, 128, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Source languages that should not be auto-translated")
MACRO_CONFIG_STR(BclTranslateOutgoingSource, bcl_translate_outgoing_source, 16, "auto", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Source language for outgoing chat translation")
MACRO_CONFIG_STR(BclTranslateOutgoingTarget, bcl_translate_outgoing_target, 16, "en", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Target language for outgoing chat translation")
MACRO_CONFIG_INT(BclTranslateOutgoingStripPunctuation, bcl_translate_outgoing_strip_punctuation, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Remove apostrophes from translated outgoing chat messages")
MACRO_CONFIG_INT(BclTranslateShiftClickTarget, bcl_translate_shift_click_target, 2, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Shift+click translate icon toggles: 0 others, 1 yours, 2 both")
MACRO_CONFIG_INT(BclTranslateMessageButton, bcl_translate_message_button, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show a translate button to the right of other players' chat messages")

// Auto update
MACRO_CONFIG_INT(BclAutoUpdate, bcl_auto_update, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically download and apply updates on startup")

// Twitch chat integration
MACRO_CONFIG_STR(BclTwitchChatNick, bcl_twitch_chat_nick, 64, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Twitch channel nick for chat integration")

// Aspect ratio
MACRO_CONFIG_INT(BclCustomAspectRatioMode, bcl_custom_aspect_ratio_mode, -1, -1, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio mode (-1=legacy auto, 0=off, 1=preset, 2=custom)")
MACRO_CONFIG_INT(BclCustomAspectRatioApplyMode, bcl_custom_aspect_ratio_apply_mode, 1, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio apply mode (0=game only, 1=full, 2=game no hud)")
MACRO_CONFIG_INT(BclCustomAspectRatio, bcl_custom_aspect_ratio, 0, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio value x100 (0=off, presets: 125=5:4, 133=4:3, 150=3:2, custom: 100-1000)")
MACRO_CONFIG_INT(BclCustomAspectRatioNum, bcl_custom_aspect_ratio_num, 0, 0, 100000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom aspect ratio numerator (width), 0=unset")
MACRO_CONFIG_INT(BclCustomAspectRatioDen, bcl_custom_aspect_ratio_den, 0, 0, 100000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom aspect ratio denominator (height), 0=unset")

// Visuals QoL
MACRO_CONFIG_INT(BclShowRealHitbox, bcl_show_real_hitbox, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show a dot at the center of your real hitbox")
MACRO_CONFIG_COL(BclShowRealHitboxColor, bcl_show_real_hitbox_color, 0xFFFFFFFFU, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Color of the real hitbox center dot")
MACRO_CONFIG_INT(BclShowhudDummyCoordIndicator, bcl_showhud_dummy_coord_indicator, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show player-below indicator when aligned above another player")
MACRO_CONFIG_INT(BclShowCorrectCheckpoint, bcl_show_correct_checkpoint, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show current tele checkpoint in movement information")
MACRO_CONFIG_INT(BclShowPointsInTab, bcl_show_points_in_tab, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show player points in the scoreboard on DDNet, EGO and Legit Network servers")
MACRO_CONFIG_INT(BclShowSpectatorNames, bcl_show_spectator_names, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show spectator nicknames in the spectator count HUD")
MACRO_CONFIG_INT(BclShowVotePercentage, bcl_show_vote_percentage, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show yes/no vote percentages on the vote HUD")
MACRO_CONFIG_INT(BclDisplayMaxFps, bcl_display_max_fps, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show the maximum FPS in the HUD counter instead of the average")
MACRO_CONFIG_INT(BclScoreboardScale, bcl_scoreboard_scale, 100, 50, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Scoreboard scale in percent, independent of UI scale")
MACRO_CONFIG_INT(BclWheelScale, bcl_wheel_scale, 100, 50, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bind/emote wheel scale in percent, independent of UI scale")
MACRO_CONFIG_INT(UiScale, ui_scale, 100, 50, 110, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Global UI scale in percent")

// Discord RPC
MACRO_CONFIG_INT(BclDiscordRPC, bcl_discord_rpc, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable bestclient light discord rpc (restart requires)")

// Cursor trail
MACRO_CONFIG_INT(BclCursorTrail, bcl_cursor_trail, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable cursor trail")
MACRO_CONFIG_INT(BclCursorTrailMode, bcl_cursor_trail_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cursor trail mode (0=cursor, 1=custom)")
MACRO_CONFIG_STR(BclCursorTrailTrailImage, bcl_cursor_trail_trail_image, IO_MAX_PATH_LENGTH, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Path to the custom cursor trail image")
MACRO_CONFIG_INT(BclCursorTrailTrailSize, bcl_cursor_trail_trail_size, 70, 10, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cursor trail size")
MACRO_CONFIG_INT(BclCursorTrailNumberOfFrames, bcl_cursor_trail_number_of_frames, 5, 1, 10, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Number of cursor trail frames")
MACRO_CONFIG_INT(BclCursorTrailOpacity, bcl_cursor_trail_opacity, 80, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cursor trail opacity")
MACRO_CONFIG_INT(BclCursorTrailSamplingFps, bcl_cursor_trail_sampling_fps, 120, 24, 120, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cursor trail sampling frequency")
MACRO_CONFIG_INT(BclCursorTrailDisableMovement, bcl_cursor_trail_disable_movement, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Keep cursor trail stable when the player moves")

// Gameplay QoL
MACRO_CONFIG_INT(TcEgoTilesAntiLag, tc_ego_tiles_antilag, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Predict Ego-Tiles freeze/unfreeze quads like the server")
MACRO_CONFIG_INT(BclConfirmQuit, bcl_confirm_quit, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show a confirmation dialog before quitting the game")
MACRO_CONFIG_INT(BclCinematicCamera, bcl_cinematic_camera, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable smooth cinematic camera movement in spectator freeview")
MACRO_CONFIG_INT(BclCinematicCameraStrength, bcl_cinematic_camera_strength, 50, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Strength of cinematic camera smoothing (0 = mild, 100 = strong)")
MACRO_CONFIG_INT(BclSpecMovedNotify, bcl_spec_moved_notify, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show 'moved in game' warning when someone hooks/hits your character while you are in spectator")
MACRO_CONFIG_STR(BclSpecMovedNotifyText, bcl_spec_moved_notify_text, 128, "you moved in game", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text shown when someone hooks/hits your character while you are in spectator")
MACRO_CONFIG_INT(BclAutoTeamLock, bcl_auto_team_lock, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically lock your team after joining it")
MACRO_CONFIG_INT(BclAutoTeamLockDelay, bcl_auto_team_lock_delay, 5, 0, 30, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Delay before auto-locking team after joining, in seconds")
MACRO_CONFIG_INT(BclExtendZoom, bcl_extend_zoom, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use 0.5 zoom steps instead of 1 for finer scroll zoom (10, 9.5, 9, 8.5 ...)")
MACRO_CONFIG_INT(BclBetterSpectate, bcl_better_spectate, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Replace say /pause bind (default Q) with +specpause radio menu")
MACRO_CONFIG_INT(BclSpecPauseShowDelay, bcl_spec_pause_show_delay, 0, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Delay before the spec pause radio becomes visible (ms)")

// CameraDrift (demo playback only)
MACRO_CONFIG_INT(BclCameraDrift, bcl_camera_drift, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable camera drift during demo playback")
MACRO_CONFIG_INT(BclCameraDriftAmount, bcl_camera_drift_amount, 50, 1, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount of camera drift during demo playback")
MACRO_CONFIG_INT(BclCameraDriftSmoothness, bcl_camera_drift_smoothness, 20, 1, 20, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Smoothness of camera drift during demo playback")
MACRO_CONFIG_INT(BclCameraDriftReverse, bcl_camera_drift_reverse, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Reverse camera drift direction during demo playback")
// Freeview Numpad Camera (spectator/demo playback)
MACRO_CONFIG_INT(BclFreeviewNumpad, bcl_freeview_numpad, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable numpad camera movement in freeview/spectator mode")
MACRO_CONFIG_INT(BclFreeviewSpeed, bcl_freeview_speed, 180, 1, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Freeview camera movement speed")
MACRO_CONFIG_INT(BclFreeviewSmoothness, bcl_freeview_smoothness, 25, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Freeview camera smoothness (acceleration)")
// DynamicFov (demo playback only)
MACRO_CONFIG_INT(BclDynamicFov, bcl_dynamic_fov, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable dynamic FOV during demo playback")
MACRO_CONFIG_INT(BclDynamicFovAmount, bcl_dynamic_fov_amount, 50, 1, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount of dynamic FOV during demo playback")
MACRO_CONFIG_INT(BclDynamicFovSmoothness, bcl_dynamic_fov_smoothness, 20, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Smoothness of dynamic FOV during demo playback")

// Quick binds (45 degrees / small sens)
MACRO_CONFIG_INT(BclPrevMouseMaxDistance45Degrees, bcl_prev_mouse_max_distance_45_degrees, 400, 0, 5000, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_INSENSITIVE, "Previous maximum cursor distance for 45 degrees")
MACRO_CONFIG_INT(BclPrevInpMousesens45Degrees, bcl_prev_inp_mousesens_45_degrees, 200, 1, 1000000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Previous mouse sensitivity for 45 degrees")
MACRO_CONFIG_INT(BclToggle45Degrees, bcl_toggle_45_degrees, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use toggle mode for 45 degrees bind")
MACRO_CONFIG_INT(BclPrevInpMousesensSmallSens, bcl_prev_inp_mousesens_small_sens, 200, 1, 1000000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Previous mouse sensitivity for small sens")
MACRO_CONFIG_INT(BclToggleSmallSens, bcl_toggle_small_sens, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use toggle mode for small sens bind")

// Inputs
MACRO_CONFIG_INT(BclInputDebounce, bcl_input_debounce, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Preserve button presses shorter than one simulation tick")
MACRO_CONFIG_INT(BclInputs, bcl_inputs, 0, 0, 6, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Input prediction mode (0 = off, 1 = fast, 2 = best, 3 = saiko, 4 = delta, 5 = f, 6 = cloud)")
MACRO_CONFIG_INT(BclBestInputAmount, bcl_best_input_amount, 0, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Best input prediction amount in 0.01 ticks")
MACRO_CONFIG_INT(BclBestInputSmoothing, bcl_best_input_smoothing, 0, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Best input smoothing amount (0-100%)")
MACRO_CONFIG_INT(BclBestInputLatencyComp, bcl_best_input_latency_comp, 0, 0, 50, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Best input latency compensation (0-50%)")
MACRO_CONFIG_INT(BclBestInputInterpolation, bcl_best_input_interpolation, 1, 1, 3, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Best input interpolation mode (1 = linear, 2 = cubic, 3 = smooth)")
MACRO_CONFIG_INT(BclBestInputOthers, bcl_best_input_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Apply best input to other tees")
MACRO_CONFIG_INT(BclSaikoInputAmount, bcl_saiko_input_amount, 0, 0, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Saiko input amount in 0.01 ticks")
MACRO_CONFIG_INT(BclSaikoInputOthers, bcl_saiko_input_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Apply Saiko input to other tees")
MACRO_CONFIG_INT(BclDeltaInputAmount, bcl_delta_input_amount, 0, 0, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Delta input amount in 0.01 ticks")
MACRO_CONFIG_INT(BclDeltaInputOthers, bcl_delta_input_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Apply delta input to other tees")
MACRO_CONFIG_INT(BclFInputAmount, bcl_f_input_amount, 0, 0, 5000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "F input amount in 0.001-tick units (0=off, 1000=1.000 ticks, max 5000=5.000 ticks)")
MACRO_CONFIG_INT(BclFInputOthers, bcl_f_input_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Apply F input to other tees")
MACRO_CONFIG_INT(BclCloudInputAmount, bcl_cloud_input_amount, 0, 0, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cloud input amount in 0.01 ticks")
MACRO_CONFIG_INT(BclCloudInputOthers, bcl_cloud_input_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Apply cloud input to other tees")
MACRO_CONFIG_INT(BclAutoMargin, bcl_auto_margin, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto margin")

// MuteOthers
MACRO_CONFIG_INT(BclMuteOthersHook, bcl_mute_others_hook, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mute hook sounds of other players")
MACRO_CONFIG_INT(BclMuteOthersHammer, bcl_mute_others_hammer, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mute hammer sounds of other players")
MACRO_CONFIG_INT(BclMuteOthersAirJump, bcl_mute_others_airjump, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mute double jump sounds of other players")

// Gores mode
MACRO_CONFIG_INT(BclGoresMode, bcl_gores_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Entity-like gores mode")
MACRO_CONFIG_INT(BclGoresModeDisableIfWeapons, bcl_gores_mode_disable_weapons, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Disable gores mode when holding shotgun, grenade or laser")

// Fast Actions
MACRO_CONFIG_INT(BclFastActions, bcl_fast_actions, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable the Fast Actions quick command selector")

// Snap Tap
MACRO_CONFIG_INT(BclSnapTap, bcl_snap_tap, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Snap Tap for opposite left/right inputs")
MACRO_CONFIG_INT(BclSnapTapDelay, bcl_snap_tap_delay, 0, 0, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Snap Tap direction switch delay in milliseconds (0 = off)")

// Assets (cursor / arrow / audio)
MACRO_CONFIG_STR(ClAssetCursor, cl_asset_cursor, 50, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "The asset for cursor")
MACRO_CONFIG_STR(ClAssetArrow, cl_asset_arrow, 50, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "The asset for arrow")
MACRO_CONFIG_STR(SndPack, snd_pack, 64, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Sound pack")

// Profile assets
MACRO_CONFIG_INT(BclProfileAssets, bcl_profile_assets, 0, 0, 511, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bitmask of asset types to save/load in profiles")

// Better preview
MACRO_CONFIG_INT(BclBetterPreview, bcl_better_preview, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Better preview")

// Media background
MACRO_CONFIG_INT(BclMenuMediaBackground, bcl_menu_media_background, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable custom media background in offline menus")
MACRO_CONFIG_INT(BclGameMediaBackground, bcl_game_media_background, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable custom media background in game background rendering")
MACRO_CONFIG_STR(BclMenuMediaBackgroundPath, bcl_menu_media_background_path, IO_MAX_PATH_LENGTH, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Path to the custom menu media background file")
MACRO_CONFIG_INT(BclGameMediaBackgroundOffset, bcl_game_media_background_offset, 0, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "How much the custom media background is fixed to the map when rendering the in-game background")

// Menu UI
MACRO_CONFIG_INT(BclMenuUiStyle, bcl_menu_ui_style, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Menu UI style (0 = legacy, 1 = Better UI)")
MACRO_CONFIG_INT(BclMenuUiNewCheckbox, bcl_menu_ui_new_checkbox, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use better checkbox style when Better UI is enabled")
MACRO_CONFIG_INT(BclMenuUiNewScrollbar, bcl_menu_ui_new_scrollbar, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use better scrollbar style when Better UI is enabled")
MACRO_CONFIG_INT(BclMenuUiNewTabOption, bcl_menu_ui_new_tab_option, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use better settings tab navigation when Better UI is enabled")
MACRO_CONFIG_INT(BclMenuUiTabSide, bcl_menu_ui_tab_side, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Settings tabs side when Better tab option is on (0 = left, 1 = right)")
MACRO_CONFIG_INT(BclMenuUiNewColorPicker, bcl_menu_ui_new_color_picker, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use better color picker style when Better UI is enabled")
MACRO_CONFIG_INT(BclMenuUiBetterFont, bcl_menu_ui_better_font, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use Better font (Actay Wide Bold) instead of default/TClient font")
MACRO_CONFIG_INT(BclShowStylePicker, bcl_show_style_picker, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show one-time menu UI style picker on first launch")
MACRO_CONFIG_INT(BclSettingsNavExpanded, bcl_settings_nav_expanded, 0, 0, 7, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Better settings nav expanded groups (bit 0 = DDNet, 1 = TClient, 2 = BestClient Light)")

// Browser Utils
MACRO_CONFIG_INT(BclAutoServerListRefresh, bcl_auto_server_list_refresh, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically refresh the server browser list while a browser tab is open")
MACRO_CONFIG_INT(BclAutoServerListRefreshSeconds, bcl_auto_server_list_refresh_seconds, 10, 1, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto refresh interval for the server browser list in seconds")
MACRO_CONFIG_INT(BclUseShortKogServerName, bcl_use_short_kog_server_name, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use short names for E-Gores and KoG servers in browser list")
MACRO_CONFIG_INT(BclShowFinishedMapOnEgo, bcl_show_finished_map_on_ego, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show completed EGO maps in the server browser")
MACRO_CONFIG_INT(BclShowFinishedMapOnLegit, bcl_show_finished_map_on_legit, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show completed Legit Network maps in the server browser")
MACRO_CONFIG_INT(BclMastersrv, bcl_mastersrv, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use the BestClient Light master server mirror for the server browser")
MACRO_CONFIG_INT(BclServerMapPreview, bcl_server_map_preview, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show a live map preview under the selected server")
MACRO_CONFIG_INT(BrFilterBestclientLight, br_filter_bestclient_light, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Filter out servers with no BestClient Light users")

// Client Indicator
MACRO_CONFIG_INT(BclClientIndicatorVersions, bcl_client_indicator_versions, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show versions tab in BestClient Light server browser panel")
MACRO_CONFIG_INT(DbgClientIndicator, dbg_client_indicator, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Debug logging for BestClient Light indicator (1=verbose, 2=dump all UDP packet bytes sent/received)")
MACRO_CONFIG_INT(BclNameplateClientIndicatorOffsetX, bcl_nameplate_client_indicator_offset_x, 0, -400, 400, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Horizontal offset for the client indicator in nameplates")
MACRO_CONFIG_INT(BclNameplateClientIndicatorOffsetY, bcl_nameplate_client_indicator_offset_y, 0, -400, 400, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Vertical offset for the client indicator in nameplates")
MACRO_CONFIG_INT(BclClientIndicatorInNamePlate, bcl_client_indicator_in_name_plate, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show client indicator in name plate")
MACRO_CONFIG_INT(BclClientIndicatorInNamePlateAboveSelf, bcl_client_indicator_in_name_plate_above_self, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show client indicator above self")
MACRO_CONFIG_INT(BclClientIndicatorInNamePlateSize, bcl_client_indicator_in_name_plate_size, 30, -50, 100, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Client indicator in name plate size")
MACRO_CONFIG_INT(BclClientIndicatorInNamePlateDynamic, bcl_client_indicator_in_name_plate_dynamic, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator in nameplates will dynamically change pos")
MACRO_CONFIG_INT(IndicatorVersion, indicator_version, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show BestClient Light version in name plates")
MACRO_CONFIG_INT(BclClientIndicatorInScoreboard, bcl_client_indicator_in_scoreboard, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show client indicator in scoreboard")
MACRO_CONFIG_INT(BclClientIndicatorInSoreboardSize, bcl_client_indicator_in_scoreboard_size, 100, -50, 100, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Client indicator in scoreboard size")
MACRO_CONFIG_STR(BclClientIndicatorServerAddress, bcl_client_indicator_server_address, 256, "150.241.70.188:8778", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator UDP presence server")
MACRO_CONFIG_STR(BclClientIndicatorBrowserUrl, bcl_client_indicator_browser_url, 256, "https://150.241.70.188:8779/users.json", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator browser JSON URL")
MACRO_CONFIG_STR(BclClientIndicatorTokenUrl, bcl_client_indicator_token_url, 256, "https://150.241.70.188:8779/token.json", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator token bootstrap URL")
MACRO_CONFIG_STR(BclClientIndicatorSharedToken, bcl_client_indicator_shared_token, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator shared token for signed UDP packets")
MACRO_CONFIG_STR(BclClientIndicatorSecretKey, bcl_client_indicator_secret_key, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Client indicator developer secret key")

// Optimizer
MACRO_CONFIG_INT(BclOptimizer, bcl_optimizer, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable optimizer features")
MACRO_CONFIG_INT(BclOptimizerDisableParticles, bcl_optimizer_disable_particles, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Disable rendering/updating all particles")
MACRO_CONFIG_INT(BclOptimizerDisableQuads, bcl_optimizer_disable_quads, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Disable rendering map quad layers")
MACRO_CONFIG_INT(BclOptimizerDisableHighDetail, bcl_optimizer_disable_high_detail, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Disable high detail map layers (mirrors gfx_high_detail off while optimizer is enabled)")
MACRO_CONFIG_INT(BclOptimizerFpsFog, bcl_optimizer_fps_fog, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cull non-map rendering outside a distance limit around the camera")
MACRO_CONFIG_INT(BclOptimizerFpsFogMode, bcl_optimizer_fps_fog_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "FPS fog mode (0=manual radius tiles, 1=by zoom percent)")
MACRO_CONFIG_INT(BclOptimizerFpsFogRadiusTiles, bcl_optimizer_fps_fog_radius_tiles, 40, 5, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "FPS fog manual radius in tiles (tile=32 units)")
MACRO_CONFIG_INT(BclOptimizerFpsFogZoomPercent, bcl_optimizer_fps_fog_zoom_percent, 70, 10, 120, CFGFLAG_CLIENT | CFGFLAG_SAVE, "FPS fog visible area percent in zoom mode")
MACRO_CONFIG_INT(BclOptimizerFpsFogRenderRect, bcl_optimizer_fps_fog_render_rect, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Render an outline rectangle showing the FPS fog area")
MACRO_CONFIG_INT(BclOptimizerFpsFogCullMapTiles, bcl_optimizer_fps_fog_cull_map_tiles, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cull map tile rendering outside the FPS fog area")

// Process priority (from Entity-Client)
MACRO_CONFIG_INT(BclHighProcessPriority, bcl_high_process_priority, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Set DDNets process priority to high")
MACRO_CONFIG_INT(BclDiscordNormalProcessPriority, bcl_discord_normal_process_priority, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Set Discord process priority to normal")

// Chat bubbles (above players)
MACRO_CONFIG_INT(BclChatBubbles, bcl_chat_bubbles, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show chat bubbles above players")
MACRO_CONFIG_INT(BclChatBubblesSelf, bcl_chat_bubbles_self, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show chat bubbles above yourself")
MACRO_CONFIG_INT(BclChatBubblesDemo, bcl_chat_bubbles_demo, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show chat bubbles in demo playback")
MACRO_CONFIG_INT(BclChatBubbleSize, bcl_chat_bubble_size, 20, 20, 30, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of chat bubbles")
MACRO_CONFIG_INT(BclChatBubbleShowTime, bcl_chat_bubble_showtime, 500, 100, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "How long to show chat bubbles (centiseconds)")
MACRO_CONFIG_INT(BclChatBubbleFadeOut, bcl_chat_bubble_fadeout, 35, 15, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat bubble fade-out duration (centiseconds)")
MACRO_CONFIG_INT(BclChatBubbleFadeIn, bcl_chat_bubble_fadein, 15, 15, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat bubble fade-in duration (centiseconds)")
MACRO_CONFIG_INT(BclChatBubbleAnimation, bcl_chat_bubble_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Animate chat bubble stacking movement")
MACRO_CONFIG_INT(BclChatBubbleCustomColors, bcl_chat_bubble_custom_colors, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use custom colors for chat bubbles")
MACRO_CONFIG_COL(BclChatBubbleBgColor, bcl_chat_bubble_bg_color, 0x40000000U, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Chat bubble background color")
MACRO_CONFIG_COL(BclChatBubbleTextColor, bcl_chat_bubble_text_color, 0xFFFFFFFFU, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Chat bubble text color")
MACRO_CONFIG_COL(BclChatBubbleOutlineColor, bcl_chat_bubble_outline_color, 0x80000000U, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Chat bubble text outline color")
MACRO_CONFIG_INT(BclChatBubbleRounding, bcl_chat_bubble_rounding, 0, 0, 30, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat bubble corner rounding (0 = auto from size)")

// Animations
MACRO_CONFIG_INT(BclAnimations, bcl_animations, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle BestClient Light UI animations")
MACRO_CONFIG_INT(BclModuleUiRevealAnimation, bcl_module_ui_reveal_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle settings UI animations (module reveals and tab switch)")
MACRO_CONFIG_INT(BclModuleUiRevealAnimationMs, bcl_module_ui_reveal_animation_ms, 180, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Module settings reveal time (in ms)")
MACRO_CONFIG_INT(BclChatAnimation, bcl_chat_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle chat animation")
MACRO_CONFIG_INT(BclChatAnimationMs, bcl_chat_animation_ms, 300, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat animation time (in ms)")
MACRO_CONFIG_INT(BclChatOpenAnimation, bcl_chat_open_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle chat open animation")
MACRO_CONFIG_INT(BclChatOpenAnimationMs, bcl_chat_open_animation_ms, 220, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat open animation time (in ms)")
MACRO_CONFIG_INT(BclChatTypingAnimation, bcl_chat_typing_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle chat typing animation")
MACRO_CONFIG_INT(BclChatTypingAnimationMs, bcl_chat_typing_animation_ms, 180, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat typing animation time (in ms)")
MACRO_CONFIG_INT(BclKillfeedAnimation, bcl_killfeed_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle killfeed animation")
MACRO_CONFIG_INT(BclKillfeedAnimationMs, bcl_killfeed_animation_ms, 200, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Killfeed animation time (in ms)")
MACRO_CONFIG_INT(BclMainMenuAnimation, bcl_main_menu_animation, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggle main menu button hover animation")
MACRO_CONFIG_INT(BclMainMenuAnimationSpeed, bcl_main_menu_animation_speed, 12, 1, 50, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Main menu hover animation speed")
MACRO_CONFIG_INT(BclChatAnimationType, bcl_chat_animation_type, 3, 1, 4, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Chat animation type")

// Motion blur
MACRO_CONFIG_INT(BclMotionBlur, bcl_motion_blur, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable motion blur via previous-frame blending")
MACRO_CONFIG_INT(BclMotionBlurStrength, bcl_motion_blur_strength, 50, 0, 95, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Strength of motion blur frame blending (0-95%)")

// Edge Info (from RushieClient)
MACRO_CONFIG_COL(RiEdgeInfoColorFreeze, ri_edge_info_color_freeze, 9930605, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Freeze color in edge info")
MACRO_CONFIG_COL(RiEdgeInfoColorKill, ri_edge_info_color_kill, 65461, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Kill color in edge info")
MACRO_CONFIG_COL(RiEdgeInfoColorSafe, ri_edge_info_color_safe, 5594535, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Safe color in edge info")
MACRO_CONFIG_INT(RiEdgeInfoCords, ri_edge_info_cords, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show upper panel of edge info")
MACRO_CONFIG_INT(RiEdgeInfoJump, ri_edge_info_jump, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show lower panel of edge info")
MACRO_CONFIG_INT(RiEdgeInfoPosX, ri_edge_info_pos_x, 50, 0, 100, CFGFLAG_CLIENT, "Legacy edge info pos x; migrated into hud_layout.cfg")
MACRO_CONFIG_INT(RiEdgeInfoPosY, ri_edge_info_pos_y, 56, 0, 100, CFGFLAG_CLIENT, "Legacy edge info pos y; migrated into hud_layout.cfg")

// Legacy HUD layout mirrors from BestClient Light 2.0 (load-only; migrated into hud_layout.cfg)
MACRO_CONFIG_INT(BclHudMusicPlayerX, bcl_hud_music_player_x, 198, -1000, 1000, CFGFLAG_CLIENT, "Legacy music player HUD X")
MACRO_CONFIG_INT(BclHudMusicPlayerY, bcl_hud_music_player_y, 0, -1000, 1000, CFGFLAG_CLIENT, "Legacy music player HUD Y")
MACRO_CONFIG_INT(BclHudMusicPlayerScale, bcl_hud_music_player_scale, 100, 25, 300, CFGFLAG_CLIENT, "Legacy music player HUD scale")
MACRO_CONFIG_INT(BclHudMusicPlayerLayoutMigrated, bcl_hud_music_player_layout_migrated, 0, 0, 5, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player HUD layout anchor migration from BestClient Light 2.0")
MACRO_CONFIG_INT(BclHudChatX, bcl_hud_chat_x, 5, -1000, 1000, CFGFLAG_CLIENT, "Legacy chat HUD X")
MACRO_CONFIG_INT(BclHudChatY, bcl_hud_chat_y, 278, -1000, 1000, CFGFLAG_CLIENT, "Legacy chat HUD Y")
MACRO_CONFIG_INT(BclHudChatScale, bcl_hud_chat_scale, 100, 25, 300, CFGFLAG_CLIENT, "Legacy chat HUD scale")
MACRO_CONFIG_INT(BclHudVotesX, bcl_hud_votes_x, 0, -1000, 1000, CFGFLAG_CLIENT, "Legacy votes HUD X")
MACRO_CONFIG_INT(BclHudVotesY, bcl_hud_votes_y, 60, -1000, 1000, CFGFLAG_CLIENT, "Legacy votes HUD Y")
MACRO_CONFIG_INT(BclHudVotesScale, bcl_hud_votes_scale, 100, 25, 300, CFGFLAG_CLIENT, "Legacy votes HUD scale")

// Keystrokes
MACRO_CONFIG_INT(BclKeystrokesStyle, bcl_keystrokes_style, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Keystrokes style (0 = Classic, 1 = Minecraft)")
MACRO_CONFIG_INT(BclKeystrokesKeyboard, bcl_keystrokes_keyboard, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show keyboard keystrokes HUD")
MACRO_CONFIG_INT(BclKeystrokesKeyboardPreset, bcl_keystrokes_keyboard_preset, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Keyboard keystrokes preset (0 = wasd-minimal, 1 = wasd-full, 2 = micro)")
MACRO_CONFIG_INT(BclKeystrokesMouse, bcl_keystrokes_mouse, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show mouse keystrokes HUD")
MACRO_CONFIG_INT(BclKeystrokesMousePreset, bcl_keystrokes_mouse_preset, 1, 1, 3, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mouse keystrokes preset (1 = mouse-arrow, 2 = mouse-dot-dot, 3 = mouse-nothing)")
MACRO_CONFIG_INT(BclKeystrokesMcLayout, bcl_keystrokes_mc_layout, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Minecraft keystrokes layout (0 = Full, 1 = Only A/D)")
MACRO_CONFIG_INT(BclKeystrokesMcShowLmb, bcl_keystrokes_mc_show_lmb, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show LMB in Minecraft keystrokes")
MACRO_CONFIG_INT(BclKeystrokesMcShowRmb, bcl_keystrokes_mc_show_rmb, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show RMB in Minecraft keystrokes")
MACRO_CONFIG_INT(BclKeystrokesMcShowSpace, bcl_keystrokes_mc_show_space, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show Space in Minecraft keystrokes")
MACRO_CONFIG_INT(BclKeystrokesMcPressedOpacity, bcl_keystrokes_mc_pressed_opacity, 92, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Pressed key opacity in Minecraft keystrokes")

// Finish prediction
MACRO_CONFIG_INT(BclFinishPrediction, bcl_finish_prediction, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable finish prediction")
MACRO_CONFIG_INT(BclFinishPredictionShowAlways, bcl_finish_prediction_show_always, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show finish prediction even before race start")
MACRO_CONFIG_INT(BclFinishPredictionTimeMode, bcl_finish_prediction_time_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Finish prediction time mode (0=remaining, 1=predicted finish time)")
MACRO_CONFIG_INT(BclFinishPredictionShowTime, bcl_finish_prediction_show_time, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show finish prediction time")
MACRO_CONFIG_INT(BclFinishPredictionShowPercentage, bcl_finish_prediction_show_percentage, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show finish prediction progress percentage")
MACRO_CONFIG_INT(BclFinishPredictionShowMillis, bcl_finish_prediction_show_millis, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show finish prediction milliseconds")
MACRO_CONFIG_INT(BclFinishPredictionConsiderFreezeWalls, bcl_finish_prediction_consider_freeze_walls, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Experimental: treat jumpable freeze walls as passable for finish prediction")

// Music Player
MACRO_CONFIG_INT(BclMusicPlayer, bcl_music_player, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Music Player HUD element")
MACRO_CONFIG_INT(BclMusicPlayerVisualizerMode, bcl_music_player_visualizer_mode, 1, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player visualizer mode (0=bottom, 1=center, 2=up)")
MACRO_CONFIG_INT(BclMusicPlayerVisualizerRounding, bcl_music_player_visualizer_rounding, 0, 0, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player visualizer rounding (0=cube, 200=soft)")
MACRO_CONFIG_INT(BclMusicPlayerVisualizerColumns, bcl_music_player_visualizer_columns, 5, 5, 10, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player visualizer bar count")
MACRO_CONFIG_INT(BclMusicPlayerColorMode, bcl_music_player_color_mode, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player color mode (0=static visualizer, 1=cover visualizer)")
MACRO_CONFIG_COL(BclMusicPlayerStaticColor, bcl_music_player_static_color, 128, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Static color for the music player when static color mode is selected")
MACRO_CONFIG_INT(BclMusicPlayerTextScale, bcl_music_player_text_scale, 110, 70, 150, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Music player text scale in percent")
MACRO_CONFIG_INT(BclMusicPlayerShowLyrics, bcl_music_player_show_lyrics, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show synchronized song lyrics in the music player")
MACRO_CONFIG_INT(BclMusicPlayerShowCurrentTime, bcl_music_player_show_current_time, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show current time tab under lyrics in the music player")

// Focus Mode Settings
MACRO_CONFIG_INT(ClFocusMode, p_focus_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable focus mode to minimize visual distractions")
MACRO_CONFIG_INT(ClFocusModeHideNames, p_focus_mode_hide_names, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide player names in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideEffects, p_focus_mode_hide_effects, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide visual effects in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideHud, p_focus_mode_hide_hud, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide HUD in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideSongPlayer, p_focus_mode_hide_song_player, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide song player in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideUI, p_focus_mode_hide_ui, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide unnecessary UI elements in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideChat, p_focus_mode_hide_chat, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide chat in focus mode")
MACRO_CONFIG_INT(ClFocusModeHideScoreboard, p_focus_mode_hide_scoreboard, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide scoreboard in focus mode")
MACRO_CONFIG_INT(ClFocusModeAutoReply, p_focus_mode_auto_reply, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto reply to pings when focus mode hides chat")
MACRO_CONFIG_STR(ClFocusModeAutoReplyMessage, p_focus_mode_auto_reply_message, 128, "my chat is hidden", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Message to auto reply when focus mode hides chat")

// Corner indicator
MACRO_CONFIG_INT(BclCornerIndicator, bcl_corner_indicator, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show the corner indicator for off-screen teammates")
MACRO_CONFIG_INT(BclCornerIndicatorOnlyInTeam, bcl_corner_indicator_only_in_team, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Only show the corner indicator while in a DDRace team")
MACRO_CONFIG_INT(BclCornerIndicatorShowAlive, bcl_corner_indicator_show_alive, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show alive teammates in the corner indicator")
MACRO_CONFIG_INT(BclCornerIndicatorShowFrozen, bcl_corner_indicator_show_frozen, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show frozen teammates in the corner indicator")
MACRO_CONFIG_INT(BclCornerIndicatorIndicateLastTeammate, bcl_corner_indicator_indicate_last_teammate, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Indicate frozen teammates in the corner indicator when the local player is the last alive")
MACRO_CONFIG_INT(BclCornerIndicatorProximity, bcl_corner_indicator_proximity, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Scale the corner indicator by distance")
MACRO_CONFIG_INT(BclCornerIndicatorProximityRange, bcl_corner_indicator_proximity_range, 2000, 200, 8000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Corner indicator proximity range")
MACRO_CONFIG_INT(BclCornerIndicatorProximityMinSize, bcl_corner_indicator_proximity_min_size, 16, 4, 64, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Minimum corner indicator size when scaled by distance")
MACRO_CONFIG_INT(BclCornerIndicatorNinjaFrozen, bcl_corner_indicator_ninja_frozen, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use the ninja skin for frozen corner indicators")
MACRO_CONFIG_INT(BclCornerIndicatorSizeAlive, bcl_corner_indicator_size_alive, 48, 12, 128, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Alive corner indicator size")
MACRO_CONFIG_INT(BclCornerIndicatorSizeFrozen, bcl_corner_indicator_size_frozen, 48, 12, 128, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Frozen corner indicator size")
MACRO_CONFIG_INT(BclCornerIndicatorLastTeammateSize, bcl_corner_indicator_last_teammate_size, 48, 12, 128, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Corner indicator size when indicating the last teammate")
MACRO_CONFIG_INT(BclCornerIndicatorCustomColors, bcl_corner_indicator_custom_colors, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use custom colors for the corner indicator")
MACRO_CONFIG_COL(BclCornerIndicatorColorAlive, bcl_corner_indicator_color_alive, 0xFFAAFF80U, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Alive corner indicator color")
MACRO_CONFIG_COL(BclCornerIndicatorColorFrozen, bcl_corner_indicator_color_frozen, 0xFF6BFF8AU, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Frozen corner indicator color")
MACRO_CONFIG_COL(BclCornerIndicatorLastTeammateColor, bcl_corner_indicator_last_teammate_color, 0xFF6EFF80U, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Corner indicator color when indicating the last teammate")

// Atmosphere
MACRO_CONFIG_INT(BclAtmosphere, bcl_atmosphere, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Weather around you")
MACRO_CONFIG_INT(BclWeather, bcl_weather, 0, 0, 3, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Weather type (0 = snow, 1 = rain, 2 = fireflies, 3 = dust)")
MACRO_CONFIG_INT(BclWeatherAmount, bcl_weather_amount, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount of weather in percent")
MACRO_CONFIG_INT(BclWeatherSnowSize, bcl_weather_snow_size, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of snow particles in percent")
MACRO_CONFIG_INT(BclWeatherSnowSpeed, bcl_weather_snow_speed, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Speed of snow particles in percent")
MACRO_CONFIG_COL(BclWeatherSnowColor, bcl_weather_snow_color, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of the snow")
MACRO_CONFIG_INT(BclWeatherSnowflakes, bcl_weather_snowflakes, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Snow look (0 = dots, 1 = snowflakes)")
MACRO_CONFIG_INT(BclWeatherRainSize, bcl_weather_rain_size, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of rain streaks in percent")
MACRO_CONFIG_INT(BclWeatherRainSpeed, bcl_weather_rain_speed, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Speed of rain in percent")
MACRO_CONFIG_INT(BclWeatherRainTilt, bcl_weather_rain_tilt, 5, -75, 75, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Rain tilt from vertical in degrees")
MACRO_CONFIG_COL(BclWeatherRainColor, bcl_weather_rain_color, 10063569, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of the rain")
MACRO_CONFIG_INT(BclWeatherFireflySize, bcl_weather_firefly_size, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of fireflies in percent")
MACRO_CONFIG_INT(BclWeatherFireflySpeed, bcl_weather_firefly_speed, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Speed of fireflies in percent")
MACRO_CONFIG_COL(BclWeatherFireflyColor, bcl_weather_firefly_color, 3791782, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of the fireflies")
MACRO_CONFIG_INT(BclAmbientDust, bcl_ambient_dust, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Drift motes of dust through the world for atmosphere")
MACRO_CONFIG_COL(BclAmbientDustColor, bcl_ambient_dust_color, 1981660, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of the ambient dust motes")
MACRO_CONFIG_INT(BclAmbientDustAmount, bcl_ambient_dust_amount, 100, 25, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount of ambient dust motes in percent")
MACRO_CONFIG_INT(BclWeatherDustSize, bcl_weather_dust_size, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Size of dust motes in percent")
MACRO_CONFIG_INT(BclWeatherDustSpeed, bcl_weather_dust_speed, 100, 25, 300, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Speed of dust motes in percent")
