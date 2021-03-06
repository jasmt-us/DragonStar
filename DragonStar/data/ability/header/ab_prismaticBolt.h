// ==================================
//
// ab_prismaticBolt.h
//
// Data for Prismatic Bolt ability.
//
// ==================================

#include "../../ability.h"

#pragma once

class Ab_PrismaticBolt : public Ability {
public:
	Ab_PrismaticBolt();

	std::vector<sf::Vector3i> GetTargetArea(sf::Vector3i selectedTile);

	std::string GetDescription();
protected:
	void execute(std::vector<ActorPtr>& targets, sf::Vector3i cursor, std::vector<sf::Vector3i> targetArea);
};