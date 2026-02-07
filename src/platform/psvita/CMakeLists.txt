# PSVita
########

# preperations
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/psvita)
set(RES_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/resources/psvita)

configure_file(${RES_SRCDIR}/template.xml.in ${RES_BINDIR}/template.xml @ONLY)

# library
target_sources(${PROJECT_NAME} PRIVATE
	audio.cpp audio.h clock.h input_buttons.cpp ui.cpp ui.h)
target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=Psp2Ui)

# Remove abi warning after vitasdk ships newer gcc
target_compile_options(${PROJECT_NAME} PUBLIC -Wno-psabi)

# This optimisation is unstable on Vita (crashes at the end of MainLoop)
target_compile_options(${PROJECT_NAME} PUBLIC -fno-optimize-sibling-calls)

# executable
set(CMAKE_EXECUTABLE_SUFFIX .elf)
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player ${PROJECT_NAME}
	vita2d -Wl,--whole-archive pthread -Wl,--no-whole-archive SceLibKernel_stub
	SceDisplay_stub SceGxm_stub SceSysmodule_stub SceCtrl_stub SceTouch_stub
	ScePgf_stub ScePower_stub SceCommonDialog_stub SceAudio_stub SceAppMgr_stub)

vita_create_self(eboot.bin easyrpg-player.elf $<$<NOT:$<CONFIG:Debug>>:STRIPPED>)
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12") # ? TODO
vita_create_vpk(EasyRPG-Player.vpk "EASYRPG01" eboot.bin
	VERSION "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK}"
	NAME "${PLAYER_NAME}"
	FILE "${RES_SRCDIR}/icon0.png" sce_sys/icon0.png
	FILE "${RES_SRCDIR}/bg.png" sce_sys/livearea/contents/bg.png
	FILE "${RES_SRCDIR}/startup.png" sce_sys/livearea/contents/startup.png
	FILE "${RES_BINDIR}/template.xml" sce_sys/livearea/contents/template.xml
	FILE "${RES_SRCDIR}/touch_ui.png" touch_ui.png)

# installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/EasyRPG-Player.vpk
	DESTINATION . COMPONENT psvita)
