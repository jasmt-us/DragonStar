// ==================================
//
// au_commandingShout.cpp
//
// ==================================

#include "../header/au_commandingShout.h"

#include "../../id/abilityID.h"
#include "../../id/attribute.h"
#include "../../id/auraID.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../id/statModType.h"

Au_CommandingShout::Au_CommandingShout() {
	name = "Commanding Shout";
	id = AuraID::CommandingShout;
	icon = "commanding_shout.png";

	categories = {
		Category::Skill
	};

	elements = {};

	statMods = {
		StatMod(StatModType::Damage, 0.05),
		StatMod(StatModType::DamageTaken, -0.05)
	};

	canCrit = false;

	baseDuration = 1200;
	timePerTick = 100;
	maxStacks = 1;
	hasTicks = false;
	fixedDuration = false; // if true, ignore VIT in calculations
	unlimitedDuration = false;
	consumedOnUse = false; // if true, aura will expire when called in combat calculations
	isBuff = true;

	isStun = false;
	isDisarm = false;
	isSilence = false;
	isSlow = false;

	uniquePerActor = false; // determines if one actor can apply multiple copies of aura
	uniqueAura = false; // if true only one copy can be applied regardless of source

	removeAllStacksOnExpiration = false; // Determines whether all stacks or 1 stack are removed when aura expires
}

std::string Au_CommandingShout::GetDescription() {
	return "Increases damage dealt by 5% and reduces\ndamage taken by 5%.";
}