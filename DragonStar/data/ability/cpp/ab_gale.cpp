// ==================================
//
// ab_gale.cpp
//
// ==================================

#include "../header/ab_gale.h"

#include "../../id/abilityID.h"
#include "../../id/abilityType.h"
#include "../../id/attribute.h"
#include "../../id/auraID.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../../hex/hexMath.h"

Ab_Gale::Ab_Gale() {
	setHarmfulSpellDefault();

	name = "Gale";
	abilityID = AbilityID::Gale;
	icon = "gale.png";

	abilityTypes = {
		AbilityType::WindSpell
	};
	tier = 1;

	categories = {
		Category::Damaging,
		Category::SingleTarget,
		Category::Direct,
		Category::Spell
	};

	elements = { Element::Wind };

	castTime = 200;
	range = 4;
	aiSearchRange = range;
	mpCost = 10;

	requireTargetsInArea = true;
	isGroundTargeted = false;

	usage = UseType::Cast;
}

std::vector<sf::Vector3i> Ab_Gale::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> area = { selectedTile };
	return area;
}

std::string Ab_Gale::GetDescription() {
	std::string damage = "45";

	if (user != nullptr) {
		damage = std::to_string(Combat::ScalingAttackEstimate(user, 45.0, Attribute::INT, abilityOptions));
	}

	std::string desc = "Launch a slicing gale at the target, dealing #damage " + damage + " #default wind damage. Reduces the cast time of Gale by 0.1s. Stacks up to 5 times and lasts 5s.";
	desc = WordWrap::WrapString(desc, charPerLine);

	return desc;
}

void Ab_Gale::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	auto result = Combat::ScalingAttack(user, targets[0], 45.0, Attribute::INT, abilityOptions);
	Combat::AddAuraStack(user, user, AuraID::Gale);
}