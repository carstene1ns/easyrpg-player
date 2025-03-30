/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <cstdlib>
#include "main_data.h"
#include "filefinder.h"
#include "filefinder_rtp.h"
#include "filesystem.h"
#include "engine/destiny.h"
#include "engine/system.h"
#include "engine/actors.h"
#include "engine/party.h"
#include "engine/enemyparty.h"
#include "engine/dynrpg.h"
#include "engine/ineluki.h"
#include "engine/hero.h"
#include "engine/screen.h"
#include "engine/pictures.h"
#include "engine/map.h"
#include "engine/variables.h"
#include "engine/strings.h"
#include "engine/switches.h"
#include "engine/targets.h"
#include "engine/quit.h"
#include "engine/windows.h"
#include "font.h"
#include "player.h"
#include "system.h"
#include "output.h"

#ifndef _WIN32
#  include <unistd.h>
#endif
#if defined(USE_SDL) && defined(__ANDROID__)
#  include <jni.h>
#  include <SDL_system.h>
#elif defined(__APPLE__) && TARGET_OS_OSX
#  include <sys/syslimits.h>
#  include "platform/macos/macos_utils.h"
#endif

// Global variables.
std::string project_path;

namespace Main_Data {
	// Dynamic Game lcf::Data
	std::unique_ptr<Game_System> game_system;
	std::unique_ptr<Game_Switches> game_switches;
	std::unique_ptr<Game_Variables> game_variables;
	std::unique_ptr<Game_Strings> game_strings;
	std::unique_ptr<Game_Screen> game_screen;
	std::unique_ptr<Game_Pictures> game_pictures;
	std::unique_ptr<Game_Windows> game_windows;
	std::unique_ptr<Game_Actors> game_actors;
	std::unique_ptr<Game_Hero> game_hero;
	std::unique_ptr<Game_Party> game_party;
	std::unique_ptr<Game_EnemyParty> game_enemyparty;
	std::unique_ptr<Game_Targets> game_targets;
	std::unique_ptr<Game_Quit> game_quit;
	std::unique_ptr<Game_DynRpg> game_dynrpg;
	std::unique_ptr<Game_Ineluki> game_ineluki;
	std::unique_ptr<Game_Destiny> game_destiny;
	bool global_save_opened = false;
	std::unique_ptr<Game_Switches> game_switches_global;
	std::unique_ptr<Game_Variables> game_variables_global;

	std::unique_ptr<FileFinder_RTP> filefinder_rtp;
}

void Main_Data::Init() {
	if (project_path.empty()) {
		// First use environment variables
		project_path =
			getenv("RPG_TEST_GAME_PATH") ? getenv("RPG_TEST_GAME_PATH") :
			getenv("RPG_GAME_PATH") ? getenv("RPG_GAME_PATH") :
			"";

		if (project_path.empty()) {
			// Set to current directory
			project_path = "";

#if defined(PLAYER_AMIGA)
			// Working directory not correctly handled
			char working_dir[256];
			getcwd(working_dir, 255);
			project_path = std::string(working_dir);
#elif defined(__APPLE__) && TARGET_OS_OSX
			// Apple Finder does not set the working directory
			// It points to HOME instead. When it is HOME change it to
			// the application directory instead

			char* home = getenv("HOME");
			char current_dir[PATH_MAX] = { 0 };
			getcwd(current_dir, sizeof(current_dir));
			if (strcmp(current_dir, "/") == 0 || strcmp(current_dir, home) == 0) {
				project_path = MacOSUtils::GetBundleDir();
			}
#endif
		}
	}
}

void Main_Data::Cleanup() {
	Game_Map::Quit();

	game_switches.reset();
	game_screen.reset();
	game_pictures.reset();
	game_windows.reset();
	game_hero.reset();
	game_party.reset();
	game_enemyparty.reset();
	game_actors.reset();
	game_targets.reset();
	game_quit.reset();
	game_system.reset();
	game_dynrpg.reset();
	game_ineluki.reset();
	game_destiny.reset();
	global_save_opened = false;
	game_switches_global.reset();
	game_variables_global.reset();
}

const std::string& Main_Data::GetDefaultProjectPath() {
	return project_path;
}
