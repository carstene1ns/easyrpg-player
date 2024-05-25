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

#include "system.h"

#ifdef SUPPORT_AUDIO

#include <cassert>
#include <ao/ao.h>
#include <thread>
#include <mutex>
#include "ao_audio.h"
#include "output.h"

namespace {
	std::thread audio_thread;
	std::mutex audio_mutex;
	bool stop_audio;
	ao_device *device = nullptr;
	int buf_size = 1024;
}

void AudioThread(AoAudio* instance) {
	std::vector<uint8_t> buffer;
	buffer.resize(buf_size);
	size_t i=0;

	while (!stop_audio) {
		instance->LockMutex();
		instance->Decode(buffer.data(), buf_size);
		instance->UnlockMutex();

		if (!ao_play(device, reinterpret_cast<char*>(buffer.data()), buf_size)) {
			Output::Warning("Couldn't play audio.");
			return;
		}
	}
}

AoAudio::AoAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	ao_initialize();

	ao_sample_format format = {};
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	device = ao_open_live(ao_default_driver_id(), &format, nullptr);
	if (device == nullptr) {
		Output::Warning("Couldn't open audio: errno={}", errno);
		return;
	}
	SetFormat(format.rate, AudioDecoder::Format::S16, format.channels);

	// Start Audio
	stop_audio = false;
    audio_thread = std::thread(AudioThread, this);
    if(!audio_thread.joinable()) {
		Output::Warning("Couldn't start audio thread.");
    }
}

AoAudio::~AoAudio() {
	stop_audio = true;
	if (audio_thread.joinable())
		audio_thread.join();

	if(!ao_close(device)) {
		Output::Warning("Problem closing audio device.");
	}
	ao_shutdown();
}

void AoAudio::LockMutex() const {
	audio_mutex.lock();
}

void AoAudio::UnlockMutex() const {
	audio_mutex.unlock();
}

#endif
