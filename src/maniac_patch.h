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


#ifndef EP_MANIAC_PATCH
#define EP_MANIAC_PATCH

#include <array>
#include <cstdint>
#include <vector>
#include "span.h"

#include "engine/strings.h"

class Game_BaseInterpreterContext;

namespace ManiacPatch {
	int32_t ParseExpression(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter);
	std::vector<int32_t> ParseExpressions(Span<const int32_t> op_codes, const Game_BaseInterpreterContext& interpreter);


	std::array<bool, 50> GetKeyRange();

	bool GetKeyState(uint32_t key_id);

	bool CheckString(std::string_view str_l, std::string_view str_r, int op, bool ignore_case);

	std::string_view GetLcfName(int data_type, int id, bool is_dynamic);
	std::string_view GetLcfDescription(int data_type, int id, bool is_dynamic);
}

#endif
