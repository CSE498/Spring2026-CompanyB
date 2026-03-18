// Author: Landon Cosby
//
// Brief Description: Unit tests for the TagManager class, which manages
// the registration of objects and their associated tags, as well as querying



// This test suite contains code written by ChatGPT
//
// Prompt: [Attached TagManager.hpp and TagManager.cpp, and TagManagerTest.cpp] description of how AnnotationSet works
// Write unit tests for the TagManager class
//
// Author: ChatGPT 5.2

// #include "catch2/catch.hpp"

#include <catch2/catch_test_macros.hpp>


#include <algorithm>
#include <vector>
#include <cstdint>


#include "../../source/tools/TagManager.hpp"
#include "../../source/tools/AnnotationSet.hpp"


// g++ -std=c++23 -Wall -I../../../third-party/Catch/single_include Tools/AnnotationSet.cpp Tools/TagManager.cpp Tools/TagManagerTest.cpp -o TagManagerTest.exe

using namespace cse498;

// Objects must have a tag before they can be registered
TEST_CASE("One Object", "[IsRegistered][RegisterObject]") {
    TagManager mgr;

    AnnotationSet s;
    mgr.RegisterObject(1, s);

    REQUIRE_FALSE(mgr.IsRegistered(1));
}

TEST_CASE("Multiple Objects", "[IsRegistered][RegisterObject]") {
    TagManager mgr;

    AnnotationSet s;
    AnnotationSet s2;

    mgr.RegisterObject(1, s);
    mgr.RegisterObject(2, s2);

    REQUIRE_FALSE(mgr.IsRegistered(1));
    REQUIRE_FALSE(mgr.IsRegistered(2));
}

TEST_CASE("One Object, with tags", "[QueryOneTag]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    mgr.RegisterObject(1, s);

    REQUIRE(mgr.IsRegistered(1));
    REQUIRE(mgr.QueryOneTag("Tag1").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag2").size() == 1);
}

TEST_CASE("Multiple Objects, with tags", "[QueryOneTag]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    AnnotationSet s2;
    s2.addTag("Tag3");
    s2.addTag("Tag4");

    mgr.RegisterObject(1, s);
    mgr.RegisterObject(2, s2);

    REQUIRE(mgr.IsRegistered(1));
    REQUIRE(mgr.IsRegistered(2));

    REQUIRE(mgr.QueryOneTag("Tag1").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag2").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag3").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag4").size() == 1);
}


TEST_CASE("Object registered then added to", "[AddTag]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    mgr.RegisterObject(1, s);

    REQUIRE(mgr.IsRegistered(1));
    REQUIRE(mgr.QueryOneTag("Tag1").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag2").size() == 1);

    REQUIRE(mgr.AddTag(1, s, "Tag3"));
    REQUIRE(mgr.QueryOneTag("Tag3").size() == 1);
}

TEST_CASE("Remove a tag from an object", "[RemoveTag]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    mgr.RegisterObject(1, s);

    REQUIRE(mgr.IsRegistered(1));
    REQUIRE(mgr.QueryOneTag("Tag1").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag2").size() == 1);

    REQUIRE(mgr.RemoveTag(1, s, "Tag1"));
    REQUIRE(mgr.QueryOneTag("Tag1").empty());
    REQUIRE_FALSE(mgr.QueryOneTag("Tag2").empty());
}

TEST_CASE("Clear all tags from an object", "[ClearTags]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    mgr.RegisterObject(1, s);

    REQUIRE(mgr.QueryOneTag("Tag1").size() == 1);
    REQUIRE(mgr.QueryOneTag("Tag2").size() == 1);

    mgr.ClearTags(1, s);

    REQUIRE(mgr.QueryOneTag("Tag1").empty());
    REQUIRE(mgr.QueryOneTag("Tag2").empty());
}



// Following helpers were written by AI as referenced above.
static std::vector<TagManager::ObjectId> SortIds(std::vector<TagManager::ObjectId> v) {
    std::sort(v.begin(), v.end());
    return v;
}


static void REQUIRE_SAME_IDS(std::vector<TagManager::ObjectId> actual, std::vector<TagManager::ObjectId> expected) {
    REQUIRE(SortIds(std::move(actual)) == SortIds(std::move(expected)));
}


static std::vector<tag> Tags(std::initializer_list<tag> xs) {
    return std::vector<tag>(xs.begin(), xs.end());
}


TEST_CASE("Query multiple tags for an object", "[QueryMultiTags]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    mgr.RegisterObject(1, s);

    REQUIRE(mgr.QueryMultiTags(Tags({"Tag1", "Tag2"})).size() == 1);
    REQUIRE(mgr.QueryMultiTags(Tags({"Tag1"})).size() == 1);

}


TEST_CASE("Query multiple tags with must not contain values", "[QueryMultiTags]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("Tag1");
    s.addTag("Tag2");

    AnnotationSet s2;
    s2.addTag("Tag3");

    mgr.RegisterObject(1, s);
    mgr.RegisterObject(2, s2);

    REQUIRE(mgr.QueryMultiTags(Tags({"Tag3"}), Tags({"Tag1", "Tag2"})).size() == 1);
    REQUIRE(mgr.QueryMultiTags(Tags({"Tag1", "Tag2"}), Tags({"Tag3"})).size() == 1);
    REQUIRE(mgr.QueryMultiTags(Tags({"Tag1"}), Tags({"Tag1", "Tag2", "Tag3"})).empty());

}


// Following tests were written by AI as referenced above. 
// They cover various scenarios for the TagManager class.
TEST_CASE("RegisterObject indexes tags for QueryOneTag", "[TagManager][RegisterObject][QueryOneTag]") {
    TagManager mgr;


    AnnotationSet a;
    a.addTag("Hungry");
    a.addTag("NearFood");


    mgr.RegisterObject(1, a);


    REQUIRE(mgr.IsRegistered(1));
    REQUIRE_SAME_IDS(mgr.QueryOneTag("Hungry"), {1});
    REQUIRE_SAME_IDS(mgr.QueryOneTag("NearFood"), {1});
    REQUIRE(mgr.QueryOneTag("DoesNotExist").empty());
}


TEST_CASE("RegisterObject overwrites old registration for same ID", "[TagManager][RegisterObject]") {
    TagManager mgr;


    AnnotationSet first;
    first.addTag("A");
    first.addTag("B");


    mgr.RegisterObject(42, first);


    REQUIRE_SAME_IDS(mgr.QueryOneTag("A"), {42});
    REQUIRE_SAME_IDS(mgr.QueryOneTag("B"), {42});


    AnnotationSet second;
    second.addTag("C");          // new set has only C
    mgr.RegisterObject(42, second);


    // A and B should no longer contain 42
    REQUIRE(mgr.QueryOneTag("A").empty());
    REQUIRE(mgr.QueryOneTag("B").empty());
    REQUIRE_SAME_IDS(mgr.QueryOneTag("C"), {42});
}


TEST_CASE("UnregisterObject removes object from all tag buckets", "[TagManager][UnregisterObject]") {
    TagManager mgr;


    AnnotationSet s;
    s.addTag("A");
    s.addTag("B");
    mgr.RegisterObject(9, s);


    REQUIRE(mgr.UnregisterObject(9));
    REQUIRE_FALSE(mgr.IsRegistered(9));


    REQUIRE(mgr.QueryOneTag("A").empty());
    REQUIRE(mgr.QueryOneTag("B").empty());


    // Unregistering again should return false
    REQUIRE_FALSE(mgr.UnregisterObject(9));
}


TEST_CASE("AddTag auto-registers missing objects and updates set + index", "[TagManager][AddTag]") {
    TagManager mgr;


    AnnotationSet s;
    // not registered yet


    REQUIRE_FALSE(mgr.IsRegistered(7));


    REQUIRE(mgr.AddTag(7, s, "X"));
    REQUIRE(mgr.IsRegistered(7));
    REQUIRE(s.hasTag("X"));
    REQUIRE_SAME_IDS(mgr.QueryOneTag("X"), {7});


    // Adding same tag again should return false (no change)
    REQUIRE_FALSE(mgr.AddTag(7, s, "X"));
    REQUIRE_SAME_IDS(mgr.QueryOneTag("X"), {7});
}


TEST_CASE("RemoveTag updates set + index and returns false when tag missing", "[TagManager][RemoveTag]") {
    TagManager mgr;


    AnnotationSet s;
    s.addTag("A");
    s.addTag("B");
    mgr.RegisterObject(11, s);


    REQUIRE_SAME_IDS(mgr.QueryOneTag("A"), {11});
    REQUIRE_SAME_IDS(mgr.QueryOneTag("B"), {11});


    REQUIRE(mgr.RemoveTag(11, s, "A"));
    REQUIRE_FALSE(s.hasTag("A"));
    REQUIRE(mgr.QueryOneTag("A").empty());
    REQUIRE_SAME_IDS(mgr.QueryOneTag("B"), {11});


    // Removing again should be false
    REQUIRE_FALSE(mgr.RemoveTag(11, s, "A"));
}


TEST_CASE("ClearTags always clears the AnnotationSet and removes index entries if registered", "[TagManager][ClearTags]") {
    TagManager mgr;


    AnnotationSet s;
    s.addTag("A");
    s.addTag("B");
    mgr.RegisterObject(100, s);


    REQUIRE_SAME_IDS(mgr.QueryOneTag("A"), {100});
    REQUIRE_SAME_IDS(mgr.QueryOneTag("B"), {100});


    mgr.ClearTags(100, s);


    REQUIRE(s.empty());
    REQUIRE_FALSE(mgr.IsRegistered(100));
    REQUIRE(mgr.QueryOneTag("A").empty());
    REQUIRE(mgr.QueryOneTag("B").empty());
}


TEST_CASE("QueryMultiTags AND behavior with required tags", "[TagManager][QueryMultiTags][AND]") {
    TagManager mgr;


    AnnotationSet a; a.addTag("A"); a.addTag("B");
    AnnotationSet b; b.addTag("A");
    AnnotationSet c; c.addTag("A"); c.addTag("B"); c.addTag("C");


    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);
    mgr.RegisterObject(3, c);


    // A AND B -> {1,3}
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A","B"})), {1,3});


    // A AND C -> {3}
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A","C"})), {3});


    // B AND C -> {3}
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"B","C"})), {3});


    // A AND Z (missing) -> {}
    REQUIRE(mgr.QueryMultiTags(Tags({"A","Z"})).empty());
}


TEST_CASE("QueryMultiTags supports NOT list (must_not_have) when must_have is non-empty", "[TagManager][QueryMultiTags][NOT]") {
    TagManager mgr;


    AnnotationSet a; a.addTag("A"); a.addTag("B");
    AnnotationSet b; b.addTag("A"); b.addTag("B"); b.addTag("C");
    AnnotationSet c; c.addTag("A"); c.addTag("B"); c.addTag("D");


    mgr.RegisterObject(10, a);
    mgr.RegisterObject(20, b);
    mgr.RegisterObject(30, c);


    // (A AND B) AND NOT C -> {10,30}
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A","B"}), Tags({"C"})), {10,30});


    // (A AND B) AND NOT C AND NOT D -> {10}
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A","B"}), Tags({"C","D"})), {10});
}


TEST_CASE("QueryMultiTags returns empty when both must_have and must_not_have are empty", "[TagManager][QueryMultiTags][Edge]") {
    TagManager mgr;


    AnnotationSet a; a.addTag("A");
    mgr.RegisterObject(1, a);


    REQUIRE(mgr.QueryMultiTags({}, {}).empty());
}


// This test matches your *current* implementation:
// If must_have is empty and must_not_have is non-empty, result starts empty and stays empty.
TEST_CASE("QueryMultiTags with empty must_have returns empty (current behavior)", "[TagManager][QueryMultiTags][Edge]") {
    TagManager mgr;


    AnnotationSet a; a.addTag("A"); a.addTag("Dead");
    AnnotationSet b; b.addTag("A");


    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);


    REQUIRE(mgr.QueryMultiTags({}, Tags({"Dead"})).empty());
}

TEST_CASE("QueryOneTag returns multiple IDs for same tag", "[TagManager][QueryOneTag]") {
    TagManager mgr;

    AnnotationSet a; a.addTag("Same");
    AnnotationSet b; b.addTag("Same");
    AnnotationSet c; c.addTag("Other");

    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);
    mgr.RegisterObject(3, c);

    REQUIRE_SAME_IDS(mgr.QueryOneTag("Same"), {1,2});
    REQUIRE_SAME_IDS(mgr.QueryOneTag("Other"), {3});
}

TEST_CASE("AddTag on already-registered object updates QueryMultiTags AND results", "[TagManager][AddTag][QueryMultiTags]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("A");
    mgr.RegisterObject(50, s);

    // Initially, A AND B should be empty
    REQUIRE(mgr.QueryMultiTags(Tags({"A","B"})).empty());

    // Add B and now A AND B should contain 50
    REQUIRE(mgr.AddTag(50, s, "B"));
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A","B"})), {50});
}

TEST_CASE("RemoveTag removes only that tag; other tags still queryable", "[TagManager][RemoveTag][QueryOneTag]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("A");
    s.addTag("B");
    s.addTag("C");
    mgr.RegisterObject(60, s);

    REQUIRE(mgr.RemoveTag(60, s, "B"));

    REQUIRE_SAME_IDS(mgr.QueryOneTag("A"), {60});
    REQUIRE(mgr.QueryOneTag("B").empty());
    REQUIRE_SAME_IDS(mgr.QueryOneTag("C"), {60});
}

TEST_CASE("ClearTags on a non-registered object does not crash and still clears the set", "[TagManager][ClearTags][Edge]") {
    TagManager mgr;

    AnnotationSet s;
    s.addTag("A");
    s.addTag("B");

    REQUIRE_FALSE(mgr.IsRegistered(123));

    // Should simply clear the provided set (per your comment/implementation)
    mgr.ClearTags(123, s);

    REQUIRE(s.empty());
    REQUIRE_FALSE(mgr.IsRegistered(123));
    // No index entries were ever created, so queries should be empty
    REQUIRE(mgr.QueryOneTag("A").empty());
    REQUIRE(mgr.QueryOneTag("B").empty());
}

TEST_CASE("UnregisterObject on one of multiple objects sharing a tag does not remove the tag bucket for others", "[TagManager][UnregisterObject]") {
    TagManager mgr;

    AnnotationSet a; a.addTag("Shared");
    AnnotationSet b; b.addTag("Shared");

    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);

    REQUIRE_SAME_IDS(mgr.QueryOneTag("Shared"), {1,2});

    REQUIRE(mgr.UnregisterObject(1));

    // Tag bucket should still exist for object 2
    REQUIRE_SAME_IDS(mgr.QueryOneTag("Shared"), {2});
}

TEST_CASE("QueryMultiTags returns empty if NOT filter removes all remaining candidates", "[TagManager][QueryMultiTags][NOT]") {
    TagManager mgr;

    AnnotationSet a; a.addTag("A"); a.addTag("Dead");
    AnnotationSet b; b.addTag("A"); b.addTag("Dead");

    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);

    // Start with all having A, then remove those with Dead => empty
    REQUIRE(mgr.QueryMultiTags(Tags({"A"}), Tags({"Dead"})).empty());
}

TEST_CASE("QueryMultiTags ignores NOT tags that do not exist in index", "[TagManager][QueryMultiTags][NOT]") {
    TagManager mgr;

    AnnotationSet a; a.addTag("A");
    AnnotationSet b; b.addTag("A"); b.addTag("B");

    mgr.RegisterObject(1, a);
    mgr.RegisterObject(2, b);

    // NOT tag doesn't exist in index, so it should not change the result
    REQUIRE_SAME_IDS(mgr.QueryMultiTags(Tags({"A"}), Tags({"NoSuchTag"})), {1,2});
}