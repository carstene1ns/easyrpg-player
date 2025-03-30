#include "engine/hero.h"
#include "doctest.h"
#include "options.h"
#include "engine/map.h"
#include "main_data.h"
#include <climits>

TEST_SUITE_BEGIN("Game_Hero_SaveCount");

TEST_CASE("SaveCounts") {
	Game_Hero ch;

	REQUIRE(!ch.IsMapCompatibleWithSave(-1));
	REQUIRE(ch.IsMapCompatibleWithSave(0));
	REQUIRE(!ch.IsMapCompatibleWithSave(1));

	REQUIRE(!ch.IsDatabaseCompatibleWithSave(-1));
	REQUIRE(ch.IsDatabaseCompatibleWithSave(0));
	REQUIRE(!ch.IsDatabaseCompatibleWithSave(1));

	ch.UpdateSaveCounts(55, 77);

	REQUIRE(!ch.IsMapCompatibleWithSave(76));
	REQUIRE(ch.IsMapCompatibleWithSave(77));
	REQUIRE(!ch.IsMapCompatibleWithSave(78));

	REQUIRE(!ch.IsDatabaseCompatibleWithSave(54));
	REQUIRE(ch.IsDatabaseCompatibleWithSave(55));
	REQUIRE(!ch.IsDatabaseCompatibleWithSave(56));
}

TEST_SUITE_END();
