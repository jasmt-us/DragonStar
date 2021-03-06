// ==================================
//
// ab_shock.h
//
// Data for Shock ability.
//
// ==================================

#pragma once

#include "../../ability.h"

class Ab_Shock : public Ability {
public:
	Ab_Shock();

	std::vector<sf::Vector3i> GetTargetArea(sf::Vector3i selectedTile);

	std::string GetDescription();
protected:
	void execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea);
};
