/* Copyright © 2026 BestProject Team */
#ifndef GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_ASPECT_RATIO_H
#define GAME_CLIENT_COMPONENTS_BESTCLIENT_LIGHT_ASPECT_RATIO_H

#include <base/types.h>

#include <game/client/component.h>

class CAspectRatio : public CComponent
{
	class CAspectConfig
	{
	public:
		int m_Mode = 0;
		int m_Ratio = 0;
		int m_ApplyMode = 1;
		int m_Num = 0;
		int m_Den = 0;

		bool operator==(const CAspectConfig &Other) const
		{
			return m_Mode == Other.m_Mode && m_Ratio == Other.m_Ratio && m_ApplyMode == Other.m_ApplyMode &&
			       m_Num == Other.m_Num && m_Den == Other.m_Den;
		}
		bool operator!=(const CAspectConfig &Other) const { return !(*this == Other); }
	};

	bool m_ConfirmSeeded = false;
	bool m_ConfirmActive = false;
	int64_t m_ConfirmDeadline = 0;
	CAspectConfig m_Prev;
	CAspectConfig m_Last;
	bool m_HudAspectDisabled = false;
	bool m_MusicPlayerAspectDisabled = false;
	bool m_CachedUseGameNoHudAspect = false;
	bool m_MenuRestoreGraphicsAspect = true;

	class SForcedKey
	{
	public:
		int m_State = -1;
		int m_Menu = -1;
		int m_AllowZoom = -1;
		int m_Spec = -1;
		int m_Mode = -2;
		int m_Ratio = -1;
		int m_Apply = -1;
		int m_Num = -1;
		int m_Den = -1;
		int m_AnyRes = -1;

		bool operator==(const SForcedKey &Other) const
		{
			return m_State == Other.m_State && m_Menu == Other.m_Menu && m_AllowZoom == Other.m_AllowZoom && m_Spec == Other.m_Spec &&
			       m_Mode == Other.m_Mode && m_Ratio == Other.m_Ratio && m_Apply == Other.m_Apply && m_Num == Other.m_Num &&
			       m_Den == Other.m_Den && m_AnyRes == Other.m_AnyRes;
		}
	};

	bool m_ForcedKeyValid = false;
	SForcedKey m_ForcedKey;
	mutable bool m_CursorScaleValid = false;
	mutable int m_CursorScaleMode = -2;
	mutable int m_CursorScaleRatio = -1;
	mutable int m_CursorScaleNum = -1;
	mutable int m_CursorScaleDen = -1;
	mutable int m_CursorScaleW = -1;
	mutable int m_CursorScaleH = -1;
	mutable float m_CursorScaleX = 1.0f;
	mutable float m_CursorScaleY = 1.0f;

	static CAspectConfig ReadConfig();
	static bool ConfirmNeeded(const CAspectConfig &Cfg);
	static bool CustomAspectConfigured();
	SForcedKey ReadForcedKey() const;
	void ResetToPrevious();

public:
	int Sizeof() const override { return sizeof(*this); }
	void OnReset() override;

	bool IsBlockedByFng() const;
	bool IsAspectBlocked() const;
	bool UseGameNoHudAspect() const;
	bool ShouldApplyCustomAspect(bool IsActiveGameplay) const;
	void ScaleIngameCursorDelta(float &X, float &Y) const;

	void BeginFrame();
	void PrepareComponent(const CComponent *pComponent);
	void BeforeFogRect();
	void AfterFogRect();

	void BeginCursorGameAspect();
	void EndCursorGameAspect();

	void UpdateConfirm();
	bool ConfirmWantsInput() const;
	bool ShowConfirmOverlay() const;
	void ApplyMenuUiAspect(bool MenuActive);
	void RenderConfirmOverlay(bool MenuActive);
	void RestoreMenuUiAspect();
};

#endif
