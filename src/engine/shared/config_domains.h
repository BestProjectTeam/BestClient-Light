// This file can be included several times.

#ifndef CONFIG_DOMAIN
#error "CONFIG_DOMAIN macro not defined"
#define CONFIG_DOMAIN(Name, ConfigPath, HasVars) ;
#endif

CONFIG_DOMAIN(DDNET, "settings_ddnet.cfg", true)
CONFIG_DOMAIN(TCLIENT, "settings_tclient.cfg", true)
CONFIG_DOMAIN(BESTCLIENT_LIGHT, "settings_bestclient_light.cfg", true)
CONFIG_DOMAIN(HUDLAYOUT, "bestclient-light/hud_layout.cfg", false) // bestclient-light
CONFIG_DOMAIN(TCLIENTPROFILES, "tclient_profiles.cfg", false)
CONFIG_DOMAIN(TCLIENTCHATBINDS, "tclient_chatbinds.cfg", false)
CONFIG_DOMAIN(TCLIENTWARLIST, "tclient_warlist.cfg", false)
