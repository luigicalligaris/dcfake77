// dcfake77-esp32
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

#include <WiFi.h>
#include "time.h"
#include "dcf77protocol.h"
// #include "dcf77protocol.c"

// Set the SSID and password  below  and remove the comment marks
//const char* wifi_ssid   = "SET_YOUR_SSID";
//const char* wifi_pass   = "SET_YOUR_PASS";

const char* ntp_server          = "thrudr";
const long  offset_gmt_sec      = -4 * 3600;
const int   offset_daylight_sec = -3 * 3600;

const unsigned led_pwm_freq       = 77490;
const unsigned led_pwm_channel    =     0;
const unsigned led_pwm_pin        =    21;
const unsigned led_pwm_resolution =     2;
const unsigned led_pwm_duty_off   =     0;
const unsigned led_pwm_duty_on    =     2;

static struct tm  local_time;
static uint8_t    dcf77_one_minute_data[60];


void PrintLocalTime()
{
	if(!getLocalTime(&local_time))
	{
		Serial.println("Failed to obtain time");
		return;
	}
	
	Serial.println(&local_time, "%A, %B %d %Y %H:%M:%S");
}

void setup()
{
	Serial.begin(115200);
	
	//connect to WiFi
	Serial.printf("Connecting to %s ", wifi_ssid);
	WiFi.begin(wifi_ssid, wifi_pass);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println(" CONNECTED");
	
	//init and get the time
	while (1)
	{
		configTime(offset_gmt_sec, offset_daylight_sec, ntp_server);
		
		if (!getLocalTime(&local_time))
			Serial.println("Failed to obtain time");
		else
			break;
		
		delayMicroseconds(100000);
	}
	
	Serial.println("Initialized time");
	
	PrintLocalTime();

	// Disconnect WiFi as it's no longer needed
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	
	// Configure LED PWM functionalitites
	ledcSetClockSource(LEDC_USE_XTAL_CLK);
	//ledcSetup(led_pwm_channel, led_pwm_freq, led_pwm_resolution);
	// Attach the channel to the GPIO to be controlled
	//ledcAttachPin(led_pwm_pin, led_pwm_channel);
	// Since v3.0 ledcSetup and letcAttachPin are combined in one function
	ledcAttachChannel(led_pwm_pin, led_pwm_freq, led_pwm_resolution, led_pwm_channel);
  
	// Initialize the vector holding the values for the DCF77 encoding
	if(!getLocalTime(&local_time))
		Serial.println("Failed to obtain time");
	dcf77_encode_data(&local_time, dcf77_one_minute_data);
}

void loop()
{
  struct timeval tv;
  
  if(!getLocalTime(&local_time))
		Serial.println("Failed to obtain time");
	
	if (local_time.tm_sec > 58)
	{
		ledcWriteChannel(led_pwm_channel, led_pwm_duty_on);
	}
	else
	{
		if (dcf77_one_minute_data[local_time.tm_sec] == 0)
		{
			ledcWriteChannel(led_pwm_channel, led_pwm_duty_off);
			delayMicroseconds(100000); //100 ms = 0
			ledcWriteChannel(led_pwm_channel, led_pwm_duty_on);
		}
		else
		{
			ledcWriteChannel(led_pwm_channel, led_pwm_duty_off);
			delayMicroseconds(200000); //200 ms = 1
			ledcWriteChannel(led_pwm_channel, led_pwm_duty_on);
		}
			
	}
	
	if (local_time.tm_sec == 0)
		dcf77_encode_data(&local_time, dcf77_one_minute_data);
	
	PrintLocalTime();
	gettimeofday(&tv, NULL);
	delayMicroseconds(1000000-tv.tv_usec); 
}
