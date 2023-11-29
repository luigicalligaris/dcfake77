// dcfake77-esp8266
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

#include <ESP8266WiFi.h>
#include "time.h"
#include "dcf77protocol.h"
// #include "dcf77protocol.c"

// const char* wifi_ssid   = "SET_YOUR_SSID";
// const char* wifi_pass   = "SET_YOUR_PASS";

//const char* ntp_server          = "pool.ntp.org";
const char* ntp_server          = "it.pool.ntp.org";
const long  offset_gmt_sec      = 2 * 3600;
const int   offset_daylight_sec = 0 * 3600;

//const unsigned pwm_freq       =  77500;
const unsigned pwm_freq       =  25833; // let's use the 3rd harmonic of this to generate 77.5 kHz
const unsigned pwm_pin        =      5;
const unsigned pwm_resolution =      3;
const unsigned pwm_duty_off   =      0;
const unsigned pwm_duty_on    =      3;

const unsigned led_pin        =      2;


static uint8_t   dcf77_one_minute_data[60];


void PrintLocalTime()
{
	time_t rawtime;
	time(&rawtime);
	
	Serial.print(ctime(&rawtime));
	Serial.print("\r");
}

void WaitNextSec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	delayMicroseconds(1000000-tv.tv_usec);
}

void setup()
{
	// Configure NTP server and offsets
	configTime(offset_gmt_sec, offset_daylight_sec, ntp_server);
	//configTime(time_zone, ntp_server);
	
	Serial.begin(115200);
	
	//connect to WiFi
	Serial.printf("Connecting to %s ", wifi_ssid);
	WiFi.begin(wifi_ssid, wifi_pass);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println(" CONNECTED");
	
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
  timeinfo = localtime(&rawtime);
	
	PrintLocalTime();

	// Disconnect WiFi as it's no longer needed
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	
	// Configure PWM functionalitites
	analogWriteFreq(pwm_freq);
	analogWriteRange(pwm_resolution);
	
	// Configure LED
	pinMode(led_pin, OUTPUT);
	
	// Initialize the vector holding the values for the DCF77 encoding
	dcf77_encode_data(timeinfo, dcf77_one_minute_data);
}

void loop()
{
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
  timeinfo = localtime (&rawtime);
	
	WaitNextSec();
	
	if (timeinfo->tm_sec > 58)
	{
		analogWrite(pwm_pin, pwm_duty_on);
	}
	else
	{
		if (dcf77_one_minute_data[timeinfo->tm_sec] == 0)
		{
			digitalWrite(led_pin, LOW);
			analogWrite(pwm_pin, pwm_duty_off);
			delayMicroseconds(100000); //100 ms = 0
			digitalWrite(led_pin, HIGH);
			analogWrite(pwm_pin, pwm_duty_on);
		}
		else
		{
			digitalWrite(led_pin, LOW);
			analogWrite(pwm_pin, pwm_duty_off);
			delayMicroseconds(200000); //200 ms = 1
			digitalWrite(led_pin, HIGH);
			analogWrite(pwm_pin, pwm_duty_on);
		}
			
	}
	
	if (timeinfo->tm_sec == 0)
		dcf77_encode_data(timeinfo, dcf77_one_minute_data);
	
	PrintLocalTime();
}
