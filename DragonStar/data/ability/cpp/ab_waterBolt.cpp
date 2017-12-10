// ==================================
//
// ab_waterBolt.cpp
//
// ==================================

#include "../header/ab_waterBolt.h"

Ab_WaterBolt::Ab_WaterBolt() {
	setHarmfulSpellDefault();

	name = "Water Bolt";
	abilityID = AbilityID::WATER_BOLT;
	icon = "water_bolt.png";

	abilityTypes = {
		AbilityType::WATER_SPELL
	};
	tier = 1;

	categories = {
		Category::DAMAGING,
		Category::SINGLE_TARGET,
		Category::DIRECT,
		Category::SPELL
	};

	elements = { Element::WATER };

	castTime = 200;
	range = 4;
	aiSearchRange = range;
	mpCost = 5;

	requireTargetsInArea = true;
	isGroundTargeted = false;

	usage = UseType::CAST;
}

std::vector<sf::Vector3i> Ab_WaterBolt::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> area = { selectedTile };
	return area;
}

std::string Ab_WaterBolt::GetDescription() {
	std::string damage = "24";

	if (user != nullptr) {
		damage = std::to_string(Combat::ScalingAttackEstimate(user, 24.0, Attribute::INT, abilityOptions));
	}

	std::string desc = "Shoot a bolt of water at the target, dealing\n#damage " + damage + " #default water damage.";
	return desc;
}

void Ab_WaterBolt::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	Combat::ScalingAttack(user, targets[0], 24.0, Attribute::INT, abilityOptions);
}