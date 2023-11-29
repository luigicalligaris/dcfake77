/* DCF77 Protocol Encoding library */
// Copyright (C) 2018-2023  Luigi Calligaris
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef DCF77_PROTOCOL_H
#define DCF77_PROTOCOL_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t dcf77_even_parity(uint8_t const* begin, uint8_t const* end);
void    dcf77_encode_data(struct tm* local_time, uint8_t* dcf77_one_minute_data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //DCF77_PROTOCOL_H
