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

#include <ogc/machine/asm.h>

	/* lowlevel handler */
	.section	.text.ll_exceptionhandler,"ax",@progbits
	.global	ll_exceptionhandler

ll_exceptionhandler:
	/* save registers */
	stw		%r11,GPR1_OFFSET(%sp)
	stmw	%r13,GPR13_OFFSET(%sp)
	mr		%r3,%sp
	/* call highlevel handler */
	b		hl_exceptionhandler
