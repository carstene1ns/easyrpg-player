# PS4
#####

# preparations
set(RES_SRCDIR ${PROJECT_SOURCE_DIR}/resources/ps4)
set(RES_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/resources/ps4)

configure_file(${OPENORBIS}/samples/piglet/sce_module/libc.prx
	${RES_BINDIR}/sce_module/libc.prx COPYONLY)
configure_file(${OPENORBIS}/samples/piglet/sce_module/libSceFios2.prx
	${RES_BINDIR}/sce_module/libSceFios2.prx COPYONLY)
configure_file(${OPENORBIS}/samples/piglet/sce_sys/about/right.sprx
	${RES_BINDIR}/sce_sys/about/right.sprx COPYONLY)
configure_file(${RES_SRCDIR}/sce_sys/icon0.png
	${RES_BINDIR}/sce_sys/icon0.png COPYONLY)

# library
target_sources(${PROJECT_NAME} PRIVATE clock.h)

# executable
add_executable(easyrpg-player main.cpp)
target_link_libraries(easyrpg-player ${PROJECT_NAME}
	bz2 SceSysmodule ScePigletv2VSH SceUserService SceAudioOut SceSystemService ScePad)

add_self(easyrpg-player)
add_pkg(easyrpg-player
	${RES_BINDIR}
	"ERPG00001"
	"EasyRPG"
	"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
