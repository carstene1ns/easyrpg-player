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

#include <cstdlib>
#include <cstring>
#include "game_config.h"
#include "system.h"
#include "ui.h"

#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

#ifdef SUPPORT_AUDIO
#  include "audio.h"

AudioInterface& CustomUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	custom_ui_t* interface = nullptr;
}

CustomUi::CustomUi(long width, long height, const Game_Config& cfg) :
	BaseUi(cfg)
{
	SetIsFullscreen(true);

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;

	const DynamicFormat format(
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000,
		PF::NoAlpha);

	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	main_surface = Bitmap::Create(current_display_mode.width,
		current_display_mode.height,
		false,
		current_display_mode.bpp
	);

#ifdef SUPPORT_AUDIO
	if(interface && interface->audio) {
		audio_ = std::move(interface->audio);
	}
#endif
}

CustomUi::~CustomUi() {
#ifdef SUPPORT_AUDIO
	audio_.reset();
#endif
}

bool CustomUi::vChangeDisplaySurfaceResolution(int new_width, int new_height) {
	BitmapRef new_main_surface = Bitmap::Create(new_width, new_height, false, current_display_mode.bpp);
	if (!new_main_surface) {
		Output::Warning("ChangeDisplaySurfaceResolution Bitmap::Create failed");
		return false;
	}

	main_surface = new_main_surface;
	current_display_mode.width = new_width;
	current_display_mode.height = new_height;
	return true;
}

bool CustomUi::ProcessEvents() {
	if(!interface) return true;

	// Poll events and process them
	//interface->pollInput();

	return true;
}

void CustomUi::UpdateDisplay() {
	if(!interface) return;

	interface->UpdateDisplay(main_surface->pixels(), current_display_mode.width,
		current_display_mode.height, main_surface->pitch());
}

void CustomUi::SetTitle(const std::string &title) {
	if(!interface) return;

	interface->SetTitle(title);
}

void CustomUi::SetUserInterface(custom_ui_t *interf) {
	interface = interf;
}

void CustomUi::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("Custom");

#if 0
	cfg.vsync.SetOptionVisible(true);
	cfg.fullscreen.SetOptionVisible(true);
	cfg.fps_limit.SetOptionVisible(true);
	cfg.window_zoom.SetOptionVisible(true);
	cfg.scaling_mode.SetOptionVisible(true);
	cfg.stretch.SetOptionVisible(true);
	cfg.game_resolution.SetOptionVisible(true);

	cfg.vsync.Set(current_display_mode.vsync);
	cfg.window_zoom.Set(current_display_mode.zoom);
	cfg.fullscreen.Set(IsFullscreen());
#endif
}
