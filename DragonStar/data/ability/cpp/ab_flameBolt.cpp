// ==================================
//
// ab_flameBolt.cpp
//
// ==================================

#include "../header/ab_flameBolt.h"

#include "../../id/abilityID.h"
#include "../../id/abilityType.h"
#include "../../id/attribute.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../../hex/hexMath.h"

Ab_FlameBolt::Ab_FlameBolt() {
	setHarmfulSpellDefault();

	name = "Flame Bolt";
	abilityID = AbilityID::FlameBolt;
	icon = "flame_bolt.png";

	abilityTypes = {
		AbilityType::FireSpell
	};
	tier = 1;

	categories = {
		Category::Damaging,
		Category::SingleTarget,
		Category::Direct,
		Category::Spell
	};

	elements = { Element::Fire };

	castTime = 200;
	range = 4;
	aiSearchRange = range;
	mpCost = 12;

	requireTargetsInArea = true;
	isGroundTargeted = false;

	usage = UseType::Cast;
}

std::vector<sf::Vector3i> Ab_FlameBolt::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> area = { selectedTile };
	return area;
}

std::string Ab_FlameBolt::GetDescription() {
	std::string damage = "55";

	if (user != nullptr) {
		damage = std::to_string(Combat::ScalingAttackEstimate(user, 55.0, Attribute::INT, abilityOptions));
	}

	std::string desc = "Shoot a bolt of fire at the target, dealing #damage " + damage + " #default fire damage.";
	desc = WordWrap::WrapString(desc, charPerLine);

	return desc;
}

void Ab_FlameBolt::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	Combat::ScalingAttack(user, targets[0], 55.0, Attribute::INT, abilityOptions);
}