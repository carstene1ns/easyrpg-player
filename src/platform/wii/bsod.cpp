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
#include <ogc/lwp_threads.h>
#include <ogc/libversion.h>
#include <cstdio>
#include <unistd.h>
#include "version.h"
#include "main.h"

extern "C" {
	void ll_exceptionhandler();
	// part of libogc
	void __exception_sethandler(uint32_t, void (*)());
	void __exception_close(uint32_t);
	void __reload();
	void VIDEO_SetFramebuffer(void *fb);
}

static frame_context *_exception_ctx = nullptr;

// copied from libogc
static const char * const _exception_name[NUM_EXCEPTIONS] = {
	"System Reset", "Machine Check", "DSI", "ISI", "Interrupt", "Alignment",
	"Program", "Floating Point", "Decrementer", "System Call", "Trace",
	"Performance", "IABR", "Reserved", "Thermal"
};

static uint32_t _exceptions_to_handle[] = { EX_MACH_CHECK, EX_DSI, EX_ISI, EX_ALIGN, EX_PRG };

static void DisplayBSOD() {
	// disable old console, if any
	//VIDEO_SetPostRetraceCallback(nullptr);

	// init new console
	void *xfb = (void*)0xC1700000;//VIDEO_GetNextFramebuffer();
	VIDEO_SetFramebuffer(xfb);
	CON_Init(xfb, 20, 20, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * VI_DISPLAY_PIX_SZ);
	CON_EnableGecko(1, true);

	// paint blue background
	u32 *p = (u32*)xfb;
	for (int i = 0; i < vmode->xfbHeight * vmode->fbWidth / 2; i++)
		*p++ = 0x0DBE0D75;

	printf("\n\x1B[44m x x   EasyRPG Player %s just crashed!\n", Version::STRING);

	printf("  ^    (built on %s, using libOGC %d.%d.%d)\n", __DATE__, _V_MAJOR_, _V_MINOR_, _V_PATCH_);

	if (_exception_ctx) {
		printf("Crash reason:\n - Exception: %s in location: %.8X",
				_exception_name[_exception_ctx->EXCPT_Number], _exception_ctx->SRR0);

		printf("\n\nStack trace:\n");
		void **frame = (void **)_exception_ctx->GPR[1];
		for (int i = 0; frame != nullptr && i < 16; i++) {
			void *ip = frame[1];
			if (ip == nullptr) {
				printf("No more data.\n");
				break;
			}

			printf(" %.8X%s", (intptr_t)ip, i % 8 == 7 ? "\n" : " <-");

			void **next = (void **)frame[0];
			if (next <= frame || !(((intptr_t)(next) % PPC_ALIGNMENT) == 0)) {
				printf("No more data.\n");
				break;
			}
			frame = next;
		}

		printf("\n\nRegisters:\n");
		for (int i = 0; i < 32; i++)
			printf(" [GPR%02d: %.8X]%s", i, _exception_ctx->GPR[i], i % 4 == 3 ? "\n": " ");

		printf(" [PC:    %.8X] [MSR:   %.8X] [CR:    %.8X] [LR:    %.8X]\n[CTR:   %.8X] [XER:   %.8X]",
			_exception_ctx->SRR0, _exception_ctx->SRR1, _exception_ctx->CR,
			_exception_ctx->LR, _exception_ctx->CTR, _exception_ctx->XER);
	}

	printf("\n\nPress RESET to return to loader...\n");
	while (!SYS_ResetButtonDown())
		usleep(20);

	printf("Reloading!\x1B[0m\n");
	usleep(1000);

	VIDEO_SetBlack(true);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	__reload();
}

extern "C" void hl_exceptionhandler(frame_context *ctx) {
	// close exception
	for (uint32_t e : _exceptions_to_handle)
		__exception_close(e);

	// stop possible garbage data interfering
	AUDIO_StopDMA();
	GX_AbortFrame();

	// let the BSOD handle the rest
	_exception_ctx = ctx;
	__lwp_thread_stopmultitasking(DisplayBSOD);
}

void InitialiseBSOD() {
	for (uint32_t e : _exceptions_to_handle)
		__exception_sethandler(e, ll_exceptionhandler);
}
