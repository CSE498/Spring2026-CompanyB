/**
 * @file TrafficWorldTest.cpp
 * @brief Catch2 unit tests for TrafficWorld.
 */

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include "../../source/Agents/DrivingAgent.hpp"
#include "../../source/Worlds/TrafficWorld.hpp"

using namespace cse498;

// Grid symbols are the same as in TrafficWorld.hpp
// '#'=grass '.'=road '|'=traffic_light_vertical
// '-'=traffic_light_horizontal 'S'=spawn 'D'=destination

static const std::vector<std::string> kMinimalGrid = {
    "#########",
    "#S..|..D#",
    "#########",
};
static const std::vector<std::string> kAllGrassGrid  = {"####","####","####"};
static const std::vector<std::string> kNoSpawnerGrid = {"#######","#..D..#","#######"};
static const std::vector<std::string> kNoDestGrid    = {"#######","#S....#","#######"};
static const std::vector<std::string> kTinyGrid      = {"####","#SD#","####"};
static const std::vector<std::string> kLineGrid      = {"#####","#...#","#####"};
static const std::vector<std::string> kMultiGrid     = {
    "###########", "#S..|..D.S#", "#.........#", "#D..-..S.D#", "###########",
};
static const std::vector<std::string> kCrossGrid = {
    "#####","##.##","#...#","##.##","#####",
};

static std::string WriteTempGrid(const std::vector<std::string> &rows,
                                 const std::string &name = "_tw_tmp.txt") {
    std::ofstream f(name);
    for (const auto &r : rows) f << r << '\n';
    return name;
}

// Construction
TEST_CASE("Construction from vector", "[TrafficWorld][construction]") {
    CHECK_NOTHROW(TrafficWorld{kMinimalGrid});
    CHECK_NOTHROW(TrafficWorld{kAllGrassGrid});
    CHECK_NOTHROW(TrafficWorld{kNoSpawnerGrid});
    CHECK_NOTHROW(TrafficWorld{kNoDestGrid});
    CHECK_NOTHROW(TrafficWorld{kMultiGrid});
    CHECK_NOTHROW(TrafficWorld{std::vector<std::string>{"#"}});
    TrafficWorld tw{kAllGrassGrid};
    CHECK(tw.GetDestinationColour(WorldPosition{0, 0}).empty());
}

TEST_CASE("Construction from file", "[TrafficWorld][construction]") {
    auto path = WriteTempGrid(kMinimalGrid, "_tw_file.txt");
    CHECK_NOTHROW(TrafficWorld{path});
    TrafficWorld from_vec{kMinimalGrid};
    TrafficWorld from_file{path};
    std::remove(path.c_str());
    // 'D' is at col 7, row 1 in kMinimalGrid
    CHECK(from_vec.GetDestinationColour(WorldPosition{7, 1}) ==
          from_file.GetDestinationColour(WorldPosition{7, 1}));
}

// GetDestinationColour
TEST_CASE("GetDestinationColour", "[TrafficWorld][accessors]") {
    TrafficWorld tw{kMinimalGrid};
    WorldPosition dest{7, 1};

    CHECK_FALSE(tw.GetDestinationColour(dest).empty());
    CHECK(tw.GetDestinationColour(WorldPosition{2, 1}).empty()); // road
    CHECK(tw.GetDestinationColour(WorldPosition{0, 0}).empty()); // grass
    CHECK(tw.GetDestinationColour(WorldPosition{999, 999}).empty()); // out of bounds
    CHECK(tw.GetDestinationColour(dest) == tw.GetDestinationColour(dest)); // stable

    TrafficWorld multi{kMultiGrid};
    CHECK_FALSE(multi.GetDestinationColour(WorldPosition{7, 1}).empty());
    CHECK_FALSE(multi.GetDestinationColour(WorldPosition{1, 3}).empty());
}

// AgentExistsAt / DirectionOfDrivingAgentAt
TEST_CASE("AgentExistsAt", "[TrafficWorld][agents]") {
    TrafficWorld tw{kLineGrid};
    CHECK_FALSE(tw.AgentExistsAt(WorldPosition{1, 1}));

    auto &a = tw.AddAgent<DrivingAgent>("A");
    auto &b = tw.AddAgent<DrivingAgent>("B");
    a.SetLocation(WorldPosition{1, 1});
    b.SetLocation(WorldPosition{3, 1});
    CHECK(tw.AgentExistsAt(WorldPosition{1, 1}));
    CHECK(tw.AgentExistsAt(WorldPosition{3, 1}));
    CHECK_FALSE(tw.AgentExistsAt(WorldPosition{2, 1}));
}

TEST_CASE("DirectionOfDrivingAgentAt", "[TrafficWorld][agents]") {
    TrafficWorld tw{kMinimalGrid};
    WorldPosition pos{2, 1};
    CHECK_FALSE(tw.DirectionOfDrivingAgentAt(pos).has_value());

    auto &agent = tw.AddAgent<DrivingAgent>("Car");
    agent.SetLocation(pos);
    agent.SetDirection(Direction::East);
    REQUIRE(tw.DirectionOfDrivingAgentAt(pos).value() == Direction::East);

    agent.SetDirection(Direction::West);
    CHECK(tw.DirectionOfDrivingAgentAt(pos).value() == Direction::West);

    // Agents that reached their destination return nullopt
    agent.SetReachedDestination(true);
    CHECK_FALSE(tw.DirectionOfDrivingAgentAt(pos).has_value());
}

// DoAction for movement
TEST_CASE("DoAction — movement", "[TrafficWorld][movement]") {
    TrafficWorld tw{kMinimalGrid};
    auto &agent = tw.AddAgent<DrivingAgent>("Car");

    SECTION("REMAIN_STILL returns 1, position unchanged") {
        WorldPosition pos{2, 1};
        agent.SetLocation(pos);
        CHECK(tw.DoAction(agent, 0 /*REMAIN_STILL*/) == 1);
        CHECK(agent.GetLocation().AsWorldPosition() == pos);
    }
    SECTION("move onto road succeeds") {
        agent.SetLocation(WorldPosition{2, 1});
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 1);
        CHECK(agent.GetLocation().AsWorldPosition() == WorldPosition{3, 1});
    }
    SECTION("move into grass is blocked") {
        WorldPosition start{2, 1};
        agent.SetLocation(start);
        CHECK(tw.DoAction(agent, 1 /*MOVE_UP*/) == 0);
        CHECK(agent.GetLocation().AsWorldPosition() == start);
    }
    SECTION("all four cardinal directions work on open cross") {
        TrafficWorld cross{kCrossGrid};
        auto &c = cross.AddAgent<DrivingAgent>("C");
        WorldPosition centre{2, 2};
        c.SetLocation(centre); CHECK(cross.DoAction(c, 1 /*UP*/)    == 1);
        c.SetLocation(centre); CHECK(cross.DoAction(c, 2 /*DOWN*/)  == 1);
        c.SetLocation(centre); CHECK(cross.DoAction(c, 3 /*LEFT*/)  == 1);
        c.SetLocation(centre); CHECK(cross.DoAction(c, 4 /*RIGHT*/) == 1);
    }
}

// DoAction for traffic lights
TEST_CASE("DoAction — traffic lights", "[TrafficWorld][traffic_lights]") {
    SECTION("vertical light blocks horizontal movement") {
        TrafficWorld tw{kMinimalGrid};
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{3, 1}); // road, left of '|' at col 4
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 0);
        CHECK(agent.GetLocation().AsWorldPosition() == WorldPosition{3, 1});
    }
    SECTION("horizontal light allows horizontal movement") {
        // '-' at col 2, row 1; initial phase ALLOW_VERTICAL means '-' cells
        // permit horizontal movement
        static const std::vector<std::string> kH = {"#####","#.-.#","#####"};
        TrafficWorld tw{kH};
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{1, 1});
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 1);
        CHECK(agent.GetLocation().AsWorldPosition() == WorldPosition{2, 1});
    }
}

// DoAction for collision detection
TEST_CASE("DoAction — collision", "[TrafficWorld][collision]") {
    SECTION("same-direction agent ahead blocks movement") {
        TrafficWorld tw{kLineGrid};
        auto &front = tw.AddAgent<DrivingAgent>("Front");
        front.SetLocation(WorldPosition{3, 1});
        front.SetDirection(Direction::East);
        auto &rear = tw.AddAgent<DrivingAgent>("Rear");
        rear.SetLocation(WorldPosition{2, 1});
        rear.SetDirection(Direction::East);
        CHECK(tw.DoAction(rear, 4 /*MOVE_RIGHT*/) == 0);
    }
    SECTION("perpendicular agent blocks movement") {
        TrafficWorld tw{kLineGrid};
        auto &blocker = tw.AddAgent<DrivingAgent>("Blocker");
        blocker.SetLocation(WorldPosition{3, 1});
        blocker.SetDirection(Direction::North);
        auto &mover = tw.AddAgent<DrivingAgent>("Mover");
        mover.SetLocation(WorldPosition{2, 1});
        mover.SetDirection(Direction::East);
        CHECK(tw.DoAction(mover, 4 /*MOVE_RIGHT*/) == 0);
    }
    SECTION("head-on agents pass through each other") {
        TrafficWorld tw{kLineGrid};
        auto &east_agent = tw.AddAgent<DrivingAgent>("East");
        east_agent.SetLocation(WorldPosition{1, 1});
        east_agent.SetDirection(Direction::East);
        auto &west_agent = tw.AddAgent<DrivingAgent>("West");
        west_agent.SetLocation(WorldPosition{2, 1});
        west_agent.SetDirection(Direction::West);
        CHECK(tw.DoAction(east_agent, 4 /*MOVE_RIGHT*/) == 1);
        CHECK(east_agent.GetLocation().AsWorldPosition() == WorldPosition{2, 1});
    }
}

// UpdateWorld — traffic lights
TEST_CASE("UpdateWorld — traffic lights", "[TrafficWorld][update]") {
    // To start, ALLOW_VERTICAL -> MOVE_RIGHT into '|' is blocked
    // After traffic_light_period (3) ticks it flips to ALLOW_HORIZONTAL
    SECTION("blocked before flip (period - 1 = 2 ticks)") {
        TrafficWorld tw{kMinimalGrid};
        for (int i = 0; i < 2; ++i) tw.UpdateWorld();
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{3, 1});
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 0);
    }
    SECTION("allowed after one full period (3 ticks)") {
        TrafficWorld tw{kMinimalGrid};
        for (int i = 0; i < 3; ++i) tw.UpdateWorld();
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{3, 1});
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 1);
    }
    SECTION("back to original phase after two periods (6 ticks)") {
        TrafficWorld tw{kMinimalGrid};
        for (int i = 0; i < 6; ++i) tw.UpdateWorld();
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{3, 1});
        CHECK(tw.DoAction(agent, 4 /*MOVE_RIGHT*/) == 0);
    }
}

// UpdateWorld for spawners
TEST_CASE("UpdateWorld — spawners", "[TrafficWorld][update][spawners]") {
    SECTION("worlds without spawners/destinations survive many ticks") {
        { TrafficWorld tw{kNoSpawnerGrid};
          for (int i = 0; i < 60; ++i) tw.UpdateWorld(); }
        { TrafficWorld tw{kNoDestGrid};
          for (int i = 0; i < 60; ++i) tw.UpdateWorld(); }
    }
    SECTION("spawn fires on tick 20, not tick 19") {
        TrafficWorld tw{kMinimalGrid};
        WorldPosition spawner{1, 1};
        for (int i = 0; i < 19; ++i) tw.UpdateWorld();
        CHECK_FALSE(tw.AgentExistsAt(spawner));
        tw.UpdateWorld();
        CHECK(tw.AgentExistsAt(spawner));
    }
    SECTION("spawner blocked when an agent already occupies it") {
        TrafficWorld tw{kMinimalGrid};
        auto &blocker = tw.AddAgent<DrivingAgent>("Blocker");
        blocker.SetLocation(WorldPosition{1, 1});
        for (int i = 0; i < 20; ++i) tw.UpdateWorld();
        CHECK(tw.GetNumAgents() == 1);
    }
    SECTION("max_spawned_agents cap (15) is respected") {
        TrafficWorld tw{kMultiGrid};
        for (int i = 0; i < 400; ++i) tw.UpdateWorld();
        int active = 0;
        for (size_t id = 0; id < tw.GetNumAgents(); ++id) {
            auto *d = dynamic_cast<DrivingAgent *>(&tw.GetAgent(id));
            if (d && !d->GetReachedDestination()) ++active;
        }
        CHECK(active <= 15);
    }
}

// UpdateWorld for destinations/despawn
TEST_CASE("UpdateWorld — destinations", "[TrafficWorld][update][destinations]") {
    // Agents must be spawned via the spawner (not AddAgent) so that
    // num_spawned_agents is correctly tracked. Manually-added agents have
    // num_spawned_agents == 0, so HandleDestinations's assert would fire.

    SECTION("agent is marked arrived when it reaches its destination") {
        TrafficWorld tw{kMinimalGrid}; // 'S' col 1, 'D' col 7, row 1
        for (int i = 0; i < 20; ++i) tw.UpdateWorld(); // spawner fires
        REQUIRE(tw.AgentExistsAt(WorldPosition{1, 1}));
        auto *driver = dynamic_cast<DrivingAgent *>(&tw.GetAgent(0));
        REQUIRE(driver != nullptr);
        driver->SetLocation(WorldPosition{7, 1}); // teleport to destination
        tw.UpdateWorld();
        CHECK(driver->GetReachedDestination());
        CHECK(driver->GetSymbol() == 'D');
    }
    SECTION("DoAction alone does not trigger arrival; UpdateWorld does") {
        TrafficWorld tw{kMinimalGrid};
        for (int i = 0; i < 20; ++i) tw.UpdateWorld();
        REQUIRE(tw.AgentExistsAt(WorldPosition{1, 1}));
        auto *driver = dynamic_cast<DrivingAgent *>(&tw.GetAgent(0));
        REQUIRE(driver != nullptr);
        driver->SetLocation(WorldPosition{7, 1});
        tw.DoAction(*driver, 0 /*REMAIN_STILL*/);
        CHECK_FALSE(driver->GetReachedDestination()); // not yet
        tw.UpdateWorld();
        CHECK(driver->GetReachedDestination());
    }
    SECTION("despawned agent slot is recycled (agent_set size stays constant)") {
        TrafficWorld tw{kMinimalGrid};
        for (int i = 0; i < 20; ++i) tw.UpdateWorld();
        REQUIRE(tw.AgentExistsAt(WorldPosition{1, 1}));
        auto *driver = dynamic_cast<DrivingAgent *>(&tw.GetAgent(0));
        REQUIRE(driver != nullptr);
        driver->SetLocation(WorldPosition{7, 1});
        tw.UpdateWorld(); // agent despawns
        REQUIRE(driver->GetReachedDestination());
        size_t slots = tw.GetNumAgents();
        for (int i = 0; i < 20; ++i) tw.UpdateWorld(); // one more spawn cycle
        CHECK(tw.GetNumAgents() == slots); // recycled, not grown
    }
}

// RunAgents and other edge cases
TEST_CASE("RunAgents and edge cases", "[TrafficWorld][misc]") {
    SECTION("RunAgents skips agents that have reached their destination") {
        TrafficWorld tw{kMinimalGrid};
        auto &agent = tw.AddAgent<DrivingAgent>("Car");
        agent.SetLocation(WorldPosition{2, 1});
        agent.SetReachedDestination(true);
        WorldPosition before = agent.GetLocation().AsWorldPosition();
        tw.RunAgents();
        CHECK(agent.GetLocation().AsWorldPosition() == before);
    }
    SECTION("GetNumAgents starts at zero and increments per AddAgent") {
        TrafficWorld tw{kLineGrid};
        CHECK(tw.GetNumAgents() == 0);
        tw.AddAgent<DrivingAgent>("A"); CHECK(tw.GetNumAgents() == 1);
        tw.AddAgent<DrivingAgent>("B"); CHECK(tw.GetNumAgents() == 2);
    }
    SECTION("GetAgent returns the correct agent by ID") {
        TrafficWorld tw{kMinimalGrid};
        auto &orig = tw.AddAgent<DrivingAgent>("TestCar");
        orig.SetLocation(WorldPosition{2, 1});
        AgentBase &got = tw.GetAgent(orig.GetID());
        CHECK(got.GetID() == orig.GetID());
        CHECK(got.GetLocation().AsWorldPosition() == orig.GetLocation().AsWorldPosition());
    }
    SECTION("grid dimensions are preserved after construction") {
        TrafficWorld tw{kMinimalGrid}; // 9 wide, 3 tall
        CHECK(tw.GetGrid().GetWidth()  == 9);
        CHECK(tw.GetGrid().GetHeight() == 3);
    }
}
