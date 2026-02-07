# Tests
#######

# executable
add_executable(test_runner_player EXCLUDE_FROM_ALL
	doctest.h
	algo.cpp
	attribute.cpp
	autobattle.cpp
	bitmapfont.cpp
	cmdline_parser.cpp
	config_param.cpp
	drawable_list.cpp
	drawable_mgr.cpp
	dynrpg.cpp
	enemyai.cpp
	filefinder.cpp
	filesystem.cpp
	filesystem_tar.cpp
	filesystem_zip.cpp
	flat_map.cpp
	font.cpp
	game_actor.cpp
	game_battlealgorithm.cpp
	game_character.cpp
	game_character_anim.cpp
	game_character_flash.cpp
	game_character_move.cpp
	game_character_moveto.cpp
	game_destiny.cpp
	game_enemy.cpp
	game_event.cpp
	game_player_input.cpp
	game_player_pan.cpp
	game_player_savecount.cpp
	json.cpp
	mock_game.cpp
	mock_game.h
	move_route.cpp
	output.cpp
	parse.cpp
	platform.cpp
	rand.cpp
	rtp.cpp
	switches.cpp
	test_main.cpp
	test_mock_actor.h
	test_move_route.h
	text.cpp
	utf.cpp
	utils.cpp
	variables.cpp
	wordwrap.cpp)

set_target_properties(test_runner_player PROPERTIES
	OUTPUT_NAME "test_runner"
	WIN32_EXECUTABLE FALSE)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	target_compile_definitions(test_runner_player
		PUBLIC EP_NATIVE_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\")
	target_compile_definitions(test_runner_player
		PUBLIC EP_TEST_PATH=\"/assets\")
	target_compile_definitions(test_runner_player
		PUBLIC DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS=1)

	target_link_libraries(test_runner_player "nodefs.js")
	set_property(TARGET test_runner_player PROPERTY LINK_FLAGS "-sALLOW_MEMORY_GROWTH --bind")
else()
	target_compile_definitions(test_runner_player
		PUBLIC EP_TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\")
endif()

target_link_libraries(test_runner_player ${PLAYER_TEST_LIBRARIES})

# target "test"
if(CMAKE_COLOR_DIAGNOSTICS)
	set(DOCTEST_COLOROPT "--force-colors")
endif()
add_custom_target(check_player
	COMMAND test_runner_player ${DOCTEST_COLOROPT}
	COMMENT "Running doctest" VERBATIM)
add_dependencies(check_player test_runner_player)

if(NOT TARGET check)
	add_custom_target(check)
endif()
add_dependencies(check check_player)
