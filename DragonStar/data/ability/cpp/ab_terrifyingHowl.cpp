// ==================================
//
// ab_terrifyingHowl.cpp
//
// ==================================

#include "../header/ab_terrifyingHowl.h"

#include "../../id/abilityID.h"
#include "../../id/abilityType.h"
#include "../../id/attribute.h"
#include "../../id/auraID.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../../hex/hexMath.h"

Ab_TerrifyingHowl::Ab_TerrifyingHowl() {
	setAttackDefault();

	name = "Terrifying Howl";
	abilityID = AbilityID::TerrifyingHowl;
	icon = "screech.png";

	abilityTypes = {
		AbilityType::Tech
	};
	tier = 1;

	categories = {
		Category::AreaOfEffect
	};

	elements = {
		Element::Physical
	};

	castTime = 100;
	range = 1;
	aiSearchRange = 3;
	targetOutsideCursor = true;
	spCost = 10;
	cooldown = 2000;

	isAttack = false;
	requireTargetsInArea = true;
	keyTileRange = true;
	keyTile = sf::Vector3i(1, 1, -2);
	canDoubleStrike = false;

	ignoreBodyBlock = true;
	ignoreLineOfSight = true;
	areaIgnoreBodyBlock = true;
	areaIgnoreLineOfSight = false;
}

std::vector<sf::Vector3i> Ab_TerrifyingHowl::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> tiles = {
		sf::Vector3i(0, 1, -1),
		sf::Vector3i(0, 2, -2),
		sf::Vector3i(1, 1, -2),
		sf::Vector3i(1, 0, -1),
		sf::Vector3i(2, 0, -2),
		sf::Vector3i(0, 3, -3),
		sf::Vector3i(1, 2, -3),
		sf::Vector3i(2, 1, -3),
		sf::Vector3i(3, 0, -3)
	};

	rotateArea(selectedTile, tiles);

	return tiles;
}

std::string Ab_TerrifyingHowl::GetDescription() {
	std::string desc = "Reduce the damage dealt and hit chance of all enemies in a cone of length 3 by 20% for 10s.";
	desc = WordWrap::WrapString(desc, charPerLine);

	return desc;
}

void Ab_TerrifyingHowl::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	for (size_t i = 0; i < targets.size(); i++) {
		Combat::AddAuraStack(user, targets[i], AuraID::TerrifyingHowl);
	}
}