// ==================================
//
// combat.cpp
//
// ==================================

#include "combat.h"

#include "random.h"
#include "../data/dataString.h"
#include "../data/id/attribute.h"
#include "../data/id/category.h"
#include "../data/id/element.h"
#include "../data/id/equipType.h"
#include "../data/id/eventType.h"
#include "../scene/battleScene.h"
#include "../entity/actor.h"
#include "../entity/statMath.h"
#include "../ui/messageLog.h"


// Hit roll.
static bool rollHit(ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	if (eventOptions.CanDodge) {
		double hitChance = user->GetHitChance(eventOptions, true) + eventOptions.BonusHitChance;

		double dodgeChance = target->GetDodgeChance(eventOptions, true);

		hitChance -= dodgeChance;

		return hitChance > 0.0 && hitChance >= Random::RandDouble(0.0, 1.0);
	}
	else {
		return true;
	}
}


// Crit roll.
static bool rollCrit(ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	if (eventOptions.CanCrit) {
		double critChance = eventOptions.BonusCritChance;

		critChance += user->GetCritChance(eventOptions, true);
		critChance -= target->GetCritChanceProtection(eventOptions, true);

		return critChance > 0.0 && critChance >= Random::RandDouble(0.0, 1.0);
	}
	else {
		return false;
	}
}

// Crit damage.
static void applyCrit(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	double critMultiplier = user->GetCritDamage(eventOptions, true) * eventOptions.BonusCritDamage;
	critMultiplier -= 1.0;
	critMultiplier = critMultiplier * target->GetCritDamageProtection(eventOptions, true);
	critMultiplier += 1.0;
	value = value * critMultiplier;
}

// Block roll.
static bool rollBlock(ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	if (eventOptions.CanBlock) {
		double blockChance = target->GetBlockChance(eventOptions, true);
		return blockChance > 0.0 && blockChance >= Random::RandDouble(0.0, 1.0);
	}
	else {
		return false;
	}
}

// Block damage reduction.
static void applyBlock(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	value *= 0.7;
}

// Armor damage reduction.
static void applyArmor(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	int targetArmor = target->GetArmor(true);
	double armorMitigation = 0.0;
	targetArmor = targetArmor * (1.0 - (target->GetArmorPen(eventOptions, true) + eventOptions.BonusArmorPen));

	if (targetArmor > 0) {
		armorMitigation = (double)targetArmor / (double)(targetArmor + CalcExpStat(50, user->GetLevel()));
	}

	value = value * (1.0 - armorMitigation);
}

// Apply resistance mitigation
static void applyResistance(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	double resistance = target->GetResistance(eventOptions, true);

	// Resistance capped at 100%.
	if (resistance > 1.0) {
		resistance = 1.0;
	}

	// Get resistance penetration.
	if (resistance > 0.0) {
		resistance = resistance * (1.0 - target->GetResistancePen(eventOptions, true));
	}

	value = value * (1.0 - resistance);
}

// Apply damage taken bonus.
static void applyDamageTaken(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	double damageTaken = target->GetDamageTaken(eventOptions, true);

	// Damage taken cant be negative.
	if (damageTaken < 0.0) {
		damageTaken = 0.0;
	}

	value *= damageTaken;
}

// Apply healing taken bonus.
static void applyHealingTaken(double& value, ActorPtr& user, ActorPtr& target, EventOptions eventOptions) {
	double healingTaken = target->GetHealingTaken(eventOptions, true);

	if (healingTaken < 0.0) {
		healingTaken = 0.0;
	}

	value *= healingTaken;
}

// Applies leech effects.
static void applyLeech(ActorPtr& user, EventOptions eventOptions, EventResult& eventResult) {
	double lifeLeech = eventOptions.BonusLifeLeech + user->GetHPLeech(eventOptions, true);
	double manaLeech = eventOptions.BonusManaLeech + user->GetMPLeech(eventOptions, true);
	double staminaLeech = eventOptions.BonusStaminaLeech;

	int hp = std::floor(eventResult.ResultValue * lifeLeech);
	int mp = std::floor(eventResult.ResultValue * manaLeech);
	int sp = std::floor(eventResult.ResultValue * staminaLeech);

	if (hp > 0) {
		eventResult.LifeLeeched = user->HealActor(hp, { Element::None }, Attribute::HP, false);
	}
	if (mp > 0) {
		eventResult.ManaLeeched = user->HealActor(mp, { Element::None }, Attribute::MP, false);
	}
	if (sp > 0) {
		eventResult.StaminaLeeched = user->HealActor(sp, { Element::None }, Attribute::SP, false);
	}
}

// Prints damage message.
static void outputDamage(ActorPtr& user, ActorPtr& target, EventOptions eventOptions, EventResult eventResult) {
	std::string output;
	if (user->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += user->GetName();
	output += " #aaaaaa > #default ";
	output += eventOptions.EventName;
	output += " #aaaaaa > ";
	if (target->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += target->GetName();
	output += " #aaaaaa > #damage ";
	if (eventResult.DidCrit) {
		output += "\\*" + std::to_string(eventResult.ResultValue) + "\\*";
	}
	else {
		output += std::to_string(eventResult.ResultValue);
	}
	output += " #default " + DataString::ElementString(eventOptions.Elements, true);

	if (eventResult.DidBlock) {
		output += " (Blocked)";
	}

	if (eventResult.LifeLeeched > 0 || eventResult.ManaLeeched > 0 || eventResult.StaminaLeeched > 0) {
		if (!eventResult.DidBlock) {
			output += " ";
		}
		output += "(Leeched";
		if (eventResult.LifeLeeched > 0) {
			output += " #heal " + std::to_string(eventResult.LifeLeeched) + " HP";
		}
		if (eventResult.ManaLeeched > 0) {
			output += " #spell " + std::to_string(eventResult.ManaLeeched) + " MP";
		}
		if (eventResult.StaminaLeeched > 0) {
			output += " #skill " + std::to_string(eventResult.StaminaLeeched) + " SP";
		}
		output += "#default )";
	}

	messageLog.AddMessage(output);
}

// Prints Healing message.
static void outputHealing(ActorPtr& user, ActorPtr& target, EventOptions eventOptions, EventResult eventResult) {
	std::string output;
	if (user->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += user->GetName();
	output += " #aaaaaa > #default ";
	output += eventOptions.EventName;
	output += " #aaaaaa > ";
	if (target->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += target->GetName();
	output += " #aaaaaa > #heal ";
	if (eventResult.DidCrit) {
		output += "\\*" + std::to_string(eventResult.ResultValue) + "\\*";
	}
	else {
		output += std::to_string(eventResult.ResultValue);
	}
	output += " #default " + DataString::ElementString(eventOptions.Elements, true);

	messageLog.AddMessage(output);
}

// Prints miss message.
static void outputMiss(ActorPtr& user, ActorPtr& target, EventOptions eventOptions, EventResult eventResult) {
	std::string output;
	if (user->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += user->GetName();
	output += " #aaaaaa > #default ";
	output += eventOptions.EventName;
	output += " #aaaaaa > ";
	if (target->IsPlayer()) {
		output += "#blue ";
	}
	else {
		output += "#red ";
	}
	output += target->GetName();
	output += " #aaaaaa > Miss";

	messageLog.AddMessage(output);
}

EventResult Combat::WeaponAttack(ActorPtr& user, ActorPtr& target, double multiplier, bool isOffHand, EventOptions eventOptions) {
	EventResult eventResult;
	double damage = 0;

	if (!isOffHand || isOffHand && user->IsDualWielding()) {
		user->OnEvent(EventType::PreCalc, target, eventOptions, eventResult, isOffHand, damage);
		eventResult.DidHit = rollHit(user, target, eventOptions);

		if (eventResult.DidHit) {
			eventResult.DidCrit = rollCrit(user, target, eventOptions);
			eventResult.DidBlock = rollBlock(user, target, eventOptions);

			// Actual damage calculation.

			// If the skill requires a certain weapon type and the proper weapon is in the off-hand, flip main hand / off-hand attacks. BUGGY
			//if (eventOptions.AllowedWeaponTypes.size() > 0 && std::find(eventOptions.AllowedWeaponTypes.begin(), eventOptions.AllowedWeaponTypes.end(), user->GetMainHandEquipType()) == eventOptions.AllowedWeaponTypes.end()) {
			//	isOffHand = !isOffHand;
			//}

			// Get weapon damage.
			if (!isOffHand) {
				damage = user->GetMainHandDamage(true);

				// Add weapon categories to attack.
				std::vector<Category> weaponCategories = user->GetMainHandCategories();
				for (size_t i = 0; i < weaponCategories.size(); i++) {
					eventOptions.Categories.push_back(weaponCategories[i]);
				}

				// If the attack has no elements, use the weapon's elements.
				if (eventOptions.Elements.size() == 0) {
					std::vector<Element> weaponElements = user->GetMainHandElements();
					for (size_t i = 0; i < weaponElements.size(); i++) {
						eventOptions.Elements.push_back(weaponElements[i]);
					}
				}
			}
			// Off-hand attack.
			else {
				damage = user->GetOffHandDamage(true);

				// Add weapon categories to attack.
				std::vector<Category> weaponCategories = user->GetOffHandCategories();
				for (size_t i = 0; i < weaponCategories.size(); i++) {
					eventOptions.Categories.push_back(weaponCategories[i]);
				}

				// If the attack has no elements, use the weapon's elements.
				if (eventOptions.Elements.size() == 0) {
					std::vector<Element> weaponElements = user->GetOffHandElements();
					for (size_t i = 0; i < weaponElements.size(); i++) {
						eventOptions.Elements.push_back(weaponElements[i]);
					}
				}
			}

			// damage multipliers
			damage *= user->GetDamageDealt(eventOptions, true);

			// weapon damage multiplier
			damage = damage * multiplier;

			if (eventResult.DidCrit) {
				applyCrit(damage, user, target, eventOptions);
			}

			if (eventResult.DidBlock) {
				applyBlock(damage, user, target, eventOptions);
			}

			applyArmor(damage, user, target, eventOptions);
			applyResistance(damage, user, target, eventOptions);
			applyDamageTaken(damage, user, target, eventOptions);

			user->OnEvent(EventType::PostCalc, target, eventOptions, eventResult, isOffHand, damage);

			// Deals damage to the actor and records amount of damage dealt.
			eventResult.ResultValue = target->DamageActor(damage, eventOptions.Elements, eventResult.DidCrit);

			// Apply leach effects after damage is dealt.
			applyLeech(user, eventOptions, eventResult);

			outputDamage(user, target, eventOptions, eventResult);

			if (target->GetCurrentHP() == 0 && eventResult.ResultValue > 0) {
				eventResult.DidKill = true;
				target->OnEvent(EventType::Death, user, eventOptions, eventResult, isOffHand, damage);
			}

			user->OnEvent(EventType::Attack, target, eventOptions, eventResult, isOffHand, damage);
			user->OnEvent(EventType::Hit, target, eventOptions, eventResult, isOffHand, damage);
			target->OnEvent(EventType::Attacked, user, eventOptions, eventResult, isOffHand, damage);

			eventResult.DidEvent = true;

		}
		// Missed attacks cannot be blocked, crit, or trigger double strike.
		else {
			eventResult.DidEvent = true;
			eventResult.DidBlock = false;
			eventResult.DidCrit = false;
			eventResult.DidDoubleStrike = false;
			eventResult.DidKill = false;
			eventResult.ResultValue = 0;

			outputMiss(user, target, eventOptions, eventResult);

			user->OnEvent(EventType::Hit, target, eventOptions, eventResult, isOffHand, damage);
			target->OnEvent(EventType::Attacked, user, eventOptions, eventResult, isOffHand, damage);
		}

		// todo: counter roll
	}
	// If attempting off-hand attack with no off-hand weapon.
	else {
		eventResult.DidEvent = false;
		eventResult.DidBlock = false;
		eventResult.DidCrit = false;
		eventResult.DidDoubleStrike = false;
		eventResult.DidKill = false;
		eventResult.ResultValue = 0;
	}

	return eventResult;
}

EventResult Combat::ScalingAttack(ActorPtr& user, ActorPtr& target, double baseDamage, Attribute attribute, EventOptions eventOptions) {
	EventResult eventResult;
	double damage = baseDamage;

	user->OnEvent(EventType::PreCalc, target, eventOptions, eventResult, false, damage);

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;

	double scaleFactor = baseDamage / 50.0;
	double attributeValue = 0;

	// todo: get attribute values
	switch (attribute) {
	case Attribute::STR:
		attributeValue = user->GetSTR(true);
		break;
	case Attribute::DEX:
		attributeValue = user->GetDEX(true);
		break;
	case Attribute::INT:
		attributeValue = user->GetINT(true);
		break;
	default:
		attributeValue = 0.0;
		break;
	}

	eventResult.DidCrit = rollCrit(user, target, eventOptions);

	// Damage Calculation
	damage = CalcExpDoubleValue(baseDamage, user->GetLevel());

	damage += attributeValue * scaleFactor;

	// damage multipliers
	damage *= user->GetDamageDealt(eventOptions, true);

	if (eventResult.DidCrit) {
		applyCrit(damage, user, target, eventOptions);
	}

	applyResistance(damage, user, target, eventOptions);
	applyDamageTaken(damage, user, target, eventOptions);

	user->OnEvent(EventType::PostCalc, target, eventOptions, eventResult, false, damage);

	// Deals damage to the actor and records amount of damage dealt.
	eventResult.ResultValue = target->DamageActor(damage, eventOptions.Elements, eventResult.DidCrit);

	// Apply leach effects after damage is dealt.
	applyLeech(user, eventOptions, eventResult);

	outputDamage(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, damage);
	}

	user->OnEvent(EventType::Hit, target, eventOptions, eventResult, false, damage);
	target->OnEvent(EventType::Attacked, user, eventOptions, eventResult, false, damage);

	return eventResult;
}

EventResult Combat::FixedAttack(ActorPtr& user, ActorPtr& target, double baseDamage, EventOptions eventOptions) {
	EventResult eventResult;
	double damage = baseDamage;

	user->OnEvent(EventType::PreCalc, target, eventOptions, eventResult, false, damage);

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;

	// Crit roll.
	eventResult.DidCrit = rollCrit(user, target, eventOptions);

	// damage multipliers
	damage *= user->GetDamageDealt(eventOptions, true);

	if (eventResult.DidCrit) {
		applyCrit(damage, user, target, eventOptions);
	}

	applyResistance(damage, user, target, eventOptions);
	applyDamageTaken(damage, user, target, eventOptions);

	user->OnEvent(EventType::PostCalc, target, eventOptions, eventResult, false, damage);

	// Deals damage to the actor and records amount of damage dealt.
	eventResult.ResultValue = target->DamageActor(damage, eventOptions.Elements, eventResult.DidCrit);

	// Apply leach effects after damage is dealt.
	applyLeech(user, eventOptions, eventResult);

	outputDamage(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, damage);
	}

	user->OnEvent(EventType::Hit, target, eventOptions, eventResult, false, damage);
	target->OnEvent(EventType::Attack, user, eventOptions, eventResult, false, damage);

	return eventResult;
}

EventResult Combat::AbsoluteAttack(ActorPtr& user, ActorPtr& target, double baseDamage, EventOptions eventOptions) {
	EventResult eventResult;

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;
	eventResult.DidCrit = false;

	double damage = baseDamage;

	// Deals damage to the actor and records amount of damage dealt.
	eventResult.ResultValue = target->DamageActor(damage, eventOptions.Elements, eventResult.DidCrit);

	// Apply leach effects after damage is dealt.
	applyLeech(user, eventOptions, eventResult);

	outputDamage(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, damage);
	}

	return eventResult;
}

EventResult Combat::OnHit(ActorPtr& user, ActorPtr& target, double baseDamage, EventOptions eventOptions) {
	EventResult eventResult;

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;
	eventResult.DidCrit = false;

	double damage = baseDamage;

	// damage multipliers
	damage *= user->GetDamageDealt(eventOptions, true);

	applyResistance(damage, user, target, eventOptions);
	applyDamageTaken(damage, user, target, eventOptions);

	// Deals damage to the actor and records amount of damage dealt.
	eventResult.ResultValue = target->DamageActor(damage, eventOptions.Elements, eventResult.DidCrit);

	// Apply leach effects after damage is dealt.
	applyLeech(user, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, damage);
	}

	return eventResult;
}

EventResult Combat::OnHeal(ActorPtr& user, ActorPtr& target, double healAmount, EventOptions eventOptions) {
	EventResult eventResult;

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;
	eventResult.DidCrit = false;

	double heal = healAmount;

	// healing multipliers
	heal *= user->GetHealingDealt(eventOptions, true);

	applyResistance(heal, user, target, eventOptions);
	applyHealingTaken(heal, user, target, eventOptions);

	// Heals the actor and records amount of healing.
	eventResult.ResultValue = target->HealActor(heal, eventOptions.Elements, Attribute::HP, eventResult.DidCrit);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, heal);
	}

	return eventResult;
}

EventResult Combat::ScalingHeal(ActorPtr& user, ActorPtr& target, double healAmount, Attribute attribute, EventOptions eventOptions) {
	EventResult eventResult;
	double heal = healAmount;

	user->OnEvent(EventType::PreCalcHeal, target, eventOptions, eventResult, false, heal);

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;

	double scaleFactor = healAmount / 50.0;
	double attributeValue = user->GetWIS(true);

	// Crit roll.
	eventResult.DidCrit = rollCrit(user, target, eventOptions);

	// Damage Calculation
	heal = CalcExpDoubleValue(healAmount, user->GetLevel());

	heal += attributeValue * scaleFactor;

	// Healing dealt.
	heal = heal * user->GetHealingDealt(eventOptions, true);

	if (eventResult.DidCrit) {
		applyCrit(heal, user, target, eventOptions);
	}

	applyHealingTaken(heal, user, target, eventOptions);
	applyResistance(heal, user, target, eventOptions);

	user->OnEvent(EventType::PostCalcHeal, target, eventOptions, eventResult, false, heal);

	// Heals actor and records amount healed.
	eventResult.ResultValue = target->HealActor(heal, eventOptions.Elements, attribute, eventResult.DidCrit);

	outputHealing(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, heal);
	}

	eventResult.DidEvent = true;

	user->OnEvent(EventType::Heal, target, eventOptions, eventResult, false, heal);
	target->OnEvent(EventType::Healed, user, eventOptions, eventResult, false, heal);

	return eventResult;
}

EventResult Combat::FixedHeal(ActorPtr& user, ActorPtr& target, double healAmount, Attribute attribute, EventOptions eventOptions) {
	EventResult eventResult;
	double heal = healAmount;

	user->OnEvent(EventType::PreCalcHeal, target, eventOptions, eventResult, false, heal);

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;

	eventResult.DidCrit = rollCrit(user, target, eventOptions);

	// Healing dealt.
	heal = heal * user->GetHealingDealt(eventOptions, true);

	if (eventResult.DidCrit) {
		applyCrit(heal, user, target, eventOptions);
	}

	applyHealingTaken(heal, user, target, eventOptions);
	applyResistance(heal, user, target, eventOptions);

	user->OnEvent(EventType::PostCalcHeal, target, eventOptions, eventResult, false, heal);

	// Heals actor and records amount healed.
	eventResult.ResultValue = target->HealActor(heal, eventOptions.Elements, attribute, eventResult.DidCrit);

	outputHealing(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, heal);
	}
	eventResult.DidEvent = true;

	user->OnEvent(EventType::Heal, target, eventOptions, eventResult, false, heal);
	target->OnEvent(EventType::Healed, user, eventOptions, eventResult, false, heal);

	return eventResult;
}

EventResult Combat::AbsoluteHeal(ActorPtr& user, ActorPtr& target, double healAmount, Attribute attribute, EventOptions eventOptions) {
	EventResult eventResult;

	// cannot miss, be blocked, or double strike
	eventResult.DidEvent = false;
	eventResult.DidHit = true;
	eventResult.DidBlock = false;
	eventResult.DidDoubleStrike = false;
	eventResult.DidCrit = false;

	double heal = healAmount;

	// Heals actor and records amount healed.
	eventResult.ResultValue = target->HealActor(heal, eventOptions.Elements, attribute, eventResult.DidCrit);

	outputHealing(user, target, eventOptions, eventResult);

	eventResult.DidKill = target->GetCurrentHP() == 0;
	if (eventResult.DidKill && eventResult.ResultValue > 0) {
		target->OnEvent(EventType::Death, user, eventOptions, eventResult, false, heal);
	}

	eventResult.DidEvent = true;

	return eventResult;
}

EventResult Combat::AddAuraStack(ActorPtr& user, ActorPtr& target, AuraID auraID) {
	EventResult eventResult;
	eventResult.DidEvent = target->AddAuraStack(user, auraID);

	return eventResult;
}

void Combat::CreateGroundEffect(ActorPtr& user, BattleScene* battleScene, AuraID auraID, sf::Vector3i position) {
	battleScene->CreateGroundEffect(user, auraID, position);
}

int Combat::WeaponAttackEstimate(ActorPtr& user, double multiplier, bool isOffHand, EventOptions eventOptions) {
	double damage = 0;

	// If the skill requires a certain weapon type and the proper weapon is in the off-hand, flip main hand / off-hand attacks. BUGGY
	//if (eventOptions.AllowedWeaponTypes.size() > 0 && std::find(eventOptions.AllowedWeaponTypes.begin(), eventOptions.AllowedWeaponTypes.end(), user->GetMainHandEquipType()) == eventOptions.AllowedWeaponTypes.end()) {
	//	isOffHand = !isOffHand;
	//}

	// Get weapon damage.
	if (!isOffHand) {
		damage = user->GetMainHandDamage(false);

		// Add weapon categories to attack.
		std::vector<Category> weaponCategories = user->GetMainHandCategories();
		for (size_t i = 0; i < weaponCategories.size(); i++) {
			eventOptions.Categories.push_back(weaponCategories[i]);
		}

		// If the attack has no elements, use the weapon's elements.
		if (eventOptions.Elements.size() == 0) {
			std::vector<Element> weaponElements = user->GetMainHandElements();
			for (size_t i = 0; i < weaponElements.size(); i++) {
				eventOptions.Elements.push_back(weaponElements[i]);
			}
		}
	}
	else {
		damage = user->GetOffHandDamage(false);

		// Add weapon categories to attack.
		std::vector<Category> weaponCategories = user->GetOffHandCategories();
		for (size_t i = 0; i < weaponCategories.size(); i++) {
			eventOptions.Categories.push_back(weaponCategories[i]);
		}

		// If the attack has no elements, use the weapon's elements.
		if (eventOptions.Elements.size() == 0) {
			std::vector<Element> weaponElements = user->GetOffHandElements();
			for (size_t i = 0; i < weaponElements.size(); i++) {
				eventOptions.Elements.push_back(weaponElements[i]);
			}
		}
	}

	// damage multipliers
	damage *= user->GetDamageDealt(eventOptions, false);

	// weapon damage multiplier
	damage = damage * multiplier;

	return (int)std::floor(damage);
}

int Combat::ScalingAttackEstimate(ActorPtr& user, double baseDamage, Attribute attribute, EventOptions eventOptions) {
	double damage = baseDamage;
	double scaleFactor = baseDamage / 50.0;
	double attributeValue = 0;

	// todo: get attribute values
	switch (attribute) {
	case Attribute::STR:
		attributeValue = user->GetSTR(false);
		break;
	case Attribute::DEX:
		attributeValue = user->GetDEX(false);
		break;
	case Attribute::INT:
		attributeValue = user->GetINT(false);
		break;
	default:
		attributeValue = 0.0;
		break;
	}

	// Damage Calculation
	damage = CalcExpDoubleValue(baseDamage, user->GetLevel());

	damage += attributeValue * scaleFactor;

	// damage multipliers
	damage *= user->GetDamageDealt(eventOptions, false);

	return (int)std::floor(damage);
}

int Combat::ScalingHealEstimate(ActorPtr & user, double healAmount, Attribute attribute, EventOptions eventOptions) {
	double heal = healAmount;
	double scaleFactor = healAmount / 50.0;
	double attributeValue = user->GetWIS(false);

	// Damage Calculation
	heal = CalcExpDoubleValue(healAmount, user->GetLevel());

	heal += attributeValue * scaleFactor;

	// healing multiplier
	heal *= user->GetHealingDealt(eventOptions, false);

	return (int)std::floor(heal);
}