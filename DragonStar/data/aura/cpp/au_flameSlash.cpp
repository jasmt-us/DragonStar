// ==================================
//
// au_flameSlash.cpp
//
// ==================================

#include "../header/au_flameSlash.h"

#include "../../id/abilityID.h"
#include "../../id/attribute.h"
#include "../../id/auraID.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../id/statModType.h"

Au_FlameSlash::Au_FlameSlash() {
	name = "Flame Slash";
	id = AuraID::FlameSlash;
	icon = "flame_slash.png";

	categories = {
		Category::Spell
	};

	elements = {
		Element::Fire
	};

	statMods = {
		StatMod(StatModType::Damage, 0.1, { Category::Direct }, { Element::Fire }),
		StatMod(StatModType::AuraSnapshotPower, 0.1, { Category::Any },{ Element::Fire })
	};

	canCrit = false;

	baseDuration = 800;
	timePerTick = 100;
	maxStacks = 1;
	hasTicks = false;
	fixedDuration = false; // if true, ignore VIT in calculations
	unlimitedDuration = false;
	consumedOnUse = true; // if true, aura will expire when called in combat calculations
	isBuff = true;

	isStun = false;
	isDisarm = false;
	isSilence = false;
	isSlow = false;

	uniquePerActor = true; // determines if one actor can apply multiple copies of aura
	uniqueAura = false; // if true only one copy can be applied regardless of source

	removeAllStacksOnExpiration = false; // Determines whether all stacks or 1 stack are removed when aura expires
}

std::string Au_FlameSlash::GetDescription() {
	return "Increases the damage of your next fire\nability by 10%.";
}