# 3DS
#####

# preparations
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/3ds)

# library
target_sources(${PROJECT_NAME} PRIVATE
	audio.cpp audio.h clock.h input_buttons.cpp ui.cpp ui.h)
target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=CtrUi)

# Remove abi warning after devkitarm ships newer gcc
target_compile_options(${PROJECT_NAME} PUBLIC -Wno-psabi)

# gfx assets
enable_language(ASM)
ctr_add_graphics_target(keyboard IMAGE
	INPUTS ${RES_SRCDIR}/keyboard.png
	OPTIONS -f rgba)
ctr_add_graphics_target(battery ATLAS
	INPUTS
		${RES_SRCDIR}/battery/empty.png
		${RES_SRCDIR}/battery/1bar.png
		${RES_SRCDIR}/battery/2bars.png
		${RES_SRCDIR}/battery/3bars.png
		${RES_SRCDIR}/battery/4bars.png
		${RES_SRCDIR}/battery/full.png
		${RES_SRCDIR}/battery/adapter.png
	OPTIONS -f rgba)
dkp_add_embedded_binary_library(3ds-assets keyboard battery)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:3ds-assets>)
target_link_libraries(${PROJECT_NAME} 3ds-assets)

# executable
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player ${PROJECT_NAME} -lcitro2d -lcitro3d)

ctr_generate_smdh(easyrpg-player.smdh
	NAME "${PLAYER_NAME} ${PLAYER_VERSION_FULL}"
	DESCRIPTION "Play your RPG Maker 2000/2003 games everywhere"
	AUTHOR "${PLAYER_AUTHORS}"
	ICON ${RES_SRCDIR}/icon.png)
ctr_create_3dsx(easyrpg-player SMDH easyrpg-player.smdh
	${BUNDLE_ARG} ${PLAYER_BUNDLE_PATH})

# installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.3dsx
	DESTINATION easyrpg-player COMPONENT 3ds)
