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

#include <stdint.h>
#include <time.h>


// Calculates the even parity of an uint8_t array formatted following DCF77 format defined in dcf77_encode_data
// begin: first element of array
// end  : first-past-the-last element of the array
uint8_t dcf77_even_parity(uint8_t const* begin, uint8_t const* end)
{
	int parity = 1;
	
	for (uint8_t const* p = begin; p != end; ++p)
		if ((*p) == 1) // If the bit is set, swap the parity
			parity *= -1;
	
	return (parity == -1) ? 1 : 0;
}


// Encode local time into the format used by the DCF77 protocol 
// There is a 1:1 encoding between the bit state expressed as an unsigned and the length of the OFF encoding, in units of 100ms
//  0:  100ms OFF keying, 900ms ON keying, meaning a 0 (reset bit)
//  1:  200ms OFF keying, 800ms ON keying, meaning a 1 (set bit)
void dcf77_encode_data(struct tm* local_time, uint8_t* dcf77_one_minute_data)
{
 
	// Compute next minute (payload contains next minute) and next hour (for summer time announcement)
	// localtime(const time_t *timer) ESP implementation seems to ignore time_t argument and always return the same static tm struct containing local_time.
	// Use localtime_r(const time_t *timer, struct tm *buf) instead in order to make sure each tm struct is allocated separately with desired time value.
	time_t raw_time = mktime(local_time);
	time_t next_minute_time = raw_time + 60;
	time_t next_hour_time = raw_time + 3600;
	struct tm next_minute;
	struct tm next_hour;
	localtime_r(&next_minute_time, &next_minute);
	localtime_r(&next_hour_time, &next_hour);

	dcf77_one_minute_data[ 0] = 0; // Start of minute. Always reset

	dcf77_one_minute_data[ 1] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 2] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 3] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 4] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 5] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 6] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 7] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 8] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[ 9] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[10] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[11] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[12] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[13] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented
	dcf77_one_minute_data[14] = 0; // Third-party information content provided by BBK and Meteo Time. Not implemented

	dcf77_one_minute_data[15] = 0; // Alarm bit. 0 = normal operation, 1 = fault in transmitter operation. Not implemented

	// Summer time announcement
	// Emmited 59 times from 01:00:16 h CET (02:00:16 h CEST) until 01:59:16 h CET (02:59:16 h CEST)
	// current next  curr.   nexth  Announcement
	// time    hour  DST?    DST?   Flag must be set?
	// --- from CET to CEST ---
	// 00:59   01:59  no     no     no
	// 01:00   03:00  no     yes    yes
	// ... (no changes) ...
	// 01:59   03:59  no     yes    yes
	// 03:00   04:00  yes    yes    no
	// --- from CEST to CET ---
	// 01:59   02:59  yes    yes    no
	// 02:00   01:00  yes    no     yes
	// ... (no changes) ...
	// 02:59   01:59  yes    no     yes
	// 01:00   02:00  no     no     no
	// Need to set the flag when local_time DST differs from next_hour DST, next_hour being calculated from local_time (not next_minute).
	dcf77_one_minute_data[16] = (local_time->tm_isdst != next_hour.tm_isdst) ? 1 : 0; // Summer time announcement. Set during hour before change.

	// Zone time bits Z1 and Z2 (second marks Nos. 17 and 18) show to which time system the time information transmitted after second mark 20 refers.
	// For the emission of CET, Z1 has the state zero and Z2 the state one. When CEST is being emitted, this is reversed.
	dcf77_one_minute_data[17] = next_minute.tm_isdst > 0 ? 1 : 0;  // Set when DST is in effect in next_minute (tm_isdst: 0=no >0=yes <0=unknown)
	dcf77_one_minute_data[18] = next_minute.tm_isdst > 0 ? 0 : 1;  // Set when DST is not in effect in next_minute (tm_isdst: 0=no >0=yes <0=unknown)

	// Announcement bit A2 (No. 19) adverts the imminent introduction of a leap second.
	// A2 is also emitted for one hour in state one before a leap second is inserted.
	// Before a leap second is inserted on the 1st of January (1st of July), A2 is therefore emitted sixty times
	// from 00:00:19 h CET (01:00:19 h CEST) until 00:59:19 h CET (01:59:19 h CEST) in state one.
	// Not implemented. Leap second info is not available in struct tm. Receiver will eventually correct the unannounced leap second on subsequent sync.
	dcf77_one_minute_data[19] = 0; // Leap second announcement. Set during hour before leap second. Not implemented

	dcf77_one_minute_data[20] = 1; // Start of encoded time. Always set

	uint8_t const minute = next_minute.tm_min;
	uint8_t const minute_mod_10 = minute % 10;
	uint8_t const minute_div_10 = minute / 10;

	dcf77_one_minute_data[21] = ( (minute_mod_10 & 0b00000001) != 0 ); // First cypher of minute. bit 0
	dcf77_one_minute_data[22] = ( (minute_mod_10 & 0b00000010) != 0 ); // First cypher of minute. bit 1
	dcf77_one_minute_data[23] = ( (minute_mod_10 & 0b00000100) != 0 ); // First cypher of minute. bit 2
	dcf77_one_minute_data[24] = ( (minute_mod_10 & 0b00001000) != 0 ); // First cypher of minute. bit 3
	dcf77_one_minute_data[25] = ( (minute_div_10 & 0b00000001) != 0 ); // Second cypher of minute. bit 0
	dcf77_one_minute_data[26] = ( (minute_div_10 & 0b00000010) != 0 ); // Second cypher of minute. bit 1
	dcf77_one_minute_data[27] = ( (minute_div_10 & 0b00000100) != 0 ); // Second cypher of minute. bit 2
	dcf77_one_minute_data[28] = dcf77_even_parity(dcf77_one_minute_data + 21, dcf77_one_minute_data + 28); // minute parity

	uint8_t const hour = next_minute.tm_hour;
	uint8_t const hour_mod_10 = hour % 10;
	uint8_t const hour_div_10 = hour / 10;

	dcf77_one_minute_data[29] = ( (hour_mod_10 & 0b00000001) != 0 ); // First decimal cypher of hour. bit 0
	dcf77_one_minute_data[30] = ( (hour_mod_10 & 0b00000010) != 0 ); // First decimal cypher of hour. bit 1
	dcf77_one_minute_data[31] = ( (hour_mod_10 & 0b00000100) != 0 ); // First decimal cypher of hour. bit 2
	dcf77_one_minute_data[32] = ( (hour_mod_10 & 0b00001000) != 0 ); // First decimal cypher of hour. bit 3
	dcf77_one_minute_data[33] = ( (hour_div_10 & 0b00000001) != 0 ); // Second decimal cypher of hour. bit 0
	dcf77_one_minute_data[34] = ( (hour_div_10 & 0b00000010) != 0 ); // Second decimal cypher of hour. bit 1
	dcf77_one_minute_data[35] = dcf77_even_parity(dcf77_one_minute_data + 29, dcf77_one_minute_data + 35);  // hour parity

	uint8_t const day_month = next_minute.tm_mday;
	uint8_t const day_month_mod_10 = day_month % 10;
	uint8_t const day_month_div_10 = day_month / 10;

	dcf77_one_minute_data[36] = ( (day_month_mod_10 & 0b00000001) != 0 ); // First decimal cypher of day of the month. bit 0
	dcf77_one_minute_data[37] = ( (day_month_mod_10 & 0b00000010) != 0 ); // First decimal cypher of day of the month. bit 1
	dcf77_one_minute_data[38] = ( (day_month_mod_10 & 0b00000100) != 0 ); // First decimal cypher of day of the month. bit 2
	dcf77_one_minute_data[39] = ( (day_month_mod_10 & 0b00001000) != 0 ); // First decimal cypher of day of the month. bit 3
	dcf77_one_minute_data[40] = ( (day_month_div_10 & 0b00000001) != 0 ); // Second decimal cypher of day of the month. bit 0
	dcf77_one_minute_data[41] = ( (day_month_div_10 & 0b00000010) != 0 ); // Second decimal cypher of day of the month. bit 1

	uint8_t day_week = next_minute.tm_wday; // This variable counts from 0 (Sunday) to 6 (Saturday)
	if (day_week == 0) // DCF77 counts from 1 (Monday) to 7 (Sunday): Change Sunday's value from 0 to 7
		day_week = 7;

	dcf77_one_minute_data[42] = ( (day_week & 0b00000001) != 0 ); // Day of the week. Monday = 1, Sunday = 7
	dcf77_one_minute_data[43] = ( (day_week & 0b00000010) != 0 ); // Day of the week. Monday = 1, Sunday = 7
	dcf77_one_minute_data[44] = ( (day_week & 0b00000100) != 0 ); // Day of the week. Monday = 1, Sunday = 7

	uint8_t const month = 1 + next_minute.tm_mon;
	uint8_t const month_mod_10 = month % 10;
	uint8_t const month_div_10 = month / 10;

	dcf77_one_minute_data[45] = ( (month_mod_10 & 0b00000001) != 0 ); // First decimal cypher of month number. bit 0
	dcf77_one_minute_data[46] = ( (month_mod_10 & 0b00000010) != 0 ); // First decimal cypher of month number. bit 1
	dcf77_one_minute_data[47] = ( (month_mod_10 & 0b00000100) != 0 ); // First decimal cypher of month number. bit 2
	dcf77_one_minute_data[48] = ( (month_mod_10 & 0b00001000) != 0 ); // First decimal cypher of month number. bit 3
	dcf77_one_minute_data[49] = ( (month_div_10 & 0b00000001) != 0 ); // Second decimal cypher of month number. bit 0

	uint8_t const year_mod_100 = next_minute.tm_year % 100;
	uint8_t const year_mod_10 = year_mod_100 % 10;
	uint8_t const year_div_10 = year_mod_100 / 10;

	dcf77_one_minute_data[50] = ( (year_mod_10 & 0b00000001) != 0 ); // First cypher of year. bit 0
	dcf77_one_minute_data[51] = ( (year_mod_10 & 0b00000010) != 0 ); // First cypher of year. bit 1
	dcf77_one_minute_data[52] = ( (year_mod_10 & 0b00000100) != 0 ); // First cypher of year. bit 2
	dcf77_one_minute_data[53] = ( (year_mod_10 & 0b00001000) != 0 ); // First cypher of year. bit 3
	dcf77_one_minute_data[54] = ( (year_div_10 & 0b00000001) != 0 ); // Second cypher of year. bit 0
	dcf77_one_minute_data[55] = ( (year_div_10 & 0b00000010) != 0 ); // Second cypher of year. bit 1
	dcf77_one_minute_data[56] = ( (year_div_10 & 0b00000100) != 0 ); // Second cypher of year. bit 2
	dcf77_one_minute_data[57] = ( (year_div_10 & 0b00001000) != 0 ); // Second cypher of year. bit 3
	dcf77_one_minute_data[58] = dcf77_even_parity(dcf77_one_minute_data + 36, dcf77_one_minute_data + 58); // date parity
}
