/**
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
#include "ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include "game_clock.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#if DEBUG_GDB
#include <debug.h>
#endif
#include <wiiuse/wpad.h>
#include <sys/iosupport.h>
#include "main.h"
#include "buttons.h"

using namespace std::chrono_literals;

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& WiiUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	u8 *main_buffer = nullptr;
	bool fullscreen = false;

	GXTexObj texObj;
	uint32_t __attribute__((aligned(16))) fifo[256 * 1024];
	uint8_t *texture;

	// Get analog stick value
	s8 WPAD_Stick(struct expansion_t exp, int axis)	{
		struct joystick_t* js;

		if (exp.type == WPAD_EXP_NUNCHUK) {
			js = &exp.nunchuk.js;
			// only one stick
			if (axis > 1) return 0;
		} else if (exp.type == WPAD_EXP_CLASSIC) {
			if (axis < 2) {
				// left stick
				js = &exp.classic.ljs;
			} else {
				// right stick
				js = &exp.classic.rjs;
				axis -= 2;
			}
		} else return 0; // unsupported expansion

		int pos = axis ? js->pos.y : js->pos.x;
		int min = axis ? js->min.y : js->min.x;
		int max = axis ? js->max.y : js->max.x;
		int center = axis ? js->center.y : js->center.x;

		if (pos > max) return 127;
		if (pos < min) return -128;

		// clamp
		pos -= center;
		f32 base = 128.0;
		int half = center - min;
		if (pos > 0) {
			base = 127.0;
			half = max - center;
		}

		return (s8)(base * ((f32)pos / (f32)half));
	}
	// Get trigger value
	s8 WPAD_Trigger(struct expansion_t exp, int side)	{
		if (exp.type != WPAD_EXP_CLASSIC)
			return 0.f; // unsupported expansion

		return side ? exp.classic.r_shoulder : exp.classic.l_shoulder;
	}
}

static void Rumble(bool half = true) {
#ifndef DEBUG
	WPAD_Rumble(WPAD_CHAN_0, 1);
	PAD_ControlMotor(PAD_CHAN0, PAD_MOTOR_RUMBLE);
	Game_Clock::SleepFor(half ? 250ms : 500ms);
	WPAD_Rumble(WPAD_CHAN_0, 0);
	PAD_ControlMotor(PAD_CHAN0, PAD_MOTOR_STOP);
#endif
}

static void ShutdownCB() {
	Player::exit_flag = true;
}

static void ResetCB(u32, void *) {
#if DEBUG_GDB
	_break();
#else
	Player::reset_flag = true;
#endif
}

static void WPADShutdownCB(s32) {
	Player::exit_flag = true;
}

WiiUi::WiiUi(int width, int height, const Game_ConfigVideo& cfg) : BaseUi(cfg) {
	SetIsFullscreen(true);
	SYS_SetPowerCallback(ShutdownCB);
	SYS_SetResetCallback(ResetCB);

	// init Input
	PAD_Init();
	WPAD_Init();
	WPAD_SetPowerButtonCallback(WPADShutdownCB);
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS);
	WPAD_SetIdleTimeout(120);
	//WiiDRC_Init();

	main_buffer = (u8*)memalign(32, width * height * 4);
	if (!main_buffer) {
		Output::Error("Failed to create screen buffer!");
	}
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	const DynamicFormat format(32,
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, PF::Alpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(main_buffer, width, height, width * 4, format);

	// init GX
	memset(fifo, 0, sizeof(fifo));
	GX_Init(fifo, sizeof(fifo));

	GX_SetCopyClear((GXColor){ 0, 0, 0, 0xFF }, GX_MAX_Z24);

	GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
	int xfbHeight = GX_SetDispCopyYScale(GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight));
	GX_SetClipMode(GX_CLIP_ENABLE);
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	GX_SetDispCopyGamma(GX_GM_1_0);
	GX_SetPixelFmt(vmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetColorUpdate(GX_ENABLE);

	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);

	Mtx44 m;
	guOrtho(m, 0, vmode->efbHeight, 0, vmode->fbWidth, 0, 1);
	GX_LoadProjectionMtx(m, GX_ORTHOGRAPHIC);

	texture = (uint8_t *)memalign(32, height * width * 4);
	GX_InitTexObj(&texObj, texture, width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_InitTexObjFilterMode(&texObj, GX_NEAR, GX_NEAR);

	// aspect ratio?
	// Eliminate overscan / add ~5% borders

#ifdef SUPPORT_AUDIO
	audio_.reset(new WiiAudio());
#endif
	Rumble();
}

WiiUi::~WiiUi() {
	Rumble();

	// deinit GX
	GX_SetClipMode(GX_CLIP_DISABLE);
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_DrawDone();
	GX_AbortFrame();

	// deinit input
	WPAD_Disconnect(WPAD_CHAN_ALL);
	WPAD_Shutdown();

	if (main_buffer)
		free(main_buffer);
	if (texture)
		free(texture);
}

void WiiUi::ProcessEvents() {
	u32 input;
	struct expansion_t exp;

	keys.reset();

	// helper
	auto check_key = [&](int key, int button) {
		if (keys[key]) return;
		keys[key] = (input & button);
	};
	auto check_stick = [](int wii, int gc) {
		int value = 0;
		constexpr int threshold = 10; // compensate stick drift

		// Wii Controller takes precedence
		if (gc > threshold || gc < -threshold)
			value = gc;

		if (wii > threshold || wii < -threshold)
			value = wii;

		// normalize
		return static_cast<float>(value) / 128.f;
	};
	auto check_trigger = [](float wii, int gc) {
		constexpr float threshold = 0.1; // compensate drift

		// normalize
		float gcf = static_cast<float>(gc) / 255.f;

		// Wii Controller takes precedence
		if (gcf > threshold || gcf < -threshold) return gcf;
		if (wii > threshold || wii < -threshold) return wii;

		return 0.f;
	};

	// Wiimote 0
	WPAD_ScanPads();
	input = WPAD_ButtonsHeld(WPAD_CHAN_0);

	// Standard Wiimote
	check_key(Input::Keys::WIIMOTE_UP, WPAD_BUTTON_UP);
	check_key(Input::Keys::WIIMOTE_DOWN, WPAD_BUTTON_DOWN);
	check_key(Input::Keys::WIIMOTE_LEFT, WPAD_BUTTON_LEFT);
	check_key(Input::Keys::WIIMOTE_RIGHT, WPAD_BUTTON_RIGHT);
	check_key(Input::Keys::WIIMOTE_A, WPAD_BUTTON_A);
	check_key(Input::Keys::WIIMOTE_B, WPAD_BUTTON_B);
	check_key(Input::Keys::WIIMOTE_ONE, WPAD_BUTTON_1);
	check_key(Input::Keys::WIIMOTE_TWO, WPAD_BUTTON_2);
	check_key(Input::Keys::WIIMOTE_MINUS, WPAD_BUTTON_MINUS);
	check_key(Input::Keys::WIIMOTE_PLUS, WPAD_BUTTON_PLUS);
	check_key(Input::Keys::WIIMOTE_HOME, WPAD_BUTTON_HOME);

	// get attached expansion if any
	WPAD_Expansion(WPAD_CHAN_0, &exp);
	if (exp.type == WPAD_EXP_NUNCHUK) {
		// Nunchuck
		check_key(Input::Keys::NUNCHUCK_Z, WPAD_NUNCHUK_BUTTON_Z);
		check_key(Input::Keys::NUNCHUCK_C, WPAD_NUNCHUK_BUTTON_C);
	} else if (exp.type == WPAD_EXP_CLASSIC) {
		// Classic Controller, WiiU Pro controller, etc.
		check_key(Input::Keys::WIIPAD_UP, WPAD_CLASSIC_BUTTON_UP);
		check_key(Input::Keys::WIIPAD_DOWN, WPAD_CLASSIC_BUTTON_DOWN);
		check_key(Input::Keys::WIIPAD_LEFT, WPAD_CLASSIC_BUTTON_LEFT);
		check_key(Input::Keys::WIIPAD_RIGHT, WPAD_CLASSIC_BUTTON_RIGHT);
		check_key(Input::Keys::WIIPAD_A, WPAD_CLASSIC_BUTTON_A);
		check_key(Input::Keys::WIIPAD_B, WPAD_CLASSIC_BUTTON_B);
		check_key(Input::Keys::WIIPAD_X, WPAD_CLASSIC_BUTTON_X);
		check_key(Input::Keys::WIIPAD_Y, WPAD_CLASSIC_BUTTON_Y);
		/* checked with trigger
		check_key(Input::Keys::WIIPAD_L, WPAD_CLASSIC_BUTTON_FULL_L);
		check_key(Input::Keys::WIIPAD_R, WPAD_CLASSIC_BUTTON_FULL_R);
		*/
		check_key(Input::Keys::WIIPAD_ZL, WPAD_CLASSIC_BUTTON_ZL);
		check_key(Input::Keys::WIIPAD_ZR, WPAD_CLASSIC_BUTTON_ZR);
		check_key(Input::Keys::WIIPAD_MINUS, WPAD_CLASSIC_BUTTON_MINUS);
		check_key(Input::Keys::WIIPAD_PLUS, WPAD_CLASSIC_BUTTON_PLUS);
		check_key(Input::Keys::WIIPAD_HOME, WPAD_CLASSIC_BUTTON_HOME);
	}

	// GC controller 0
	PAD_ScanPads();
	input = PAD_ButtonsHeld(PAD_CHAN0);

	check_key(Input::Keys::WIIPAD_UP, PAD_BUTTON_UP);
	check_key(Input::Keys::WIIPAD_DOWN, PAD_BUTTON_DOWN);
	check_key(Input::Keys::WIIPAD_LEFT, PAD_BUTTON_LEFT);
	check_key(Input::Keys::WIIPAD_RIGHT, PAD_BUTTON_RIGHT);
	check_key(Input::Keys::WIIPAD_A, PAD_BUTTON_A);
	check_key(Input::Keys::WIIPAD_B, PAD_BUTTON_B);
	check_key(Input::Keys::WIIPAD_X, PAD_BUTTON_X);
	check_key(Input::Keys::WIIPAD_Y, PAD_BUTTON_Y);
	/* checked with trigger
	check_key(Input::Keys::WIIPAD_L, PAD_TRIGGER_L);
	check_key(Input::Keys::WIIPAD_R, PAD_TRIGGER_R);
	*/
	check_key(Input::Keys::WIIPAD_ZR, PAD_TRIGGER_Z);
	check_key(Input::Keys::WIIPAD_HOME, PAD_BUTTON_MENU);

	// Fullscreen mode support
	input = PAD_ButtonsDown(PAD_CHAN0) | WPAD_ButtonsDown(WPAD_CHAN_0) << 16;
	if ((input & WPAD_CLASSIC_BUTTON_FULL_R << 16) ||
		(input & PAD_TRIGGER_R)) {
		fullscreen = !fullscreen;

		char infos[64];
		extern u8 __Arena2Lo[];
		if ((u32)SYS_GetArena2Lo() > (u32)__Arena2Lo)
			sprintf(infos, "%d bytes free (MEM2)", (u32)SYS_GetArena2Size());
		else
			sprintf(infos, "%d bytes free (MEM1)", (u32)SYS_GetArena1Size());
		Output::Info("Memory: {}", infos);
	}

	// Analog support
	analog_input.primary.x = check_stick(WPAD_Stick(exp, 0), PAD_StickX(PAD_CHAN0));
	analog_input.primary.y = check_stick(WPAD_Stick(exp, 1), PAD_StickY(PAD_CHAN0));
	analog_input.secondary.x = check_stick(WPAD_Stick(exp, 2), PAD_SubStickX(PAD_CHAN0));
	analog_input.secondary.y = check_stick(WPAD_Stick(exp, 3), PAD_SubStickY(PAD_CHAN0));

	// Trigger support
	analog_input.trigger_left = check_trigger(WPAD_Trigger(exp, 0), PAD_TriggerL(PAD_CHAN0));
	analog_input.trigger_right = check_trigger(WPAD_Trigger(exp, 1), PAD_TriggerR(PAD_CHAN0));
}

void WiiUi::UpdateDisplay() {
	Mtx m;

	// copy to texture
	int w = main_surface->width();
	for (int y = 0; y < main_surface->height(); y++) {
		for (int x = 0; x < w; x++) {
			// get
			u8 *color = (u8 *)main_surface->pixels() + (w * 4 * y) + (x * 4);
			u16 ar = (color[3] << 8) | color[0];
			u16 gb = (color[1] << 8) | color[2];
			// set
			u32 offset = (((y&(~3)) << 2) * w) + ((x&(~3)) << 4) + ((((y&3) << 2) + (x&3)) << 1);
			*((u16*)(texture + offset)) = ar;
			*((u16*)(texture + offset + 32)) = gb;
		}
	}
	DCFlushRange(texture, sizeof(texture));
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	// update screen
	guMtxIdentity(m);
	guMtxTrans(m, 0, 0, 0);
	GX_LoadPosMtxImm(m, GX_PNMTX0);

	int overscan_x = fullscreen ? 0 : 16;
	int overscan_y = fullscreen ? 0 : 12;
	int wide = 0;
	int x1 = 0 + overscan_x + wide;
	int y1 = 0 + overscan_y;
	int x2 = x1 + main_surface->width()*2 - (overscan_x * 2) - (wide * 2);
	int y2 = y1 + main_surface->height()*2 - (overscan_y * 2);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3s16(x1, y1, 0);
		GX_Color1u32(0xFFFFFFFF);
		GX_TexCoord2f32(0, 0);

		GX_Position3s16(x2, y1, 0);
		GX_Color1u32(0xFFFFFFFF);
		GX_TexCoord2f32(1, 0);

		GX_Position3s16(x2, y2, 0);
		GX_Color1u32(0xFFFFFFFF);
		GX_TexCoord2f32(1, 1);

		GX_Position3s16(x1, y2, 0);
		GX_Color1u32(0xFFFFFFFF);
		GX_TexCoord2f32(0, 1);
	GX_End();

	GX_DrawDone();
	GX_InvalidateTexAll();

	fb ^= 1;
	GX_CopyDisp(xfb[fb], GX_TRUE);

	VIDEO_SetNextFramebuffer(xfb[fb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE) {
		VIDEO_WaitVSync();
	}
}

bool WiiUi::LogMessage(const std::string &message) {
	// send to usbgecko
	if (usb_isgeckoalive(CARD_SLOTB)) {
		Wii::LogPrint(message);
		return true;
	}

	// HLE in dolphin emulator
	std::string m = std::string("[" GAME_TITLE "] ") + message + "\n";
	puts(m.c_str());

	// additional console/usbgecko output not needed
	return true;
}
