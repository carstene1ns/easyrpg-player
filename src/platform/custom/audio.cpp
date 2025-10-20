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

#include <cstdint>

#include "audio.h"
#include "output.h"

namespace {
	CustomAudio* instance = nullptr;
	custom_audio_interface_t* interface = nullptr;
	bool enable_audio = false;
	unsigned int samples_per_frame = 0;
	std::vector<uint8_t> buffer;
}

CustomAudio::CustomAudio(const Game_ConfigAudio& cfg) :
	GenericAudio(cfg)
{
	if (!interface) {
		Output::Error("CustomAudio: Need to provide interface first!");
		return;
	}

	buffer.resize(interface->buffer_size);
	SetFormat(interface->freq, interface->format, interface->channels);
	samples_per_frame = interface->buffer_size / Game_Clock::GetTargetGameFps();
}

CustomAudio::~CustomAudio() {
	EnableAudio(false);
	buffer.clear();
}

void CustomAudio::LockMutex() const {
	interface->lock();
}

void CustomAudio::UnlockMutex() const {
	interface->unlock();
}

void CustomAudio::EnableAudio(bool enabled) {
	enable_audio = enabled;
}

void CustomAudio::AudioCallback() {
	if (!enable_audio) return;

	instance->LockMutex();
	instance->Decode(buffer.data(), samples_per_frame * interface->channels * 2);
	instance->UnlockMutex();

	interface->render(buffer.data(), samples_per_frame);
}

void CustomAudio::SetAudioInterface(custom_audio_interface_t *interf) {
	interface = interf;
}

#endif
