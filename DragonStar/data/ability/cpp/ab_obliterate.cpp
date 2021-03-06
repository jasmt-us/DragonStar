// ==================================
//
// obliterate.cpp
//
// ==================================

#include "../header/ab_obliterate.h"

#include "../../id/abilityID.h"
#include "../../id/abilityType.h"
#include "../../id/attribute.h"
#include "../../id/category.h"
#include "../../id/element.h"
#include "../../../scene/battleScene.h"

Ab_Obliterate::Ab_Obliterate() {
	setHarmfulSpellDefault();

	name = "Obliterate";
	abilityID = AbilityID::Obliterate;
	icon = "obliterate.png";

	abilityTypes = {
		AbilityType::DarkSpell
	};
	tier = 2;

	categories = {
		Category::Damaging,
		Category::SingleTarget,
		Category::Direct,
		Category::Spell
	};

	elements = { Element::Dark };

	castTime = 100;
	range = 4;
	aiSearchRange = range;
	mpCost = 80;
	cooldown = 3000;

	requireTargetsInArea = true;
	isGroundTargeted = false;

	usage = UseType::Instant;
}

std::vector<sf::Vector3i> Ab_Obliterate::GetTargetArea(sf::Vector3i selectedTile) {
	std::vector<sf::Vector3i> area = { selectedTile };
	return area;
}

std::string Ab_Obliterate::GetDescription() {
	std::string damage = "190";

	if (user != nullptr) {
		damage = std::to_string(Combat::ScalingAttackEstimate(user, 190.0, Attribute::INT, abilityOptions));
	}

	std::string desc = "Gather an immense amount of dark energy and fire it at the target, dealing #damage " + damage + " #default dark damage.";
	desc = WordWrap::WrapString(desc, charPerLine);

	return desc;
}

void Ab_Obliterate::execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea) {
	Combat::ScalingAttack(user, targets[0], 190.0, Attribute::INT, abilityOptions);
}