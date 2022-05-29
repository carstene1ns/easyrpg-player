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

#include <gccore.h>
#include <ogcsys.h>
#include <ogc/conf.h>
#include <ogc/machine/processor.h>
#if DEBUG_GDB
#include <debug.h>
#endif
#include <sys/iosupport.h>
#include <fat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <algorithm>
#include "player.h"
#include "baseui.h"
#include "main.h"
#include "ui.h"
#include "bsod.h"
#include "game_clock.h"

using namespace std::chrono_literals;

GXRModeObj *vmode;
void *xfb[2];
int fb;

int main(int argc, char* argv[]) {
	// activate 64-byte fetches for the L2 cache
	L2Enhance();

	// reload custom ios
	u32 version = IOS_GetVersion();
	s32 preferred = IOS_GetPreferredVersion();
	if(preferred > 0 && version != (u32)preferred)
		IOS_ReloadIOS(preferred);

	// setup video
	VIDEO_Init();
	VIDEO_SetBlack(true);
	vmode = VIDEO_GetPreferredMode(nullptr);

	// widescreen
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
		// Wii U letterbox
		if ((*(u32*)(0xCD8005A0) >> 16) == 0xCAFE) {
			write32(0xd8006a0, 0x30000004);
			mask32(0xd8006a8, 0, 2);
		}
		vmode->viWidth = 678; // 720
	} else {
		vmode->viWidth = 672;
	}
	vmode->viXOrigin = (720 - vmode->viWidth) / 2;
	VIDEO_Configure(vmode);
	// framebuffer
	for (int i = 0; i <= 1; i++) {
		xfb[i] = SYS_AllocateFramebuffer(vmode);
		DCInvalidateRange(xfb[i], VIDEO_GetFrameBufferSize(vmode));
		xfb[i] = MEM_K0_TO_K1(xfb[i]);
		VIDEO_ClearFrameBuffer(vmode, xfb[i], COLOR_BLACK);
	}
	fb = 0;
	VIDEO_SetNextFramebuffer(xfb[fb]);

	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();

#ifdef DEBUG_GDB
	DEBUG_Init(GDBSTUB_DEVICE_USB, CARD_SLOTB);
#else
	InitialiseBSOD();

	// Enable USBGecko output
	CON_EnableGecko(CARD_SLOTB, true);
#endif

	// cmdline
	std::vector<std::string> args(argv, argv + argc);

	// dolphin
	bool is_emu = argc == 0;
	if(is_emu) {
		// set arbitrary application path
		args.push_back("/easyrpg-player");
	}

	// Check if a game directory was provided
	if (std::none_of(args.cbegin(), args.cend(),
		[](const std::string& a) { return a == "--project-path"; })) {

		// Working directory not correctly handled, provide it manually
		char working_dir[256];
		getcwd(working_dir, 255);
		args.push_back("--project-path");
		args.push_back(working_dir);
	}

	int ret = EXIT_FAILURE;

	// Init libfat (Mount SD/USB)
	if (fatInitDefault()) {
		if (is_emu || !strchr(argv[0], '/')) {
			Wii::LogPrint("Debug: USBGecko/Dolphin mode, changing dir to default.");
			chdir("/apps/easyrpg");
		}

		// Run Player
		Player::Init(std::move(args));
		Player::Run();

		ret = EXIT_SUCCESS;
		Wii::LogPrint("Debug: Player shut down.");

		// deinit storage
		fatUnmount("sd");
		fatUnmount("usb");
	} else {
		// Cannot use Output::Error() here, too soon
		Wii::LogPrint("Error: Couldn't mount any storage medium (SD/USB - FAT32)!\n\n"
			"EasyRPG Player will close now.");
		Game_Clock::SleepFor(5s);
	}

	// deinit video
	VIDEO_SetBlack(true);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();
	for (int i = 0; i <= 1; i++) {
		free(MEM_K1_TO_K0(xfb[i]));
		xfb[i] = nullptr;
	}

	return(ret);
}


void Wii::LogPrint(std::string const& msg) {
	// poor mans colored log, cannot use rang as it cripples the message
	std::string lvl = msg.substr(0, 5);

	int color = 37; // white
	if (lvl == "Error") color = 31; // red
	else if (lvl == "Warni") color = 33; // orange
	else if (lvl == "Debug") color = 34; // blue

#ifdef NDEBUG
	// no console, no usbgecko = no need to print
	static bool has_usbgecko = usb_isgeckoalive(CARD_SLOTB);
	if (!has_usbgecko && lvl != "Error") return;
#endif

	// initialize console for error display on startup
	if(!DisplayUi && lvl == "Error") {
		CON_Init(VIDEO_GetNextFramebuffer(), 20, 20, vmode->fbWidth,
			vmode->xfbHeight, vmode->fbWidth * VI_DISPLAY_PIX_SZ);
		puts("\n\n");
	}

	printf("\x1B[%dm%s\x1B[0m\n", color, msg.c_str());
}
