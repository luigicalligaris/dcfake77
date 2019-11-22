#include <WiFi.h>
#include "time.h"
#include "dcf77protocol.h"
// #include "dcf77protocol.c"

// const char* wifi_ssid   = "SET_YOUR_SSID";
// const char* wifi_pass   = "SET_YOUR_PASS";

const char* ntp_server          = "pool.ntp.org";
const long  offset_gmt_sec      = -3 * 3600;
const int   offset_daylight_sec = -3 * 3600;

const unsigned led_pwm_freq       = 77490;
const unsigned led_pwm_channel    =     0;
const unsigned led_pwm_pin        =    16;
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

void WaitNextSec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	delayMicroseconds(1000000-tv.tv_usec);
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
	ledcSetup(led_pwm_channel, led_pwm_freq, led_pwm_resolution);
	
	// Attach the channel to the GPIO to be controlled
	ledcAttachPin(led_pwm_pin, led_pwm_channel);
	
	// Initialize the vector holding the values for the DCF77 encoding
	if(!getLocalTime(&local_time))
		Serial.println("Failed to obtain time");
	dcf77_encode_data(&local_time, dcf77_one_minute_data);
}

void loop()
{
	WaitNextSec();
	
	if(!getLocalTime(&local_time))
		Serial.println("Failed to obtain time");
	
	if (local_time.tm_sec > 58)
	{
		ledcWrite(led_pwm_channel, led_pwm_duty_on);
	}
	else
	{
		if (dcf77_one_minute_data[local_time.tm_sec] == 0)
		{
			ledcWrite(led_pwm_channel, led_pwm_duty_off);
			delayMicroseconds(100000); //100 ms = 0
			ledcWrite(led_pwm_channel, led_pwm_duty_on);
		}
		else
		{
			ledcWrite(led_pwm_channel, led_pwm_duty_off);
			delayMicroseconds(200000); //200 ms = 1
			ledcWrite(led_pwm_channel, led_pwm_duty_on);
		}
			
	}
	
	if (local_time.tm_sec == 0)
		dcf77_encode_data(&local_time, dcf77_one_minute_data);
	
	PrintLocalTime();
}
