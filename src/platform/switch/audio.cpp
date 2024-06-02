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

#ifdef SUPPORT_AUDIO

#include "audio.h"
#include "output.h"

#include <switch.h>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <malloc.h>

#define ALIGN_TO(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

namespace {
	constexpr int samplerate = 48000;
	constexpr int bytes_per_sample = 4;
	constexpr int samples_per_buf = 4096;
	constexpr int buf_size = samples_per_buf * bytes_per_sample;
	static Thread audio_thread;
	static Mutex audio_mutex;
}

void switch_audio_thread(void* args) {
	NxAudio* instance = (NxAudio *)args;
	uint32_t released_count;
	AudioOutBuffer source_buffers[2], *released_buffer;

	// Init audio buffers
	for (int i = 0; i < 2; i++){
		source_buffers[i].buffer = memalign(0x1000, ALIGN_TO(buf_size, 0x1000));
		if (!source_buffers[i].buffer) {
			Output::Error("Could not create audio buffers!");
			return;
		}
		source_buffers[i].next = nullptr;
		source_buffers[i].buffer_size = buf_size;
		source_buffers[i].data_size = buf_size;
		source_buffers[i].data_offset = 0;

		// Fill in first portion of audio
		instance->LockMutex();
		instance->Decode((uint8_t*)source_buffers[i].buffer, buf_size);
		instance->UnlockMutex();
		audoutAppendAudioOutBuffer(&source_buffers[i]);
	}

	// Render audio until termination requested
	while(instance->want_audio) {
		audoutWaitPlayFinish(&released_buffer, &released_count, UINT64_MAX);
		instance->LockMutex();
		instance->Decode((uint8_t*)released_buffer->buffer, buf_size);
		instance->UnlockMutex();
		audoutAppendAudioOutBuffer(released_buffer);
	}

	// Free memory
	free(source_buffers[0].buffer);
	free(source_buffers[1].buffer);
}

NxAudio::NxAudio(const Game_ConfigAudio& cfg) : GenericAudio(cfg) {
	// Setup GenericAudio
	SetFormat(samplerate, AudioDecoder::Format::S16, 2);

	// Initialize audio service
	audoutInitialize();
	audoutStartAudioOut();
	mutexInit(&audio_mutex);

	// Start streaming thread
	want_audio = true;
	threadCreate(&audio_thread, switch_audio_thread, this, nullptr, 0x10000, 0x2B, -2);
	if (R_FAILED(threadStart(&audio_thread))) {
		Output::Error("Failed to init audio thread.");
		return;
	}
}

NxAudio::~NxAudio() {
	// Close streaming thread
	want_audio = false;
	threadWaitForExit(&audio_thread);
	threadClose(&audio_thread);

	// Terminate audio service
	audoutStopAudioOut();
	audoutExit();
}

void NxAudio::LockMutex() const {
	mutexLock(&audio_mutex);
}

void NxAudio::UnlockMutex() const {
	mutexUnlock(&audio_mutex);
}

#endif
