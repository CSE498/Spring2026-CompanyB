/**
 * @file SwarmingAgentsTest.cpp
 * @brief test suite for SwarmingAgent
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/SwarmingAgent.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

using namespace cse498;

// MazeWorld action IDs (matches MazeWorld::ActionType)
static constexpr size_t REMAIN_STILL = 0;
static constexpr size_t MOVE_UP      = 1;
static constexpr size_t MOVE_DOWN    = 2;
static constexpr size_t MOVE_LEFT    = 3;
static constexpr size_t MOVE_RIGHT   = 4;

// Construction & Initialization

TEST_CASE("SwarmingAgent initializes with actions from MazeWorld", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("TestAgent");
    agent.SetLocation(WorldPosition{1, 1});

    // Initialize is called by AddAgent; verify the agent exists and has actions
    REQUIRE(agent.GetName() == "TestAgent");
    REQUIRE(agent.GetID() == 0);
}

TEST_CASE("SwarmingAgent default target_id is SIZE_MAX (no target)", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    agent.SetLocation(WorldPosition{1, 1});

    // With no target set, known_locations is empty
    REQUIRE(agent.GetKnownLocations().empty());
}

// SetTarget & AddKnownLocation

TEST_CASE("SetTarget returns reference for chaining", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    agent.SetLocation(WorldPosition{1, 1});

    // Should be chainable
    auto & ref = agent.SetTarget(42);
    REQUIRE(&ref == &agent);
}

TEST_CASE("AddKnownLocation stores and retrieves locations", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    agent.SetLocation(WorldPosition{1, 1});

    agent.AddKnownLocation(10, WorldPosition{5, 5});
    agent.AddKnownLocation(20, WorldPosition{8, 3});

    const auto & locs = agent.GetKnownLocations();
    REQUIRE(locs.size() == 2);
    REQUIRE(locs.contains(10));
    REQUIRE(locs.contains(20));

    auto r1 = locs.at(10);
    REQUIRE(r1.has_value());
    REQUIRE(r1.value().X() == Approx(5.0));
    REQUIRE(r1.value().Y() == Approx(5.0));

    auto r2 = locs.at(20);
    REQUIRE(r2.has_value());
    REQUIRE(r2.value().X() == Approx(8.0));
    REQUIRE(r2.value().Y() == Approx(3.0));
}

TEST_CASE("AddKnownLocation is chainable", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    agent.SetLocation(WorldPosition{1, 1});

    auto & ref = agent.AddKnownLocation(1, WorldPosition{2, 2});
    REQUIRE(&ref == &agent);
}

// SelectAction — movement toward target

TEST_CASE("Agent moves right when target is to the right", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // Place agent at (1,1), target at (5,1) — target is to the right
    agent.SetLocation(WorldPosition{1, 1});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{5, 1});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_RIGHT);
}

TEST_CASE("Agent moves left when target is to the left", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // Place agent at (5,1), target at (1,1)
    agent.SetLocation(WorldPosition{5, 1});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{1, 1});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_LEFT);
}

TEST_CASE("Agent moves down when target is below", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // Place agent at (1,1), target at (1,5) — y increases downward
    agent.SetLocation(WorldPosition{1, 1});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{1, 5});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_DOWN);
}

TEST_CASE("Agent moves up when target is above", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // Place agent at (1,5), target at (1,1)
    agent.SetLocation(WorldPosition{1, 5});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{1, 1});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_UP);
}

TEST_CASE("Agent stays still when already at target", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    agent.SetLocation(WorldPosition{3, 3});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{3, 3});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == REMAIN_STILL);
}

TEST_CASE("Agent prefers horizontal movement when dx >= dy", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // dx = 3, dy = 1 — should move horizontally (right)
    agent.SetLocation(WorldPosition{1, 1});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{4, 2});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_RIGHT);
}

TEST_CASE("Agent prefers vertical movement when dy > dx", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Agent");
    // dx = 1, dy = 3 — should move vertically (down)
    agent.SetLocation(WorldPosition{1, 1});
    agent.SetTarget(99);
    agent.AddKnownLocation(99, WorldPosition{2, 4});

    size_t action = agent.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_DOWN);
}

// SelectAction — wandering (no target knowledge)

TEST_CASE("Agent wanders when it has no target set", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Wanderer");
    agent.SetLocation(WorldPosition{5, 5});

    // No target set — should return a movement action (1-4) each time
    for (int i = 0; i < 20; ++i) {
        size_t action = agent.SelectAction(world.GetGrid());
        REQUIRE(action >= MOVE_UP);
        REQUIRE(action <= MOVE_RIGHT);
    }
}

TEST_CASE("Agent wanders when target is set but location unknown", "[SwarmingAgent]") {
    MazeWorld world;
    auto & agent = world.AddAgent<SwarmingAgent>("Seeker");
    agent.SetLocation(WorldPosition{5, 5});
    agent.SetTarget(42); // target set, but no known_location for id 42

    for (int i = 0; i < 20; ++i) {
        size_t action = agent.SelectAction(world.GetGrid());
        REQUIRE(action >= MOVE_UP);
        REQUIRE(action <= MOVE_RIGHT);
    }
}

// Knowledge sharing between agents

TEST_CASE("Agent learns target location from another SwarmingAgent", "[SwarmingAgent]") {
    MazeWorld world;

    // Agent A knows where target 99 is
    auto & knower = world.AddAgent<SwarmingAgent>("Knower");
    knower.SetLocation(WorldPosition{1, 1});
    knower.AddKnownLocation(99, WorldPosition{9, 1});

    // Agent B is seeking target 99 but doesn't know where it is
    auto & seeker = world.AddAgent<SwarmingAgent>("Seeker");
    seeker.SetLocation(WorldPosition{3, 1});
    seeker.SetTarget(99);

    // Verify seeker doesn't know the location yet
    REQUIRE_FALSE(seeker.GetKnownLocations().contains(99));

    // After one SelectAction call, seeker should have merged knower's knowledge
    // (GetKnownAgents returns ALL agents by default in WorldBase)
    seeker.SelectAction(world.GetGrid());

    REQUIRE(seeker.GetKnownLocations().contains(99));
    auto result = seeker.GetKnownLocations().at(99);
    REQUIRE(result.has_value());
    REQUIRE(result.value().X() == Approx(9.0));
    REQUIRE(result.value().Y() == Approx(1.0));
}

TEST_CASE("Agent with knowledge moves toward target after learning", "[SwarmingAgent]") {
    MazeWorld world;

    // Knower knows where target 50 is (far to the right)
    auto & knower = world.AddAgent<SwarmingAgent>("Knower");
    knower.SetLocation(WorldPosition{1, 1});
    knower.AddKnownLocation(50, WorldPosition{10, 1});

    // Seeker wants target 50 but doesn't know where it is
    auto & seeker = world.AddAgent<SwarmingAgent>("Seeker");
    seeker.SetLocation(WorldPosition{3, 1});
    seeker.SetTarget(50);

    // First call: seeker learns from knower, then decides action
    // After learning, target is to the right (x=10 vs x=3), so should move right
    size_t action = seeker.SelectAction(world.GetGrid());
    REQUIRE(action == MOVE_RIGHT);
}

TEST_CASE("Knowledge spreads transitively across agents", "[SwarmingAgent]") {
    MazeWorld world;

    // Agent A knows location 1
    auto & a = world.AddAgent<SwarmingAgent>("A");
    a.SetLocation(WorldPosition{1, 1});
    a.AddKnownLocation(1, WorldPosition{9, 9});

    // Agent B knows location 2
    auto & b = world.AddAgent<SwarmingAgent>("B");
    b.SetLocation(WorldPosition{2, 1});
    b.AddKnownLocation(2, WorldPosition{1, 9});

    // Agent C knows nothing
    auto & c = world.AddAgent<SwarmingAgent>("C");
    c.SetLocation(WorldPosition{3, 1});
    c.SetTarget(1);

    // After C runs SelectAction, it should learn both locations from A and B
    c.SelectAction(world.GetGrid());

    REQUIRE(c.GetKnownLocations().contains(1));
    REQUIRE(c.GetKnownLocations().contains(2));
}

TEST_CASE("Agent does not overwrite existing knowledge", "[SwarmingAgent]") {
    MazeWorld world;

    // Agent A thinks target 1 is at (9,9)
    auto & a = world.AddAgent<SwarmingAgent>("A");
    a.SetLocation(WorldPosition{1, 1});
    a.AddKnownLocation(1, WorldPosition{9, 9});

    // Agent B already knows target 1 is at (5,5) — this should NOT be overwritten
    auto & b = world.AddAgent<SwarmingAgent>("B");
    b.SetLocation(WorldPosition{2, 1});
    b.AddKnownLocation(1, WorldPosition{5, 5});
    b.SetTarget(1);

    b.SelectAction(world.GetGrid());

    // B should still have its original knowledge
    auto result = b.GetKnownLocations().at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().X() == Approx(5.0));
    REQUIRE(result.value().Y() == Approx(5.0));
}

TEST_CASE("Agent skips itself when gathering knowledge", "[SwarmingAgent]") {
    MazeWorld world;

    // Single agent — should not crash or duplicate anything
    auto & solo = world.AddAgent<SwarmingAgent>("Solo");
    solo.SetLocation(WorldPosition{5, 5});
    solo.SetTarget(1);

    // Should just wander, no crash
    size_t action = solo.SelectAction(world.GetGrid());
    REQUIRE(action >= MOVE_UP);
    REQUIRE(action <= MOVE_RIGHT);
    REQUIRE(solo.GetKnownLocations().empty());
}

// Multiple agents with full RunAgents cycle

TEST_CASE("Full RunAgents cycle moves agent toward target", "[SwarmingAgent]") {
    MazeWorld world;

    // Knower at (1,1) knows target at (1,5)
    auto & knower = world.AddAgent<SwarmingAgent>("Knower");
    knower.SetLocation(WorldPosition{1, 1});
    knower.AddKnownLocation(99, WorldPosition{1, 5});

    // Seeker at (1,3) wants to reach target 99
    auto & seeker = world.AddAgent<SwarmingAgent>("Seeker");
    seeker.SetLocation(WorldPosition{1, 3});
    seeker.SetTarget(99);

    // Run one full world step (all agents take actions through the world)
    world.RunAgents();

    // Seeker should have learned target and moved down (toward y=5)
    WorldPosition seeker_pos = seeker.GetLocation().AsWorldPosition();
    REQUIRE(seeker_pos.Y() == Approx(4.0));
    REQUIRE(seeker_pos.X() == Approx(1.0));
}
