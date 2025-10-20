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

#ifndef EP_CUSTOM_UI_H
#define EP_CUSTOM_UI_H

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

struct AudioInterface;

using custom_update_display_func = void (*) (void *data, int width, int height, int pitch);
using custom_set_title_func = void (*) (const std::string& title);
struct custom_ui_t {
	custom_update_display_func UpdateDisplay;
	custom_set_title_func SetTitle;

#ifdef SUPPORT_AUDIO
	std::unique_ptr<AudioInterface> audio;
#endif
};

/**
 * Custom UI class.
 */
class CustomUi final : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param cfg config options
	 */
	CustomUi(long width, long height, const Game_Config& cfg);

	/**
	 * Destructor.
	 */
	~CustomUi() override;

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */
	bool vChangeDisplaySurfaceResolution(int new_width, int new_height) override;
	void UpdateDisplay() override;
	void SetTitle(const std::string &title) override;
	bool ProcessEvents() override;
	void vGetConfig(Game_ConfigVideo& cfg) const override;

#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio() override;
#endif
	/** @} */

	static void SetUserInterface(custom_ui_t *interf);

private:

#ifdef SUPPORT_AUDIO
	std::unique_ptr<AudioInterface> audio_;
#endif
};

#endif
