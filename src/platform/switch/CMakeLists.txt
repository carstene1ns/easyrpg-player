# Switch
########

# preparations
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/switch)

# dependencies
find_package(OpenGL CONFIG REQUIRED)
find_library(GLAD glad REQUIRED)

# library
target_sources(${PROJECT_NAME} PRIVATE
	audio.cpp audio.h clock.h input_buttons.cpp ui.cpp ui.h)
target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=NxUi)
target_link_libraries(${PROJECT_NAME} OpenGL::EGL)

# gfx assets
enable_language(ASM)
dkp_add_embedded_binary_library(switch-assets ${RES_SRCDIR}/touch_ui.png)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_sources(${PROJECT_NAME} INTERFACE $<TARGET_OBJECTS:switch-assets>)
target_link_libraries(${PROJECT_NAME} switch-assets)

# executable
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player ${PROJECT_NAME} ${GLAD})

nx_generate_nacp(easyrpg-player.nacp
	NAME "${PLAYER_NAME}"
	AUTHOR "${PLAYER_AUTHORS}"
	VERSION "${PLAYER_VERSION_FULL}")
nx_create_nro(easyrpg-player NACP easyrpg-player.nacp
	ICON ${RES_SRCDIR}/icon.jpg
	${BUNDLE_ARG} ${PLAYER_BUNDLE_PATH})

# installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.nro
	DESTINATION easyrpg-player COMPONENT switch)
