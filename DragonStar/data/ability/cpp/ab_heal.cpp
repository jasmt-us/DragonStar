// ==================================
//
// ab_heal.cpp
//
// ==================================

#include "../header/ab_heal.h"

#include "../../id/abilityID.h"
#include "../../id/abilityType.h"
#include "../../id/attribute.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../../hex/hexMath.h"

Ab_Heal::Ab_Heal() {
	setHelpfulSpellDefault();

	name = "Heal";
	abilityID = AbilityID::Heal;
	icon = "heal.png";

	abilityTypes = {
		AbilityType::DivineHeal
	};
	tier = 1;

	categories = {
		Category::Direct,
		Category::Healing,
		Category::SingleTarget,
		Category::Spell
	};

	elements = { Element::DivineHeal };

	castTime = 250;
	range = 5;
	aiSearchRange = range;
	mpCost = 25;

	usage = UseType::Cast;
}

std::vector<sf::Vector3i> Ab_Heal::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> area = { selectedTile };
	return area;
}

std::string Ab_Heal::GetDescription() {
	std::string healing = "80";

	if (user != nullptr) {
		healing = std::to_string(Combat::ScalingHealEstimate(user, 80.0, Attribute::HP, abilityOptions));
	}

	std::string desc = "Heal an ally with divine light, restoring #heal " + healing + " #default HP.";
	desc = WordWrap::WrapString(desc, charPerLine);

	return desc;
}

void Ab_Heal::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	Combat::ScalingHeal(user, targets[0], 80.0, Attribute::HP, abilityOptions);
}