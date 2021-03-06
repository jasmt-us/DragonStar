// ==================================
//
// eq_heavyBody.cpp
//
// ==================================

#include "../header/eq_heavyBody.h"

#include "../../id/abilityID.h"
#include "../../id/auraID.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../id/equipmentID.h"
#include "../../id/equipType.h"
#include "../../id/itemQuality.h"
#include "../../id/statModType.h"

Eq_IronArmor::Eq_IronArmor() {
	name = "Iron Armor";
	iconFilepath = "body.png";
	itemQuality = ItemQuality::Common;
	weight = 16;
	tier = 1;

	textureFilepath = "iron_armor.png";
	equipType = EquipType::HeavyBody;
	equipmentID = EquipmentID::IronArmor;

	equipPower = 24;
	blockChance = 0.0;
	statMods = {};
}