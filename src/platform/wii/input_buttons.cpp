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

// Headers
#include "input_buttons.h"
#include "keys.h"
#include "buttons.h"

Input::ButtonMappingArray Input::GetDefaultButtonMappings() {
	return {
		// Wiimote (turned sideways)
		{DOWN, Keys::WIIMOTE_LEFT},
		{LEFT, Keys::WIIMOTE_UP},
		{RIGHT, Keys::WIIMOTE_DOWN},
		{UP, Keys::WIIMOTE_RIGHT},
		{TOGGLE_FPS, Keys::WIIMOTE_A},
		{SHIFT, Keys::WIIMOTE_B},
		{CANCEL, Keys::WIIMOTE_ONE},
		{DECISION, Keys::WIIMOTE_TWO},
		{FAST_FORWARD, Keys::WIIMOTE_MINUS},
		{FAST_FORWARD_PLUS, Keys::WIIMOTE_PLUS},
		{SETTINGS_MENU, Keys::WIIMOTE_HOME},

		// Primary Joystick of PADS / Nunchuck
		{LEFT, Keys::JOY_STICK_PRIMARY_LEFT},
		{RIGHT, Keys::JOY_STICK_PRIMARY_RIGHT},
		{DOWN, Keys::JOY_STICK_PRIMARY_DOWN},
		{UP, Keys::JOY_STICK_PRIMARY_UP},

		// Nunchuck
		{DECISION, Keys::NUNCHUCK_Z},
		{CANCEL, Keys::NUNCHUCK_C},

		// all PADS: Classic Controller/Pro, GC Controller, Wii U Pro
		{DOWN, Keys::WIIPAD_DOWN},
		{LEFT, Keys::WIIPAD_LEFT},
		{RIGHT, Keys::WIIPAD_RIGHT},
		{UP, Keys::WIIPAD_UP},
		{DECISION, Keys::WIIPAD_A},
		{CANCEL, Keys::WIIPAD_B},
		{SHIFT, Keys::WIIPAD_X},
		{N1, Keys::WIIPAD_Y},
		/* trigger
		{N3, Keys::WIIPAD_L},
		{N5, Keys::WIIPAD_R},
		*/
		{N2, Keys::WIIPAD_LTP},
		{N3, Keys::WIIPAD_LTF},
		{N4, Keys::WIIPAD_RTP},
		{N5, Keys::WIIPAD_RTF},
		{N9, Keys::WIIPAD_ZL},
		{TOGGLE_FPS, Keys::WIIPAD_ZR},
		{FAST_FORWARD, Keys::WIIPAD_MINUS},
		{FAST_FORWARD_PLUS, Keys::WIIPAD_PLUS},
		{SETTINGS_MENU, Keys::WIIPAD_HOME},
		{N1, Keys::JOY_STICK_SECONDARY_DOWN_LEFT},
		{N2, Keys::JOY_STICK_SECONDARY_DOWN},
		{N3, Keys::JOY_STICK_SECONDARY_DOWN_RIGHT},
		{N4, Keys::JOY_STICK_SECONDARY_LEFT},
		{N6, Keys::JOY_STICK_SECONDARY_RIGHT},
		{N7, Keys::JOY_STICK_SECONDARY_UP_LEFT},
		{N8, Keys::JOY_STICK_SECONDARY_UP},
		{N9, Keys::JOY_STICK_SECONDARY_UP_RIGHT},
	};
}

Input::DirectionMappingArray Input::GetDefaultDirectionMappings() {
	return {
		{ Direction::DOWN, DOWN },
		{ Direction::LEFT, LEFT },
		{ Direction::RIGHT, RIGHT },
		{ Direction::UP, UP },
	};
}

Input::KeyNamesArray Input::GetInputKeyNames() {
	return {
		{Keys::WIIMOTE_LEFT, "Wiimote D-PAD Left"},
		{Keys::WIIMOTE_UP, "Wiimote D-PAD Up"},
		{Keys::WIIMOTE_DOWN, "Wiimote D-PAD Down"},
		{Keys::WIIMOTE_RIGHT, "Wiimote D-PAD Right"},
		{Keys::WIIMOTE_A, "Wiimote A"},
		{Keys::WIIMOTE_B, "Wiimote B"},
		{Keys::WIIMOTE_ONE, "Wiimote 1"},
		{Keys::WIIMOTE_TWO, "Wiimote 2"},
		{Keys::WIIMOTE_MINUS, "Wiimote -"},
		{Keys::WIIMOTE_PLUS, "Wiimote +"},
		{Keys::WIIMOTE_HOME, "Wiimote Home"},

		{Keys::NUNCHUCK_Z, "Nunchuck Z"},
		{Keys::NUNCHUCK_C, "Nunchuck C"},

		{Keys::WIIPAD_DOWN, "Pad Down"},
		{Keys::WIIPAD_LEFT, "Pad Left"},
		{Keys::WIIPAD_RIGHT, "Pad Right"},
		{Keys::WIIPAD_UP, "Pad Up"},
		{Keys::WIIPAD_A, "Pad A"},
		{Keys::WIIPAD_B, "Pad B"},
		{Keys::WIIPAD_X, "Pad X"},
		{Keys::WIIPAD_Y, "Pad Y"},
		/* trigger
		{Keys::WIIPAD_L, "Pad L"},
		{Keys::WIIPAD_R, "Pad R"},
		*/
		{Keys::WIIPAD_LTP, "Pad L Trigger"},
		{Keys::WIIPAD_RTP, "Pad R Trigger"},
		{Keys::WIIPAD_LTF, "Pad L Trigger (full)"},
		{Keys::WIIPAD_RTF, "Pad R Trigger (full)"},
		{Keys::WIIPAD_ZL, "Pad ZL"},
		{Keys::WIIPAD_ZR, "Pad ZR"},
		{Keys::WIIPAD_MINUS, "Pad -"},
		{Keys::WIIPAD_PLUS, "Pad +"},
		{Keys::WIIPAD_HOME, "Pad Home"},
	};
}
