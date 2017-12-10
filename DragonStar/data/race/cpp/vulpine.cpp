// ==================================
//
// vulpine.cpp
//
// ==================================

#include "../header/vulpine.h"

Vulpine::Vulpine(){
	name = "Vulpine";

	filepath = "gfx/actor/vulpine";

	activeRacial = AbilityID::RACIAL_SPIRIT_TAP;

	passiveRacial = "Wild Hunter";
	passiveDesc = "Increases water, wind, and earth resistance by 10%. Increases damage dealt to beasts by 10%.";
	passiveIcon = "gfx/ui/icon/default.png";

	// beast damage nyi
	passivePerks = std::vector<StatMod>({
		StatMod(StatModType::RESISTANCE, 0.1, { Category::ANY }, { Element::WATER, Element::WIND, Element::EARTH } )
	});

	baseEXPPerLevel = 300;

	baseHP = 340;
	baseMP = 230;
	baseSP = 100;

	baseSTR = 24;
	baseDEX = 34;
	baseINT = 33;
	baseWIS = 37;

	baseVIT = 100;

	bool canFly = false;
	bool canSwim = false;
}