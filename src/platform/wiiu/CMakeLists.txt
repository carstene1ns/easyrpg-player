# Wii U
#######

# preparations
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/wiiu)
set(RES_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/resources/wiiu)

# library
target_sources(${PROJECT_NAME} PRIVATE main.h input_buttons.cpp)

# executable
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player ${PROJECT_NAME})

wut_create_rpx(easyrpg-player)
wut_create_wuhb(easyrpg-player
	NAME "${PLAYER_NAME} ${PLAYER_VERSION_FULL}"
	SHORTNAME "${PLAYER_NAME}"
	AUTHOR "${PLAYER_AUTHORS}"
	ICON ${RES_SRCDIR}/icon.png
	TVSPLASH ${RES_SRCDIR}/splash-tv.png
	DRCSPLASH ${RES_SRCDIR}/splash-drc.png
	${BUNDLE_ARG} ${PLAYER_BUNDLE_PATH})

# debug information
player_handle_dkp_debug_files(easyrpg-player)

# installation
install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.rpx
		${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.wuhb
	DESTINATION . COMPONENT wiiu)

# packaging
set(CPACK_PLATFORM "wiiu" PARENT_SCOPE)
