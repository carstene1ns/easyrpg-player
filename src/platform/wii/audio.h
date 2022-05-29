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

#ifndef EP_PLATFORM_WII_AUDIO_H
#define EP_PLATFORM_WII_AUDIO_H

// TODO: make this relative with source restructure, e.g. "engine/audio.h"
#include_next "audio.h"

#include <ogcsys.h>
#include <aesndlib.h>
#include <memory>

#include "audio_decoder.h"
#include "game_clock.h"

class WiiAudio : public AudioInterface {
public:
	WiiAudio();
	~WiiAudio();

	void BGM_Play(Filesystem_Stream::InputStream stream, int volume, int pitch, int fadein) override;
	void BGM_Pause() override;
	void BGM_Resume() override;
	void BGM_Stop() override;
	bool BGM_PlayedOnce() const override;
	bool BGM_IsPlaying() const override;
	int BGM_GetTicks() const override;
	void BGM_Fade(int fade) override;
	void BGM_Volume(int volume) override;
	void BGM_Pitch(int pitch) override;
	void SE_Play(std::unique_ptr<AudioSeCache> se, int volume, int pitch) override;
	void SE_Stop() override;
	virtual void Update() override;
}; // class WiiAudio

#endif
