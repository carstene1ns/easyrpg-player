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

#include <string>
#include <vector>
#include "player.h"
#include "utils.h"
#include "output.h"
#include "audio.h"
#include "ui.h"

namespace {
	Game_ConfigAudio empty_conf;
	custom_audio_interface_t empty_audio = {
		.freq = 22050,
		.channels = 2,
		.buffer_size = 512,
		.format = AudioDecoder::Format::S16,

		// no-ops
		.lock = []() {},
		.unlock = []() {},
		.render = [](void*, int) {}
	};

	custom_ui_t empty_ui = {
		// no-ops
		.UpdateDisplay = [](void*, int, int, int) {},
		.SetTitle = [](const std::string&) {}
	};

	void LogCallback(LogLevel lvl, std::string const& msg, LogCallbackUserData /* userdata */) {
		// no-op
	}
}

extern "C" int main(int argc, char* argv[]) {
	std::vector<std::string> args;

	Output::SetLogCallback(LogCallback);

	CustomAudio::SetAudioInterface(&empty_audio);
	#ifdef SUPPORT_AUDIO
		empty_ui.audio = std::make_unique<CustomAudio>(empty_conf);
	#endif

	CustomUi::SetUserInterface(&empty_ui);

	Player::Init(std::move(args));
	Player::Run();

	// invoke main loop
	while(DisplayUi.get()) {
		Player::MainLoop();
	}

	return Player::exit_code;
}
