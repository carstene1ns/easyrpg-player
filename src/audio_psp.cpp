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

#ifdef PSPUI

#include <cstring>
#include <malloc.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pspaudiolib.h>
#include <pspdebug.h>

#include "baseui.h"
#include "player.h"
#include "audio_psp.h"
#include "audio_decoder.h"
#include "filefinder.h"
#include "output.h"

/* 16-bit, stereo */
namespace {
	void audioCallback(void* buf, unsigned int length, void *userdata) {
		PspAudio* audio = static_cast<PspAudio*>(userdata);
		int vol = 0x4000 * audio->GetDecoder()->GetVolume() / 100;
		pspAudioSetVolume(0, vol, vol);

		int freq, channels;
		AudioDecoder::Format format;
		audio->GetDecoder()->GetFormat(freq, format, channels);

		int out_len = -1;

		if (channels == 2) {
			out_len = audio->GetDecoder()->Decode((uint8_t *)buf, length * 4);
		} else if(channels == 1) {
			out_len = audio->GetDecoder()->Decode((uint8_t *)buf, length * 2);

			if (out_len > 0) {
				// stereo interleaving
				int pos = out_len / 2;
				uint16_t * bp = (uint16_t *) buf;
				while (pos > 0) {
					bp[pos * 2] = bp[pos];
					bp[pos * 2 - 1] = bp[pos];
					pos--;
				}
			}
		}
		if (out_len == -1) {
			Output::Warning("Couldn't decode BGM.\n%s", audio->GetDecoder()->GetError().c_str());
			pspAudioSetChannelCallback(0, NULL, NULL);
			return;
		}

		if (audio->GetDecoder()->IsFinished()) {
			pspAudioSetChannelCallback(0, NULL, NULL);
		}
	}
}

PspAudio::PspAudio() {
	pspAudioInit();
}

PspAudio::~PspAudio() {
	pspAudioEnd();
}

void PspAudio::BGM_Play(std::string const& file, int volume, int pitch, int fadein) {
	FILE* filehandle = FileFinder::fopenUTF8(file, "rb");
	if (!filehandle) {
		Output::Warning("Music not readable: %s", file.c_str());
		return;
	}

	audio_decoder = AudioDecoder::Create(filehandle, file);
	if (audio_decoder) {
		if (!audio_decoder->Open(filehandle)) {
			Output::Warning("Couldn't play %s BGM.\n%s", file.c_str(), audio_decoder->GetError().c_str());
			audio_decoder.reset();
			return;
		}

		// Can't use BGM_Stop here because it destroys the audio_decoder
		pspAudioSetChannelCallback(0, NULL, NULL);

		audio_decoder->SetLooping(true);
		bgm_starttick = DisplayUi->GetTicks();

		audio_decoder->SetFormat(44100, AudioDecoder::Format::S16, 2);
		audio_decoder->SetFade(0, volume, fadein);
		audio_decoder->SetPitch(pitch);

		pspAudioSetChannelCallback(0, audioCallback, this);
		return;
	}
	fclose(filehandle);
}

void PspAudio::BGM_Pause() {
	if (audio_decoder) {
		audio_decoder->Pause();
		return;
	}
}

void PspAudio::BGM_Resume() {
	if (audio_decoder) {
		bgm_starttick = DisplayUi->GetTicks();
		audio_decoder->Resume();
		return;
	}
}

void PspAudio::BGM_Stop() {
	pspAudioSetChannelCallback(0, NULL, NULL);
	audio_decoder.reset();
}

bool PspAudio::BGM_PlayedOnce() const {
	if (audio_decoder) {
		return (audio_decoder->GetLoopCount() > 0);
	}

	return false;
}

bool PspAudio::BGM_IsPlaying() const {
	if (audio_decoder)
		return true;

	return false;
}

unsigned PspAudio::BGM_GetTicks() const {
	if (audio_decoder) {
		return audio_decoder->GetTicks();
	}

	return 0;
}

void PspAudio::BGM_Volume(int volume) {
	if (audio_decoder) {
		audio_decoder->SetVolume(volume);
		return;
	}
}

void PspAudio::BGM_Pitch(int pitch) {
	if (audio_decoder) {
		audio_decoder->SetPitch(pitch);
	}
}

void PspAudio::BGM_Fade(int fade) {
	if (audio_decoder) {
		bgm_starttick = DisplayUi->GetTicks();
		audio_decoder->SetFade(audio_decoder->GetVolume(), 0, fade);
		return;
	}
}

void PspAudio::SE_Play(std::string const& file, int volume, int /* pitch */) {


}

void PspAudio::SE_Stop() {

}

void PspAudio::Update() {
	if (audio_decoder && bgm_starttick > 0) {
		int t = DisplayUi->GetTicks();
		audio_decoder->Update(t - bgm_starttick);
		bgm_starttick = t;
	}
}

AudioDecoder* PspAudio::GetDecoder() {
	return audio_decoder.get();
}

#endif
