#include <game/client/components/bestclient-light/keybind_map.h>

#include <gtest/gtest.h>

TEST(KeybindMap, Formats)
{
	bool SawLetter = false;
	bool SawFunction = false;
	bool SawNav = false;
	bool SawNumpad = false;
	for(const CKeybindMapKey &KeyDef : KEYBIND_MAP_KEYS)
	{
		const bool Full = KeybindMapKeyVisible(EKeybindMapFormat::FULL, KeyDef.m_Key);
		const bool Tenkeyless = KeybindMapKeyVisible(EKeybindMapFormat::TENKEYLESS, KeyDef.m_Key);
		const bool Compact = KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KeyDef.m_Key);
		EXPECT_TRUE(Full);
		if(KeyDef.m_Zone == EKeybindMapZone::ALPHA)
		{
			EXPECT_TRUE(Tenkeyless);
			EXPECT_TRUE(Compact);
			if(KeyDef.m_Key == KEY_A)
				SawLetter = true;
		}
		else if(KeyDef.m_Zone == EKeybindMapZone::FUNCTION_NAV)
		{
			EXPECT_TRUE(Tenkeyless);
			EXPECT_FALSE(Compact);
			if(KeyDef.m_Key == KEY_F1)
				SawFunction = true;
			if(KeyDef.m_Key == KEY_HOME || KeyDef.m_Key == KEY_UP)
				SawNav = true;
		}
		else
		{
			EXPECT_EQ(KeyDef.m_Zone, EKeybindMapZone::NUMPAD);
			EXPECT_FALSE(Tenkeyless);
			EXPECT_FALSE(Compact);
			if(KeyDef.m_Key == KEY_KP_0)
				SawNumpad = true;
		}
	}

	EXPECT_TRUE(SawLetter);
	EXPECT_TRUE(SawFunction);
	EXPECT_TRUE(SawNav);
	EXPECT_TRUE(SawNumpad);
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::FULL, KEY_KP_ENTER));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::FULL, KEY_F12));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::TENKEYLESS, KEY_RIGHT));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::TENKEYLESS, KEY_F12));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::TENKEYLESS, KEY_KP_ENTER));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::TENKEYLESS, KEY_KP_0));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_A));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_SPACE));
	EXPECT_TRUE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_RIGHTBRACKET));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_F1));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_HOME));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_UP));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_DELETE));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::COMPACT, KEY_KP_5));
	EXPECT_FALSE(KeybindMapKeyVisible(EKeybindMapFormat::FULL, KEY_MOUSE_1));
}

TEST(KeybindMap, BoundClassification)
{
	EXPECT_TRUE(KeybindMapIsBound("+left"));
	EXPECT_FALSE(KeybindMapIsBound(""));
	EXPECT_FALSE(KeybindMapIsBound(nullptr));
}

TEST(KeybindMap, ApplyClearAndReset)
{
	CKeybindMapBinding aSlots[2];
	aSlots[0].m_Key = KEY_G;
	aSlots[1].m_Key = KEY_D;
	ASSERT_TRUE(str_copy(aSlots[1].m_aCommand, "+right", sizeof(aSlots[1].m_aCommand)) >= 0);

	ASSERT_TRUE(KeybindMapCommit(aSlots, 2, KEY_G, EKeybindMapCommit::APPLY, KeybindMapMoveLeftCommand()));
	EXPECT_STREQ(aSlots[0].m_aCommand, "+left");
	EXPECT_STREQ(aSlots[1].m_aCommand, "+right");

	ASSERT_TRUE(KeybindMapCommit(aSlots, 2, KEY_G, EKeybindMapCommit::APPLY, "+jump"));
	EXPECT_STREQ(aSlots[0].m_aCommand, "+jump");
	EXPECT_STREQ(aSlots[1].m_aCommand, "+right");

	ASSERT_TRUE(KeybindMapCommit(aSlots, 2, KEY_G, EKeybindMapCommit::CLEAR, nullptr));
	EXPECT_STREQ(aSlots[0].m_aCommand, "");
	EXPECT_STREQ(aSlots[1].m_aCommand, "+right");

	ASSERT_TRUE(str_copy(aSlots[0].m_aCommand, "say hi", sizeof(aSlots[0].m_aCommand)) >= 0);
	aSlots[0].m_Key = KEY_A;
	ASSERT_TRUE(str_copy(aSlots[1].m_aCommand, "say bye", sizeof(aSlots[1].m_aCommand)) >= 0);
	ASSERT_TRUE(KeybindMapCommit(aSlots, 2, KEY_A, EKeybindMapCommit::RESET, nullptr));
	EXPECT_STREQ(aSlots[0].m_aCommand, "+left");
	EXPECT_STREQ(aSlots[1].m_aCommand, "say bye");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_A), "+left");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_SPACE), "+jump");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_Q), "say /spec");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_EQUALS), "spectate_next");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_G), "");
	EXPECT_STREQ(KeybindMapDefaultCommand(KEY_RIGHT), "+right");

	CKeybindMapBinding RightSlot;
	RightSlot.m_Key = KEY_RIGHT;
	ASSERT_TRUE(str_copy(RightSlot.m_aCommand, "say hi", sizeof(RightSlot.m_aCommand)) >= 0);
	ASSERT_TRUE(KeybindMapCommit(&RightSlot, 1, KEY_RIGHT, EKeybindMapCommit::RESET, nullptr));
	EXPECT_STREQ(RightSlot.m_aCommand, "+right");
	EXPECT_STREQ(aSlots[0].m_aCommand, "+left");
	EXPECT_STREQ(aSlots[1].m_aCommand, "say bye");

	CKeybindMapBinding EscapeSlot;
	EscapeSlot.m_Key = KEY_ESCAPE;
	ASSERT_TRUE(str_copy(EscapeSlot.m_aCommand, "+left", sizeof(EscapeSlot.m_aCommand)) >= 0);
	EXPECT_FALSE(KeybindMapCommit(&EscapeSlot, 1, KEY_ESCAPE, EKeybindMapCommit::CLEAR, nullptr));
	EXPECT_STREQ(EscapeSlot.m_aCommand, "+left");

	EXPECT_FALSE(KeybindMapCommit(aSlots, 2, KEY_G, EKeybindMapCommit::APPLY, "+left"));
	EXPECT_STREQ(aSlots[0].m_aCommand, "+left");
	EXPECT_STREQ(aSlots[1].m_aCommand, "say bye");
}

TEST(KeybindMap, ActionSearch)
{
	EXPECT_TRUE(KeybindMapActionVisible("", "Move left", "+left"));
	EXPECT_TRUE(KeybindMapActionVisible("move", "Move left", "+left"));
	EXPECT_TRUE(KeybindMapActionVisible("+jump", "Jump", "+jump"));
	EXPECT_FALSE(KeybindMapActionVisible("hook", "Move left", "+left"));
}
