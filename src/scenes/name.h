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

#ifndef EP_SCENE_NAME_H
#define EP_SCENE_NAME_H

#include "scene.h"
#include "windows/name.h"
#include "windows/face.h"
#include "windows/keyboard.h"

/**
 * Scene_Item class.
 */
class Scene_Name : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Name(Game_Actor& actor, int charset, bool use_default_name);

	void Start() override;
	void vUpdate() override;

protected:
	std::vector<Window_Keyboard::Mode> layouts;
	int layout_index = 0;
	bool use_default_name = false;

private:
	Game_Actor& actor;

	std::unique_ptr<Window_Keyboard> kbd_window;
	std::unique_ptr<Window_Name> name_window;
	std::unique_ptr<Window_Face> face_window;
};

#endif
