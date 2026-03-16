/**
 * @file SwarmingAgent.cpp
 * @brief Implementation of the SwarmingAgent class.
 */

#include "SwarmingAgent.hpp"
#include "../core/WorldBase.hpp"

namespace cse498 {

size_t SwarmingAgent::SelectAction(const WorldGrid & /*grid*/) {
    // get knowledge from nearby agents
    // TODO: right now we are just getting all agents, this later
    // should be changed for the Queue system we have planned
    std::vector<size_t> nearby = world.GetKnownAgents(*this);
    for (size_t agent_id : nearby) {
        if (agent_id == GetID()) continue; // skip self

        const AgentBase & other = world.GetAgent(agent_id);

        // TEMP SOLUTION
        // I believe the correct implementation is to make a GetKnownLocations
        // function in the AgentBase class that returns nothing then this overrides it
        // but I am unsure if we are allowed to change the AgentBase class, so for
        // now I am doing a dynamic cast to check if the other agent is a SwarmingAgent
        // and then accessing its known locations that way. This is not ideal but
        // it works for now.

        const SwarmingAgent * swarmer = dynamic_cast<const SwarmingAgent *>(&other);
        if (!swarmer) continue;

        // Merge their knowledge into ours
        const auto & other_locations = swarmer->GetKnownLocations();
        for (auto it = other_locations.begin(); it != other_locations.end(); ++it) {
            const auto & entry = *it;
            if (!known_locations.contains(entry.key)) {
                known_locations.insert(entry.key, entry.value);
            }
        }
    }

    // CASE 2
    // We are aware we have a target AND we know the target location
    // We can then advance towards it
    if (target_id != SIZE_MAX &&
         known_locations.contains(target_id))
    {
        auto result = known_locations.at(target_id);
        if (result.has_value()) {
            WorldPosition target_pos = result.value();
            WorldPosition my_pos = GetLocation().AsWorldPosition();

            double dx = target_pos.X() - my_pos.X();
            double dy = target_pos.Y() - my_pos.Y();

            // move along the axis with greater distance first
            // later we should account for queue size as well
            if (std::abs(dx) >= std::abs(dy)) {
                if (dx > 0) return GetActionID("right");
                if (dx < 0) return GetActionID("left");
            } else {
                if (dy > 0) return GetActionID("down");
                if (dy < 0) return GetActionID("up");
            }

            // Already at the target — stay put
            return 0;
        }
    }

    // CASE 3
    // We do not know where our target is yet
    // Wander randomly to find it
    std::uniform_int_distribution<int> dist(1, 4);
    switch (dist(rng)) {
        case 1: return GetActionID("up");
        case 2: return GetActionID("down");
        case 3: return GetActionID("left");
        case 4: return GetActionID("right");
    }

    return 0;
}

} // namespace cse498
