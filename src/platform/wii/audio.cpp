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

#include <cstring>
#include <malloc.h>

#include "audio.h"
#include "filefinder.h"
#include "game_clock.h"
#include "output.h"
#include "audio_secache.h"

namespace {
	constexpr int BGM_BUFS = 2;
	constexpr int MAX_SES = 20;
	static void *bgm_buf[BGM_BUFS];
	Game_Clock::time_point bgm_starttick;
	AESNDPB *bgm_voice = nullptr;
	struct se_voice {
		AESNDPB *pb = nullptr;
		bool running = false;
		void *buf = nullptr;
	};
	struct se_voice se_voices[MAX_SES];
	bool dsp_inited = false;
	std::unique_ptr<AudioDecoderBase> bgm_decoder;
	constexpr int SNDBUFFERSIZE = 5760;
	int cur_bgm_buf = 0;

	constexpr int AUDIOSTACK = 16384*2;
	lwpq_t audioqueue;
	lwp_t athread = LWP_THREAD_NULL;
	uint8_t astack[AUDIOSTACK];
	bool stopaudio = false;
	mutex_t bgm_mutex;
}

static void *AudioThread (void *) {
	while (!stopaudio) {
		// clear old data
		memset(bgm_buf[cur_bgm_buf], 0, SNDBUFFERSIZE);

		if (bgm_decoder) {
			LWP_MutexLock(bgm_mutex);
			bgm_decoder->Decode((uint8_t *) bgm_buf[cur_bgm_buf], SNDBUFFERSIZE);
			LWP_MutexUnlock(bgm_mutex);
		}
		// make sure data is in main memory
		DCFlushRange(bgm_buf[cur_bgm_buf], SNDBUFFERSIZE);

		LWP_ThreadSleep(audioqueue);
	}
	return nullptr;
}

u32 get_voice_format(AudioDecoder::Format format, int channels, AudioDecoder::Format &out_format) {
	out_format = format;
	u32 aesnd_format;

	switch (format) {
		case AudioDecoder::Format::U8:
			aesnd_format = channels == 2 ? VOICE_STEREO8_UNSIGNED : VOICE_MONO8_UNSIGNED;
			break;
		case AudioDecoder::Format::S8:
			aesnd_format = channels == 2 ? VOICE_STEREO8 : VOICE_MONO8;
			break;
		case AudioDecoder::Format::U16:
			aesnd_format = channels == 2 ? VOICE_STEREO16_UNSIGNED : VOICE_MONO16_UNSIGNED;
			break;
		default:
			// default format
			aesnd_format = channels == 2 ? VOICE_STEREO16 : VOICE_MONO16;
			out_format = AudioDecoder::Format::S16;
			break;
	}

	return aesnd_format;
}

void WiiAudio::BGM_Play(Filesystem_Stream::InputStream filestream, int volume, int pitch, int fadein) {
	if (!dsp_inited)
		return;

	if (!filestream) {
		Output::Warning("Couldn't play BGM {}: File not readable", filestream.GetName());
		return;
	}

	bgm_decoder = AudioDecoder::Create(filestream);
	if (!bgm_decoder || !bgm_decoder->Open(std::move(filestream))) {
		Output::Warning("Couldn't play BGM {}: Format not supported", filestream.GetName());
		return;
	}

	int frequency;
	AudioDecoder::Format format, out_format;
	int channels;

	bgm_decoder->SetPitch(pitch);
	bgm_decoder->GetFormat(frequency, format, channels);
	bgm_decoder->SetVolume(0);
	bgm_decoder->SetFade(volume, std::chrono::milliseconds(fadein));
	bgm_decoder->SetLooping(true);

	AESND_SetVoiceFormat(bgm_voice, get_voice_format(format, channels, out_format));
	if (format != out_format)
		bgm_decoder->SetFormat(frequency, out_format, channels);
	AESND_SetVoiceFrequency(bgm_voice, frequency);
	AESND_SetVoiceStream(bgm_voice, true);
	AESND_SetVoiceStop(bgm_voice, false);
}

void WiiAudio::BGM_Pause()  {
	if (!dsp_inited)
		return;

	AESND_SetVoiceStop(bgm_voice, true);
}

void WiiAudio::BGM_Resume()  {
	if (!dsp_inited)
		return;

	bgm_starttick = Game_Clock::now();
	AESND_SetVoiceStop(bgm_voice, false);
}

void WiiAudio::BGM_Stop() {
	if (!dsp_inited)
		return;

	bgm_decoder.reset();
}

bool WiiAudio::BGM_PlayedOnce() const {
	if (!bgm_decoder)
		return true;

	return bgm_decoder->GetLoopCount() > 0;
}

bool WiiAudio::BGM_IsPlaying() const {
	if (!bgm_decoder)
		return false;

	return !bgm_decoder->IsFinished();
}

int WiiAudio::BGM_GetTicks() const {
	if (!bgm_decoder)
		return 0;

	return bgm_decoder->GetTicks();
}

void WiiAudio::BGM_Fade(int fade) {
	if (!bgm_decoder)
		return;

	bgm_starttick = Game_Clock::now();
	bgm_decoder->SetFade(0, std::chrono::milliseconds(fade));
}

void WiiAudio::BGM_Volume(int volume) {
	if (!bgm_decoder)
		return;

	AESND_SetVoiceVolume(bgm_voice, volume*2.55, volume*2.55);
}

void WiiAudio::BGM_Pitch(int pitch) {
	if (!bgm_decoder)
		return;

	bgm_decoder->SetPitch(pitch);
	// do in hardware?
	//AESND_SetVoiceFrequency(bgm_voice, pitch / 100 * DSP_DEFAULT_FREQ);
}

void WiiAudio::SE_Play(std::unique_ptr<AudioSeCache> se, int volume, int pitch) {
	if (!dsp_inited)
		return;

	int se_channel;

	AESNDPB *this_voice = nullptr;
	for (int i = 0; i <= MAX_SES; i++)
		if (!se_voices[i].running) {
			this_voice = se_voices[i].pb;
			se_channel = i;
			se_voices[i].running = true;
			break;
		}

	if (!this_voice) {
		Output::Warning("Couldn't play SE {}: No free channel available", se->GetName());
		return;
	}

	auto dec = se->CreateSeDecoder();
	dec->SetPitch(pitch);

	int frequency;
	AudioDecoder::Format format, out_format;
	int channels;

	std::vector<uint8_t> dec_buf;
	std::vector<uint8_t>* out_buf = nullptr;
	AudioSeRef se_ref;

	dec->GetFormat(frequency, format, channels);
	// When the DSP supports the format and the audio is not pitched the raw
	// buffer can be used directly
	u32 aesnd_format = get_voice_format(format, channels, out_format);
	if (format == out_format && pitch == 100) {
		se_ref = se->GetSeData();
		out_buf = &se_ref->buffer;
	} else {
		// otherwise decode to temporary buffer
		dec->SetFormat(frequency, out_format, channels);
		dec_buf = dec->DecodeAll();
		out_buf = &dec_buf;
	}
	float vol = volume * 2.55f;
	AESND_SetVoiceVolume(this_voice, vol, vol);

	// Upload to DSP and start playing
	size_t bsize = out_buf->size();
	if (se_voices[se_channel].buf != nullptr)
			free(se_voices[se_channel].buf);
	se_voices[se_channel].buf = (u8*)memalign(32, bsize);
	memcpy(se_voices[se_channel].buf, out_buf->data(), bsize);
	DCFlushRange(se_voices[se_channel].buf, bsize);

	AESND_PlayVoice(this_voice, aesnd_format, se_voices[se_channel].buf, bsize,
		frequency, 0, false);
}

void WiiAudio::SE_Stop() {
	if (!dsp_inited)
		return;

	for (int i = 0; i < MAX_SES; ++i) {
		AESND_SetVoiceStop(se_voices[i].pb, true);
		se_voices[i].running = false;
		if (se_voices[i].buf != nullptr) {
			free(se_voices[i].buf);
			se_voices[i].buf = nullptr;
		}
	}
}

void WiiAudio::Update() {
	if (!bgm_decoder)
		return;

	auto t = Game_Clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(t - bgm_starttick);
	bgm_decoder->Update(us);
	bgm_starttick = t;
}

void bgm_voice_dsp_callback(AESNDPB *pb, u32 state) {
	if (stopaudio || state != VOICE_STATE_STREAM) return;

	if (!bgm_decoder) {
		AESND_SetVoiceStop(pb, true);
		return;
	}

	// play current buffer, switch buffers, then let thread decode more
	//LWP_MutexLock(bgm_mutex);
	AESND_SetVoiceBuffer(pb, bgm_buf[cur_bgm_buf], SNDBUFFERSIZE);
	cur_bgm_buf = (cur_bgm_buf + 1) % BGM_BUFS;
	LWP_ThreadSignal(audioqueue);
	//LWP_MutexUnlock(bgm_mutex);
}

void se_voice_dsp_callback(AESNDPB *pb, u32 state) {
	if (stopaudio || state != VOICE_STATE_STOPPED) return;

	// HACK: get voice number out of opaque struct
	u32 *pvoiceno = (u32 *)&(((char *)(pb))[80]);
	se_voices[*pvoiceno].running = false;
}

WiiAudio::WiiAudio() {
	int i;

	AESND_Init();
	dsp_inited = true;

	for (i = 0; i < MAX_SES; i++) {
		se_voices[i].pb = AESND_AllocateVoice(se_voice_dsp_callback);
		se_voices[i].running = false;
	}
	for (i = 0; i < BGM_BUFS; i++)
		bgm_buf[i] = memalign(32, SNDBUFFERSIZE);
	bgm_voice = AESND_AllocateVoice(bgm_voice_dsp_callback);

	LWP_MutexInit(&bgm_mutex, 0);
	LWP_InitQueue(&audioqueue);
	LWP_CreateThread(&athread, AudioThread, nullptr, astack, AUDIOSTACK, 67);
	stopaudio = false;
}

WiiAudio::~WiiAudio() {
	int i;

	if (!dsp_inited)
		return;

	// do not use AESND_Reset(), hangs the DSP
	AESND_Pause(true);

	stopaudio = true;
	LWP_ThreadSignal(audioqueue);
	LWP_JoinThread(athread, nullptr);
	LWP_CloseQueue(audioqueue);
	athread = LWP_THREAD_NULL;
	LWP_MutexDestroy(bgm_mutex);

	AESND_SetVoiceStop(bgm_voice, true);
	AESND_FreeVoice(bgm_voice);
	for (i = 0; i < BGM_BUFS; i++)
		free(bgm_buf[i]);
	for (i = 0; i < MAX_SES; i++) {
		AESND_SetVoiceStop(se_voices[i].pb, true);
		AESND_FreeVoice(se_voices[i].pb);
		if (se_voices[i].buf != nullptr)
			free(se_voices[i].buf);
	}
}

#endif
