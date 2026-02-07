cmake_minimum_required(VERSION 3.18...4.2 FATAL_ERROR)

project(EasyRPG_Player VERSION 0.8.1
	DESCRIPTION "Interpreter for RPG Maker 2000/2003 games"
	HOMEPAGE_URL "https://easyrpg.org"
	LANGUAGES CXX)
set(PLAYER_NAME "EasyRPG Player")
set(PLAYER_AUTHORS "EasyRPG Team")

# Extra CMake Module files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules")
include(CMakeDependentOption)
include(PlayerConfigureWindows)
include(PlayerFindPackage)
include(PlayerBuildType)
include(PlayerMisc)

# Dependencies provided by CMake Presets
option(PLAYER_FIND_ROOT_PATH_APPEND "Append CMAKE_PREFIX_PATH to CMAKE_FIND_ROOT_PATH. \
	Works around problematic CMake Toolchain files" OFF)

list(APPEND CMAKE_PREFIX_PATH "${PLAYER_PREFIX_PATH_APPEND}")
if(PLAYER_FIND_ROOT_PATH_APPEND)
	list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_PREFIX_PATH}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Must be at global scope, otherwise breaks -DPLAYER_BUILD_LIBLCF (see CMP0077)
option(BUILD_SHARED_LIBS "Build shared easyrpg_libretro core" ON)

option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Prefer config files over bundled FindXXX files. \
	Set this to OFF when configuration fails and report a bug." ON)

option(PLAYER_NONFREE "For commercial usage. \
	Disables libraries and fonts that are licensed under (L)GPL." OFF)

# Source Files
include(PlayerMainSources.cmake)
add_library(${PROJECT_NAME} OBJECT ${PLAYER_MAIN_SOURCES})

# These are actually unused when building in CMake, and should be removed someday:
# src/platform/opendingux/opendingux_input_buttons.cpp, src/platform/psp/psp_input_buttons.cpp

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optimize floating point math functions into intrinsics and don't check or set errno (i.e. sqrt(-1))
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(${PROJECT_NAME} PUBLIC "-fno-math-errno")
endif()

# Version setup
set(PLAYER_VERSION ${PROJECT_VERSION})
if(NOT PROJECT_VERSION_TWEAK)
	# a kludge to match the valid version format
	set(PROJECT_VERSION_TWEAK 0)
	if(NOT PROJECT_VERSION_PATCH)
		set(PROJECT_VERSION_PATCH 0)
	endif()
endif()
set(PLAYER_VERSION_FULL ${PLAYER_VERSION})
string(CONCAT PROJECT_VERSION
	${PROJECT_VERSION_MAJOR} "." ${PROJECT_VERSION_MINOR} "."
	${PROJECT_VERSION_PATCH} "." ${PROJECT_VERSION_TWEAK})
set(PLAYER_VERSION_DEFS EP_VERSION="${PLAYER_VERSION}"
	EP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} EP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
	EP_VERSION_PATCH=${PROJECT_VERSION_PATCH} EP_VERSION_TWEAK=${PROJECT_VERSION_TWEAK})

player_find_gitversion(VERSION_VAR PLAYER_VERSION_GIT MESSAGE_VAR GIT_STATUS)
if(PLAYER_VERSION_GIT)
	string(APPEND PLAYER_VERSION_FULL "-${PLAYER_VERSION_GIT}")
	list(APPEND PLAYER_VERSION_DEFS EP_VERSION_GIT="${PLAYER_VERSION_GIT}")
endif()

string(TIMESTAMP PLAYER_DATE "(%Y-%m-%d)")
set(PLAYER_VERSION_APPEND ${PLAYER_DATE} CACHE STRING "Additional version information to include")
if(NOT PLAYER_VERSION_APPEND STREQUAL "")
	list(APPEND PLAYER_VERSION_DEFS EP_VERSION_APPEND="${PLAYER_VERSION_APPEND}")
endif()

set_property(SOURCE src/version.cpp PROPERTY COMPILE_DEFINITIONS ${PLAYER_VERSION_DEFS})

# Make content available for romfs/wuhb bundle
if(NINTENDO_3DS OR NINTENDO_WIIU OR NINTENDO_SWITCH)
	option(PLAYER_BUNDLE "Embed a directory in the executable" OFF)
	set(PLAYER_BUNDLE_PATH "content" CACHE PATH "Directory to include in executable")
	set(BUNDLE_ARG "_IGNORE_ME")
	if(PLAYER_BUNDLE)
		if(NINTENDO_WIIU)
			set(BUNDLE_ARG "CONTENT")
		else()
			set(BUNDLE_ARG "ROMFS")
		endif()
	endif()
endif()

# Platform setup
set(HELP_STRING "Platform to compile for.")
if(NINTENDO_3DS)
	set(PLAYER_TARGET_PLATFORM "3ds" CACHE STRING "${HELP_STRING}")
	add_subdirectory(src/platform/3ds)
elseif(NINTENDO_SWITCH)
	set(PLAYER_TARGET_PLATFORM "switch" CACHE STRING "${HELP_STRING}")
	add_subdirectory(src/platform/switch)
elseif(VITA)
	set(PLAYER_TARGET_PLATFORM "psvita" CACHE STRING "${HELP_STRING}")
	add_subdirectory(src/platform/psvita)
elseif(NINTENDO_WII)
	set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "${HELP_STRING} Options: SDL2 SDL1")
	set_property(CACHE PLAYER_TARGET_PLATFORM PROPERTY STRINGS SDL2 SDL1)
	add_subdirectory(src/platform/wii)
elseif(NINTENDO_WIIU)
	set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "${HELP_STRING}")
	add_subdirectory(src/platform/wiiu)
elseif(AMIGA)
	set(PLAYER_TARGET_PLATFORM "SDL1" CACHE STRING "${HELP_STRING}")
elseif(PS4)
	set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "${HELP_STRING}")
	add_subdirectory(src/platform/ps4)
else()
	set(PLAYER_TARGET_PLATFORM "SDL2" CACHE STRING "${HELP_STRING} Options: SDL3 SDL2 SDL1 libretro")
	set_property(CACHE PLAYER_TARGET_PLATFORM PROPERTY STRINGS SDL3 SDL2 SDL1 libretro)
endif()
unset(HELP_STRING)
set(PLAYER_BUILD_EXECUTABLE ON)
set(PLAYER_TEST_LIBRARIES ${PROJECT_NAME})

if(ANDROID AND PLAYER_GRADLE_BUILD)
	# Build invoked by Gradle
	# Ugly: Gradle has no way to branch based on the ABI
	# Inject correct search path based on PLAYER_ANDROID_TOOLCHAIN_PATH
	foreach(f ${PLAYER_ANDROID_TOOLCHAIN_PATH})
		list(APPEND CMAKE_FIND_ROOT_PATH "${f}/${CMAKE_ANDROID_ARCH_ABI}-toolchain")
	endforeach(f)
endif()

if(PLAYER_TARGET_PLATFORM STREQUAL "SDL3")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/sdl3_ui.cpp
		src/platform/sdl/sdl3_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=3)

	player_find_package(NAME SDL3 TARGET SDL3::SDL3 REQUIRED)

	if(ANDROID)
		set(PLAYER_BUILD_EXECUTABLE OFF)
	endif()
elseif(PLAYER_TARGET_PLATFORM STREQUAL "SDL2")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/sdl2_ui.cpp
		src/platform/sdl/sdl2_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SDL=2)

	# SDL2 depends on some systems on SDL2::SDL2main but SDL2::SDL2 is not always a dependency of it
	# Manually add the dependencies
	player_find_package(NAME SDL2 VERSION 2.0.14 TARGET SDL2::SDL2 REQUIRED)

	if(TARGET SDL2::SDL2main)
		target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)
	endif()

	if(ANDROID)
		set(PLAYER_BUILD_EXECUTABLE OFF)
	endif()

	if(WIN32)
		target_link_libraries(${PROJECT_NAME} "Dwmapi")
	endif()
elseif(PLAYER_TARGET_PLATFORM STREQUAL "SDL1")
	if(NOT NINTENDO_WII)
		player_find_package(NAME SDL1 TARGET SDL::SDLmain REQUIRED)
	endif()

	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/axis.h
		src/platform/sdl/sdl_ui.cpp
		src/platform/sdl/sdl_ui.h)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		USE_SDL=1
		EASYRPG_CONFIG_NAME="config_sdl1.ini")
elseif(PLAYER_TARGET_PLATFORM STREQUAL "libretro")
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_UI=LibretroUi USE_LIBRETRO=1)
	set(PLAYER_BUILD_EXECUTABLE OFF)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/builds/libretro)
	target_link_libraries(${PROJECT_NAME} retro_common)
elseif(PLAYER_TARGET_PLATFORM MATCHES "^3ds|psvita|switch$")
	# handled in platform folder
else()
	message(FATAL_ERROR "Invalid target platform: ${PLAYER_TARGET_PLATFORM}")
endif()

# Sound system
set(HELP_STRING "Audio system to use.")
if(PLAYER_TARGET_PLATFORM STREQUAL "SDL3")
	set(PLAYER_AUDIO_BACKEND "SDL3" CACHE STRING "${HELP_STRING} Options: SDL3 OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL3 OFF)
elseif(PLAYER_TARGET_PLATFORM STREQUAL "SDL2")
	if(NINTENDO_WII)
		set(PLAYER_AUDIO_BACKEND "SDL2" CACHE STRING "${HELP_STRING} Options: SDL2 AESND OFF")
		set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL2 AESND OFF)
	else()
		set(PLAYER_AUDIO_BACKEND "SDL2" CACHE STRING "${HELP_STRING} Options: SDL2 OFF")
		set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL2 OFF)
	endif()
elseif(PLAYER_TARGET_PLATFORM STREQUAL "SDL1")
	if(NINTENDO_WII)
		set(PLAYER_AUDIO_BACKEND "AESND" CACHE STRING "${HELP_STRING} Options: SDL1 AESND OFF")
		set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL1 AESND OFF)
	else()
		set(PLAYER_AUDIO_BACKEND "SDL1" CACHE STRING "${HELP_STRING} Options: SDL1 OFF")
		set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS SDL1 OFF)
	endif()
else()
	# Assuming that all platforms not targeting SDL have only one audio backend
	set(PLAYER_AUDIO_BACKEND ${PLAYER_TARGET_PLATFORM} CACHE STRING
		"${HELP_STRING} Options: ${PLAYER_TARGET_PLATFORM} OFF")
	set_property(CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS ${PLAYER_TARGET_PLATFORM} OFF)
endif()
unset(HELP_STRING)

set(PLAYER_HAS_AUDIO ON)
get_property(AUDIO_PROP CACHE PLAYER_AUDIO_BACKEND PROPERTY STRINGS)
if(PLAYER_AUDIO_BACKEND STREQUAL "SDL3")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/sdl3_audio.cpp
		src/platform/sdl/sdl3_audio.h)
elseif(PLAYER_AUDIO_BACKEND STREQUAL "SDL2")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/sdl2_audio.cpp
		src/platform/sdl/sdl2_audio.h)
elseif(PLAYER_AUDIO_BACKEND STREQUAL "SDL1")
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/sdl/sdl_audio.cpp
		src/platform/sdl/sdl_audio.h)
elseif(PLAYER_AUDIO_BACKEND STREQUAL "AESND")
	# wii
elseif(PLAYER_AUDIO_BACKEND STREQUAL ${PLAYER_TARGET_PLATFORM})
	# intentionally left blank
elseif(PLAYER_AUDIO_BACKEND STREQUAL "OFF")
	set(PLAYER_HAS_AUDIO OFF)
else()
	message(FATAL_ERROR "Invalid Audio Backend: ${PLAYER_AUDIO_BACKEND}.\n"
		" Options: ${AUDIO_PROP}")
endif()

if(NOT PLAYER_AUDIO_BACKEND IN_LIST AUDIO_PROP)
	message(FATAL_ERROR "Incompatible audio backend ${PLAYER_AUDIO_BACKEND}.\n"
		"Please clear the CMake cache.\n Options: ${AUDIO_PROP}")
endif()
unset(AUDIO_PROP)

# Shared by homebrew platforms
if(PLAYER_CONSOLE_PORT)
	target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions -fno-rtti)
	set(CMAKE_DL_LIBS "") # hack4icu!
	set(PLAYER_ENABLE_TESTS OFF)
	option(PLAYER_VERSIONED_PACKAGES "Create zip packages with versioned name (for internal use)" ON)
endif()
# Enable some common code paths/workarounds
target_compile_definitions(${PROJECT_NAME} PUBLIC
	$<$<BOOL:${PLAYER_DEVKITPRO}>:PLAYER_NINTENDO>)

# Amiga (TODO: move into /src/platform)
if(AMIGA)
	target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions -fno-rtti)
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_AMIGA)
	if(MORPHOS)
		target_compile_definitions(${PROJECT_NAME} PUBLIC
			_GLIBCXX_USE_C99 _NO_PPCINLINE __MORPHOS_SHAREDLIBS)
		# -noixemul handled as "M68K_CRT" in toolchain file
		target_sources(${PROJECT_NAME} PRIVATE
			src/platform/morphos/integration.cpp)
		set_property(SOURCE src/platform/morphos/integration.cpp
			PROPERTY COMPILE_DEFINITIONS PLAYER_VERSION="${PLAYER_VERSION_FULL}")
	endif()
	if(AMIGAOS4)
		# FIXME: needed?
		target_compile_definitions(${PROJECT_NAME} PUBLIC
			_GLIBCXX_USE_C99_STDINT_TR1 _GLIBCXX_USE_C99_STDIO)
		target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/src/platform/amigaos4)
	endif()
	if(AROS)
		message(WARNING "AROS is unsupported currently. Patches welcome!")
	endif()
endif()

if(NOT PLAYER_BUILD_EXECUTABLE AND BUILD_SHARED_LIBS)
	# Need fPIC when compiling a shared library (e.g. libretro.so)
	include(CheckPIESupported)
	check_pie_supported()
	set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
	set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
endif()

if(ANDROID)
	target_link_libraries(${PROJECT_NAME} android log)
endif()

if(WIN32)
	target_sources(${PROJECT_NAME} PRIVATE
		src/registry.cpp
		src/platform/windows/utils.cpp
		src/platform/windows/utils.h
		src/platform/windows/midiout_device_win32.cpp
		src/platform/windows/midiout_device_win32.h)
endif()

if(APPLE)
	enable_language(OBJCXX)
	target_sources(${PROJECT_NAME} PRIVATE
		src/platform/macos/macos_utils.mm
		src/platform/macos/macos_utils.h)

	if(PLAYER_MACOS)
		find_library(MACOSAUDIOUNIT AudioUnit REQUIRED)
		target_sources(${PROJECT_NAME} PRIVATE
			src/platform/macos/midiout_device_coreaudio.cpp
			src/platform/macos/midiout_device_coreaudio.h)
	endif()

	find_library(MACOSFOUNDATION Foundation REQUIRED)
	find_library(MACOSAUDIOTOOLBOX AudioToolbox REQUIRED)
	target_link_libraries(${PROJECT_NAME} ${MACOSFOUNDATION} ${MACOSAUDIOUNIT} ${MACOSAUDIOTOOLBOX})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	option(PLAYER_JS_BUILD_SHELL "Build the Player executable as a shell file (.html) \
		instead of a standalone javascript file (.js)" OFF)
	set(PLAYER_JS_GAME_URL "games/" CACHE STRING "Game URL/directory where the web player searches for games")
	set(PLAYER_JS_OUTPUT_NAME "easyrpg-player" CACHE STRING "Output name of the js, html and wasm files")
	set_property(SOURCE src/async_handler.cpp APPEND PROPERTY
		COMPILE_DEFINITIONS "EM_GAME_URL=\"${PLAYER_JS_GAME_URL}\"")
endif()

# Endianess
target_compile_definitions(${PROJECT_NAME} PRIVATE
	$<$<BOOL:${PLAYER_BIGENDIAN}>:WORDS_BIGENDIAN>)

# App Icon (for macOS)
set(${PROJECT_NAME}_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Player.icns")

# Do not code sign on Macs
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.easyrpg.player")

# liblcf
option(PLAYER_BUILD_LIBLCF "For developers: Clone liblcf and builds it along with the Player." OFF)
set(PLAYER_BUILD_LIBLCF_GIT "https://github.com/EasyRPG/liblcf.git"
	CACHE STRING "Git repository of liblcf to clone. Requires PLAYER_BUILD_LIBLCF=ON.")
set(PLAYER_BUILD_LIBLCF_BRANCH "master"
	CACHE STRING "Branch of the liblcf repository to clone. Requires PLAYER_BUILD_LIBLCF=ON.")

if(PLAYER_BUILD_LIBLCF)
	# liblcf is built as part of this cmake file
	set(LIBLCF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf")
	if(NOT EXISTS ${LIBLCF_PATH})
		find_package(Git REQUIRED)
		execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1" "--branch"
			"${PLAYER_BUILD_LIBLCF_BRANCH}"
			"${PLAYER_BUILD_LIBLCF_GIT}"
			"${LIBLCF_PATH}")
	endif()
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/liblcf/builds/cmake/Modules")

	# Always build static liblcf and do not install it
	function(add_liblcf)
		set(BUILD_SHARED_LIBS OFF)
		set(LIBLCF_ENABLE_INSTALL OFF)
		add_subdirectory(${LIBLCF_PATH})
	endfunction()
	add_liblcf()
	target_link_libraries(${PROJECT_NAME} lcf)
else()
	# Use system package
	player_find_package(NAME liblcf VERSION 0.8.1 TARGET liblcf::liblcf REQUIRED)
endif()

# Detect all required libraries
# PNG pulls in zlib which has a broken config when not both static and shared library are installed
player_find_package(NAME PNG TARGET PNG::PNG REQUIRED CONFIG_BROKEN)
player_find_package(NAME fmt TARGET fmt::fmt VERSION 5.2 REQUIRED)

# Do not use player_find_package. enable_language used by pixman on Android does not work properly inside functions
find_package(Pixman REQUIRED)
target_link_libraries(${PROJECT_NAME} PIXMAN::PIXMAN)

# Always enable Wine registry support on non-Windows, but not for console ports
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT PLAYER_CONSOLE_PORT)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_WINE=1)
endif()

# freetype and harfbuzz
option(PLAYER_WITH_FREETYPE "Support FreeType font rendering" ON)
cmake_dependent_option(PLAYER_WITH_HARFBUZZ
	"Enable HarfBuzz text shaping (Requires FreeType)" ON
	"PLAYER_WITH_FREETYPE" OFF)

# Freetype is not really broken however only using FT config without HB config causes problems
player_find_package(NAME freetype
	CONDITION PLAYER_WITH_FREETYPE
	DEFINITION HAVE_FREETYPE
	TARGET freetype
	CONFIG_BROKEN)
if(TARGET freetype)
	# Harfbuzz cmake config is broken and deprecated, upstream suggests using pkg-config
	player_find_package(NAME harfbuzz
		CONDITION PLAYER_WITH_HARFBUZZ
		DEFINITION HAVE_HARFBUZZ
		TARGET harfbuzz::harfbuzz
		CONFIG_BROKEN)
endif()

# lzh archive support
option(PLAYER_WITH_LHASA "Support running games in lzh archives" ON)
player_find_package(NAME lhasa
	CONDITION PLAYER_WITH_LHASA
	DEFINITION HAVE_LHASA
	TARGET LHASA::liblhasa)

# json support
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	player_find_package(NAME nlohmann_json
		VERSION 3.9.1
		DEFINITION HAVE_NLOHMANN_JSON
		TARGET nlohmann_json::nlohmann_json
		REQUIRED)
else()
	option(PLAYER_WITH_NLOHMANN_JSON "Support processing of JSON files" ON)
	player_find_package(NAME nlohmann_json
		VERSION 3.9.1
		CONDITION PLAYER_WITH_NLOHMANN_JSON
		DEFINITION HAVE_NLOHMANN_JSON
		TARGET nlohmann_json::nlohmann_json
		ONLY_CONFIG)
endif()

# Configure Audio backends
if(PLAYER_HAS_AUDIO)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SUPPORT_AUDIO=1)

	if(PLAYER_TARGET_PLATFORM STREQUAL "libretro")
		if(WIN32 OR UNIX OR APPLE)
			set(SUPPORT_NATIVE_MIDI ON)
		endif()
	elseif(WIN32 OR PLAYER_MACOS)
		set(SUPPORT_NATIVE_MIDI ON)
	elseif(UNIX)
		find_package(ALSA)
		if(ALSA_FOUND)
			set(SUPPORT_NATIVE_MIDI ON)
		endif()
	endif()

	cmake_dependent_option(PLAYER_WITH_NATIVE_MIDI
		"Play MIDI audio using the API of the operating system" ON
		"SUPPORT_NATIVE_MIDI" OFF)
	if(PLAYER_WITH_NATIVE_MIDI)
		target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_NATIVE_MIDI=1)
		if(WIN32)
			target_link_libraries(${PROJECT_NAME} "winmm")
		elseif(ALSA_FOUND)
			target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_ALSA=1)
			target_sources(${PROJECT_NAME} PRIVATE
				src/platform/linux/midiout_device_alsa.cpp
				src/platform/linux/midiout_device_alsa.h)
			target_link_libraries(${PROJECT_NAME} ALSA::ALSA)
			find_package(Threads)
			if(Threads_FOUND)
				target_link_libraries(${PROJECT_NAME} Threads::Threads)
			endif()
		endif()
	endif()

	# Provide fmmidi options
	option(PLAYER_ENABLE_FMMIDI "Enable internal MIDI sequencer. \
		Will be used when external MIDI library fails." ON)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		$<$<BOOL:${PLAYER_ENABLE_FMMIDI}>:WANT_FMMIDI=1>)
endif()

cmake_dependent_option(PLAYER_WITH_MPG123
	"Play MP3 audio with libmpg123" ON
	"PLAYER_HAS_AUDIO;NOT PLAYER_NONFREE" OFF)
cmake_dependent_option(PLAYER_WITH_LIBSNDFILE
	"Play WAV audio with libsndfile" ON
	"PLAYER_HAS_AUDIO;NOT PLAYER_NONFREE" OFF)
cmake_dependent_option(PLAYER_WITH_OGGVORBIS
	"Play Ogg Vorbis audio with libvorbis" ON
	"PLAYER_HAS_AUDIO" OFF)
cmake_dependent_option(PLAYER_WITH_OPUS
	"Play Opus audio with opusfile" ON
	"PLAYER_HAS_AUDIO" OFF)
cmake_dependent_option(PLAYER_WITH_WILDMIDI
	"Play MIDI audio with wildmidi" ON
	"PLAYER_HAS_AUDIO;NOT PLAYER_NONFREE" OFF)
cmake_dependent_option(PLAYER_WITH_FLUIDSYNTH
	"Play MIDI audio with fluidsynth" ON
	"PLAYER_HAS_AUDIO;NOT PLAYER_NONFREE" OFF)
cmake_dependent_option(PLAYER_WITH_FLUIDLITE
	"Play MIDI audio with fluidlite" ON
	"PLAYER_HAS_AUDIO;NOT PLAYER_NONFREE" OFF)
cmake_dependent_option(PLAYER_WITH_XMP
	"Play MOD audio with libxmp" ON
	"PLAYER_HAS_AUDIO" OFF)
cmake_dependent_option(PLAYER_ENABLE_DRWAV
	"Play WAV audio with dr_wav (built-in). Unsupported files are played by libsndfile." ON
	"PLAYER_HAS_AUDIO" OFF)

if(PLAYER_HAS_AUDIO)
	set(RESAMPLER_PROP "Auto speexdsp samplerate OFF")
	set(PLAYER_AUDIO_RESAMPLER "Auto" CACHE STRING
		"Audio resampler to use. Options: ${RESAMPLER_PROP}")
	set_property(CACHE PLAYER_AUDIO_RESAMPLER PROPERTY STRINGS Auto speexdsp samplerate OFF)

	if(PLAYER_AUDIO_RESAMPLER STREQUAL "Auto")
		set(PLAYER_AUDIO_RESAMPLER_IS_AUTO ON)
	endif()

	cmake_dependent_option(PLAYER_WITH_SPEEXDSP
		"Use speexdsp as resampler when available." ON
		"PLAYER_HAS_AUDIO;PLAYER_AUDIO_RESAMPLER_IS_AUTO" OFF)
	cmake_dependent_option(PLAYER_WITH_SAMPLERATE
		"Use libsamplerate as resampler when available." ON
		"PLAYER_HAS_AUDIO;PLAYER_AUDIO_RESAMPLER_IS_AUTO" OFF)

	if(PLAYER_AUDIO_RESAMPLER_IS_AUTO)
		player_find_package(NAME speexdsp
			CONDITION PLAYER_WITH_SPEEXDSP
			DEFINITION HAVE_LIBSPEEXDSP
			TARGET speexdsp::speexdsp)
		if(NOT TARGET speexdsp::speexdsp)
			player_find_package(
				NAME Samplerate
				CONDITION PLAYER_WITH_SAMPLERATE
				DEFINITION HAVE_LIBSAMPLERATE
				TARGET Samplerate::Samplerate)
		endif()
	elseif(PLAYER_AUDIO_RESAMPLER STREQUAL "speexdsp")
		player_find_package(NAME speexdsp
			DEFINITION HAVE_LIBSPEEXDSP
			TARGET speexdsp::speexdsp
			REQUIRED)
	elseif(PLAYER_AUDIO_RESAMPLER STREQUAL "samplerate")
		player_find_package(NAME Samplerate
			DEFINITION HAVE_LIBSAMPLERATE
			TARGET Samplerate::Samplerate
			REQUIRED)
	elseif(NOT PLAYER_AUDIO_RESAMPLER)
		# no-op
	else()
		message(FATAL_ERROR "Invalid Audio Resampler: ${PLAYER_AUDIO_RESAMPLER}\n"
			" Options: ${RESAMPLER_PROP}")
	endif()
	unset(RESAMPLER_PROP)

	# mpg123
	player_find_package(NAME mpg123
		CONDITION PLAYER_WITH_MPG123
		DEFINITION HAVE_LIBMPG123
		TARGET MPG123::libmpg123)

	# libsndfile
	player_find_package(NAME SndFile
		CONDITION PLAYER_WITH_LIBSNDFILE
		DEFINITION HAVE_LIBSNDFILE
		TARGET SndFile::sndfile)

	# libvorbis
	player_find_package(NAME Vorbis
		CONDITION PLAYER_WITH_OGGVORBIS
		DEFINITION HAVE_OGGVORBIS
		TARGET Vorbis::vorbisfile)

	if(NOT TARGET Vorbis::vorbisfile)
		player_find_package(NAME Tremor
			CONDITION PLAYER_WITH_OGGVORBIS
			DEFINITION HAVE_TREMOR
			TARGET Tremor::Tremor)
	endif()

	# opusfile
	player_find_package(NAME OpusFile
		CONDITION PLAYER_WITH_OPUS
		DEFINITION HAVE_OPUS
		TARGET OpusFile::opusfile)

	# wildmidi
	player_find_package(NAME WildMidi
		CONDITION PLAYER_WITH_WILDMIDI
		DEFINITION HAVE_LIBWILDMIDI
		TARGET "WildMidi::libwildmidi;WildMidi::libwildmidi-static")

	# fluidsynth
	player_find_package(NAME FluidSynth
		CONDITION PLAYER_WITH_FLUIDSYNTH
		DEFINITION HAVE_FLUIDSYNTH
		TARGET FluidSynth::libfluidsynth)

	# fluidlite
	if(NOT TARGET FluidSynth::libfluidsynth)
		player_find_package(NAME FluidLite
			CONDITION PLAYER_WITH_FLUIDLITE
			DEFINITION HAVE_FLUIDLITE
			TARGET "fluidlite::fluidlite;fluidlite::fluidlite-static")
	endif()

	# xmp (lite)
	player_find_package(NAME XMP
		VERSION 4.5.0
		CONDITION PLAYER_WITH_XMP
		DEFINITION HAVE_LIBXMP
		TARGET XMP::XMP)

	if(PLAYER_ENABLE_FMMIDI)
		target_sources(${PROJECT_NAME} PRIVATE
			src/decoder_fmmidi.cpp
			src/decoder_fmmidi.h
			src/midiprogram.h
			src/midisynth.cpp
			src/midisynth.h)
	endif()

	if(PLAYER_ENABLE_DRWAV)
		target_compile_definitions(${PROJECT_NAME} PUBLIC WANT_DRWAV=1)
		target_sources(${PROJECT_NAME} PRIVATE
			src/decoder_drwav.cpp
			src/decoder_drwav.h
			src/external/dr_wav.h)
	endif()
endif()

# Fonts
option(PLAYER_WITH_BAEKMUK "Enable built-in Baekmuk font (Korean)" ON)
cmake_dependent_option(PLAYER_WITH_WQY
	"Enable built-in WenQuanYi font (Chinese)" ON
	"NOT PLAYER_NONFREE" OFF)
target_compile_definitions(${PROJECT_NAME} PUBLIC
	$<$<BOOL:${PLAYER_WITH_BAEKMUK}>:WANT_FONT_BAEKMUK=1>)
target_compile_definitions(${PROJECT_NAME} PUBLIC
	$<$<BOOL:${PLAYER_WITH_WQY}>:WANT_FONT_WQY=1>)

# Executable
if(PLAYER_BUILD_EXECUTABLE AND ${PLAYER_TARGET_PLATFORM} MATCHES "^SDL.*$" AND NOT PLAYER_CONSOLE_PORT)
	if(APPLE)
		set(EXE_NAME "EasyRPG-Player.app")
		set_source_files_properties(${${PROJECT_NAME}_BUNDLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
		add_executable(${EXE_NAME} MACOSX_BUNDLE "src/platform/sdl/main.cpp" ${${PROJECT_NAME}_BUNDLE_ICON})
		configure_file(resources/macos/Info.plist.in resources/macos/Info.plist @ONLY)
		set_target_properties(${EXE_NAME} PROPERTIES
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/resources/macos/Info.plist")
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "${PLAYER_NAME}")
	else()
		set(EXE_NAME ${PROJECT_NAME}_exe)
		if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
			add_executable(${EXE_NAME} "src/platform/emscripten/main.cpp")
		else()
			add_executable(${EXE_NAME} "src/platform/sdl/main.cpp")
		endif()
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "easyrpg-player")
	endif()

	if(WIN32)
		# Open console for Debug builds
		set_target_properties(${EXE_NAME} PROPERTIES WIN32_EXECUTABLE $<$<NOT:$<CONFIG:Debug>>:TRUE>)

		# Add resources
		string(REPLACE "." "," RC_VERSION ${PROJECT_VERSION})
		string(TIMESTAMP RC_YEAR "%Y")
		configure_file(resources/windows/player.rc.in resources/windows/player.rc @ONLY)
		target_sources(${EXE_NAME} PRIVATE
			resources/windows/player.manifest
			"${CMAKE_CURRENT_BINARY_DIR}/resources/windows/player.rc")

		# Set Visual Studio startup project
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})

		# Change executable name
		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "Player")
	endif()

	target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		# Do not confuse this with our own -DUSE_SDL
		# Prevent Emscripten from pulling in bundled SDL headers that break the build
		target_compile_options(${PROJECT_NAME} PUBLIC "-sUSE_SDL=0")

		set(PLAYER_JS_PREJS "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-pre.js")
		set(PLAYER_JS_POSTJS "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-post.js")
		set(PLAYER_JS_SHELL "${CMAKE_CURRENT_SOURCE_DIR}/resources/emscripten/emscripten-shell.html")

		set_property(TARGET ${EXE_NAME} PROPERTY LINK_FLAGS
			"-sALLOW_MEMORY_GROWTH -sMINIFY_HTML=0 -sMODULARIZE -sEXPORT_NAME=createEasyRpgPlayer \
			 -sEXIT_RUNTIME --bind --pre-js ${PLAYER_JS_PREJS} --post-js ${PLAYER_JS_POSTJS} \
			 -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=['$autoResumeAudioContext','$dynCall'] \
			 -sEXPORTED_RUNTIME_METHODS=['FS'] \
			 -sEXPORTED_FUNCTIONS=_main,_malloc,_free")
		set_source_files_properties("src/platform/sdl/main.cpp" PROPERTIES
			OBJECT_DEPENDS "${PLAYER_JS_PREJS};${PLAYER_JS_POSTJS};${PLAYER_JS_SHELL}")

		if(PLAYER_JS_BUILD_SHELL)
			set_target_properties(${EXE_NAME} PROPERTIES SUFFIX ".html")
			set_property(TARGET ${EXE_NAME} APPEND_STRING PROPERTY LINK_FLAGS " --shell-file ${PLAYER_JS_SHELL}")
		endif()

		target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions)

		target_sources(${PROJECT_NAME} PRIVATE
				src/platform/emscripten/clock.h
				src/platform/emscripten/interface.cpp
				src/platform/emscripten/interface.h)

		target_link_libraries(${EXE_NAME} "idbfs.js")

		set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "${PLAYER_JS_OUTPUT_NAME}")
	endif()

	# installation
	include(GNUInstallDirs)
	if(APPLE)
		install(TARGETS ${EXE_NAME} RUNTIME DESTINATION BUNDLE DESTINATION "${CMAKE_BINARY_DIR}/Package")
	else()
		install(TARGETS ${EXE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()

	if(UNIX)
		# Install desktop entry, icons and metainfo
		install(FILES resources/unix/easyrpg-player.desktop
			DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
		install(FILES resources/unix/icon-48.png RENAME easyrpg-player.png
			DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps) # legacy
		install(FILES resources/unix/icon-48.png RENAME easyrpg-player.png
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps)
		install(FILES resources/unix/easyrpg-player.svg
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
		install(FILES resources/unix/easyrpg-player.metainfo.xml
			DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	endif()

	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:${EXE_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
		# Emscripten does not install the wasm file (or the js file when building a shell)
		if(PLAYER_JS_BUILD_SHELL)
			install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLAYER_JS_OUTPUT_NAME}.js
				DESTINATION ${CMAKE_INSTALL_BINDIR})
		endif()
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PLAYER_JS_OUTPUT_NAME}.wasm
			DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()
elseif(PLAYER_CONSOLE_PORT AND NOT PLAYER_TARGET_PLATFORM STREQUAL "libretro")
	if(NOT CPACK_PLATFORM)
		set(CPACK_PLATFORM "${PLAYER_TARGET_PLATFORM}")
	endif()

	install(TARGETS easyrpg-player RUNTIME DESTINATION . COMPONENT debug)

	# debug information
	if(PLAYER_DEVKITPRO)
		dkp_target_generate_symbol_list(easyrpg-player)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.map
				${CMAKE_CURRENT_BINARY_DIR}/easyrpg-player.lst
			DESTINATION . COMPONENT debug)
	endif()

	# packaging for distribution
	set(CPACK_GENERATOR "ZIP")
	set(CPACK_SOURCE_GENERATOR ";") # disable source archive
	set(CPACK_ARCHIVE_COMPONENT_INSTALL ON) # split archives
	set(CPACK_PACKAGE_FILE_NAME "easyrpg-player")
	if(PLAYER_VERSIONED_PACKAGES)
		string(APPEND CPACK_PACKAGE_FILE_NAME "-${PLAYER_VERSION}")
	endif()
	set(CPACK_ARCHIVE_DEBUG_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${CPACK_PLATFORM}-debug")
	set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # we do this manually
	include(CPack)
else() # library
	if(PLAYER_TARGET_PLATFORM STREQUAL "libretro")
		add_library(easyrpg_libretro
			src/platform/libretro/audio.cpp
			src/platform/libretro/audio.h
			src/platform/libretro/clock.cpp
			src/platform/libretro/clock.h
			src/platform/libretro/input_buttons.cpp
			src/platform/libretro/ui.cpp
			src/platform/libretro/ui.h
			src/platform/libretro/midiout_device.cpp
			src/platform/libretro/midiout_device.h)
		set_property(SOURCE src/platform/libretro/ui.cpp
			PROPERTY COMPILE_DEFINITIONS PLAYER_VERSION_FULL="${PLAYER_VERSION_FULL}")
		target_link_libraries(easyrpg_libretro ${PROJECT_NAME})
		set_target_properties(easyrpg_libretro PROPERTIES PREFIX "")
		set(PLAYER_TEST_LIBRARIES "easyrpg_libretro")

		if(WIN32)
			set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT easyrpg_libretro)
		endif()
	elseif(ANDROID AND ${PLAYER_TARGET_PLATFORM} MATCHES "^SDL.*$")
		add_library(easyrpg_android
			src/platform/android/android.cpp
			src/platform/android/android.h
			src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.cpp
			src/platform/android/org_easyrpg_player_player_EasyRpgPlayerActivity.h
			src/platform/android/filesystem_apk.cpp
			src/platform/android/filesystem_apk.h
			src/platform/android/filesystem_saf.cpp
			src/platform/android/filesystem_saf.h
			src/platform/sdl/main.cpp)
		target_link_libraries(easyrpg_android ${PROJECT_NAME})
		set_target_properties(easyrpg_android PROPERTIES DEBUG_POSTFIX "")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/builds/android/app/src/gamebrowser)
		set(PLAYER_TEST_LIBRARIES "easyrpg_android")
	else()
		message(FATAL_ERROR "Unsupported library target platform: ${PLAYER_TARGET_PLATFORM}")
	endif()
endif()

# manpage (target "man")
set(MAN_NAME easyrpg-player.6)
find_program(ASCIIDOCTOR_EXECUTABLE asciidoctor)
set(MANUAL_STATUS "Unavailable")
if(ASCIIDOCTOR_EXECUTABLE)
	add_custom_command(OUTPUT resources/unix/${MAN_NAME}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/resources/unix
		COMMAND ${ASCIIDOCTOR_EXECUTABLE} -a player_version="${PLAYER_VERSION}"
			-b manpage -D ${CMAKE_CURRENT_BINARY_DIR}/resources/unix
			${CMAKE_CURRENT_SOURCE_DIR}/resources/unix/${MAN_NAME}.adoc
		DEPENDS resources/unix/${MAN_NAME}.adoc
		COMMENT "(Re-)building manpage ${MAN_NAME}"
		VERBATIM)
	if(UNIX)
		add_custom_target(player_man ALL DEPENDS resources/unix/${MAN_NAME})
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/unix/${MAN_NAME}
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
	else()
		add_custom_target(player_man DEPENDS resources/unix/${MAN_NAME})
	endif()
	# Handling with extra target, because a dependency could already provide "man"
	if(NOT TARGET man)
		add_custom_target(man)
	endif()
	add_dependencies(man player_man)
	set(MANUAL_STATUS "Generated")
else()
	# no asciidoctor, distribution archive?
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/unix/${MAN_NAME})
		install(FILES resources/unix/${MAN_NAME} DESTINATION share/man/man6)
		set(MANUAL_STATUS "Bundled")
	endif()
endif()

# bash completion
set(BASHCOMPLETION_STATUS "Unavailable")
find_package(bash-completion QUIET)
if(BASH_COMPLETION_FOUND OR (UNIX AND NOT APPLE))
	set(BASHCOMPLETION_STATUS "Available")
	install(FILES resources/unix/bash-completion/easyrpg-player
		DESTINATION "${CMAKE_INSTALL_DATADIR}/bash-completion/completions")
endif()

# Doxygen (target "doc")
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(resources/Doxyfile.in resources/Doxyfile @ONLY)

	add_custom_target(player_doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/resources/Doxyfile
		DEPENDS ${PROJECT_NAME}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	if(NOT TARGET doc)
		# Handle liblcf doc target
		add_custom_target(doc)
	endif()
	add_dependencies(doc player_doc)
endif()

# Unit tests (target "check")
option(PLAYER_ENABLE_TESTS "Execute unit tests after compilation finishes" ON)
if(PLAYER_ENABLE_TESTS)
	add_subdirectory(tests)
endif()

# Instrumentation framework
set(PLAYER_ENABLE_INSTRUMENTATION "OFF" CACHE STRING "Build performance instrumentation hooks")
set_property(CACHE PLAYER_ENABLE_INSTRUMENTATION PROPERTY STRINGS OFF VTune)
if(NOT ${PLAYER_ENABLE_INSTRUMENTATION} STREQUAL "OFF")
	target_compile_definitions(${PROJECT_NAME} PUBLIC PLAYER_INSTRUMENTATION=${PLAYER_ENABLE_INSTRUMENTATION})
	if(${PLAYER_ENABLE_INSTRUMENTATION} STREQUAL "VTune")
		player_find_package(NAME VTune TARGET VTune::ITT REQUIRED)
	endif()
endif()

# Benchmarks
option(PLAYER_ENABLE_BENCHMARKS "Build benchmarks" OFF)
if(PLAYER_ENABLE_BENCHMARKS)
	add_subdirectory(bench)
endif()

# Print summary
message(STATUS "")
message(STATUS "${PLAYER_NAME} version ${PLAYER_VERSION} has been configured --")
message(STATUS "Git info: ${GIT_STATUS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(TARGET_STATUS "${PLAYER_TARGET_PLATFORM}")
if(NINTENDO_WIIU)
	set(TARGET_STATUS "Wii U (SDL2)")
elseif(NINTENDO_WII)
	set(TARGET_STATUS "Wii (${TARGET_STATUS})")
elseif(AMIGA)
	set(TARGET_STATUS "Amiga (SDL1)")
endif()
message(STATUS "Target system: ${TARGET_STATUS}")
if(PLAYER_BUNDLE)
	message(STATUS "Embedding directory \"${PLAYER_BUNDLE_PATH}\"")
endif()
message(STATUS "")

if(PLAYER_BUILD_LIBLCF)
	message(STATUS "liblcf: Built from ${PLAYER_BUILD_LIBLCF_GIT}")
	message(STATUS "")
endif()

message(STATUS "Audio backend: ${PLAYER_AUDIO_BACKEND}")
if(PLAYER_HAS_AUDIO)
	message(STATUS "")
	if(TARGET SndFile::sndfile)
		list(APPEND WAV_LIBS "libsndfile")
	endif()
	if(PLAYER_ENABLE_DRWAV)
		list(APPEND WAV_LIBS "built-in (dr_wav)")
	endif()
	player_summary_list("WAV playback" WAV_LIBS)

	if(TARGET FluidSynth::libfluidsynth)
		list(APPEND MIDI_LIBS "FluidSynth")
	endif()
	if(TARGET fluidlite::fluidlite OR TARGET fluidlite::fluidlite-static)
		list(APPEND MIDI_LIBS "FluidLite")
	endif()
	if(TARGET WildMidi::libwildmidi OR TARGET WildMidi::libwildmidi-static)
		list(APPEND MIDI_LIBS "WildMidi")
	endif()
	if(PLAYER_WITH_NATIVE_MIDI)
		list(APPEND MIDI_LIBS "Native")
	endif()
	if(PLAYER_ENABLE_FMMIDI)
		list(APPEND MIDI_LIBS "built-in (FmMidi)")
	endif()
	player_summary_list("MIDI playback" MIDI_LIBS)
	player_summary_line("MP3 playback" "TARGET MPG123::libmpg123" mpg123)
	player_summary_line("Ogg Vorbis playback"
		"TARGET Vorbis::vorbisfile" libvorbis
		"TARGET Tremor::Tremor" tremor)
	player_summary_line("MOD playback" "TARGET XMP::XMP" libxmp)
	player_summary_line("Opus playback" "TARGET OpusFile::opusfile" opusfile)
	player_summary_line("Resampler"
		"TARGET speexdsp::speexdsp" speexdsp
		"TARGET Samplerate::Samplerate" libsamplerate)
endif()

message(STATUS "")
player_summary_line("Font rendering"
	"TARGET harfbuzz::harfbuzz" "Freetype with Harfbuzz / built-in"
	"TARGET freetype" "Freetype and built-in" "built-in")
player_summary_line("LZH archive support" "TARGET LHASA::liblhasa" lhasa)
player_summary_line("JSON support" "TARGET nlohmann_json::nlohmann_json" nlohmann_json)

message(STATUS "")
message(STATUS "Manual page: ${MANUAL_STATUS}")
message(STATUS "Bash completion: ${BASHCOMPLETION_STATUS}")
player_summary_line("Doxygen" DOXYGEN_FOUND "Available (target \"doc\")" "Unavailable")
message(STATUS "")

if(${PLAYER_TARGET_PLATFORM} STREQUAL "SDL1")
	message(WARNING " SDL1 is deprecated!\n Please migrate to SDL2 or SDL3!")
endif()
