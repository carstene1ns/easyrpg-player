# Wii
#####

# preparations
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/wii)
set(RES_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/resources/wii)

string(TIMESTAMP WII_DATE "%Y%m%d000000")
configure_file(${RES_SRCDIR}/meta.xml.in ${RES_BINDIR}/meta.xml @ONLY)

# dependencies
if(PLAYER_TARGET_PLATFORM STREQUAL "SDL1")
	find_package(SDL REQUIRED)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL_INCLUDE_DIR})
endif()

# library
target_sources(${PROJECT_NAME} PRIVATE clock.h input_buttons.cpp)

if(PLAYER_AUDIO_BACKEND STREQUAL "AESND")
	target_sources(${PROJECT_NAME} PRIVATE audio.cpp audio.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC AUDIO_AESND)
endif()

# executable
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player
	${PROJECT_NAME} ${SDL_LIBRARIES} -laesnd -lfat -lwiikeyboard)
if(TARGET SDL2::SDL2main)
	# Missing dependency?
	target_link_libraries(SDL2::SDL2main INTERFACE -lfat)
endif()

ogc_create_dol(easyrpg-player)

# debug information
player_handle_dkp_debug_files(easyrpg-player)

# installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.dol
	RENAME boot.dol
	DESTINATION easyrpg-player COMPONENT wii)
install(FILES
		${RES_SRCDIR}/icon.png
		${RES_BINDIR}/meta.xml
	DESTINATION easyrpg-player COMPONENT wii)

# packaging
set(CPACK_PLATFORM "wii" PARENT_SCOPE)
