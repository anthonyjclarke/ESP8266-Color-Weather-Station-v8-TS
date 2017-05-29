/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/
#include "DHT.h"
#include <simpleDSTadjust.h>

// DHT Settings
// Uncomment whatever type you're using!

// #define DHTPIN D2 // NodeMCU
#define DHTPIN D7 // Wemos D1R2 Mini (DHT not currently implemented here)

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321 (DHT not currently implemented here)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#if DHTTYPE == DHT22
#define DHTTEXT "DHT22"
#elif DHTTYPE == DHT21
#define DHTTEXT "DHT21"
#elif DHTTYPE == DHT11
#define DHTTEXT "DHT11"
#endif
char FormattedTemperature[10];
char FormattedHumidity[10];

// Double-reset detector settings (for on-demand configuration portal)...

// Number of seconds after reset during which a 
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

// Settings for the name of the OTA host and config portal password...
// HOSTNAME for OTA update
#define HOSTNAME "ESP8266-OTA-"

const char* configPortalPassword = "portal-pass";

// *** The following are default values for the configuration...
//      DST rules for US Eastern Time Zone (New York, Boston, Louisville)
// *** Updated to Sydney

int UTC_OFFSET = +10;
//dstRule StartRule = {"EDT", Second, Sun, Mar, 2, 3600}; // Eastern Daylight time = UTC/GMT -4 hours;
//dstRule EndRule = {"EST", First, Sun, Nov, 1, 0};       // Eastern Standard time = UTC/GMT -5 hour;
struct dstRule StartRule = {"AEDT", First, Sun, Oct, 2, 3600}; // Australia Eastern Daylight time = UTC/GMT +11 hours
struct dstRule EndRule = {"AEST", First, Sun, Apr, 2, 0};      // Australia Eastern Standard time = UTC/GMT +10 hour

// Uncomment for 24 Hour style clock
//#define STYLE_24HR

#define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

// Wunderground Settings (default)
boolean IS_METRIC = true;
char WUNDERGROUND_API_KEY[18] = "xxxxxx";
char WUNDERGROUND_LANGUAGE[4] = "EN";
char WUNDERGROUND_COUNTRY[4] = "AU";
char WUNDERGROUND_CITY[20] = "Sydney";
char TZ_CITY[30] = "Sydney";

//  Various time constants used for updating
const int UPDATE_INTERVAL_SECS = 20 * 60;                // Update current observations every 10 minutes
const int UPDATE_INTERVAL_FORECAST_SECS = 2 * 60 * 60;   // Update forecast every 2 hours
const int UPDATE_INTERVAL_ASTRO_SECS = 12 * 60 * 60;     // Update astronomy data 2 times daily
const int UPDATE_MID_PANEL_INTERVAL_SECS = 6;            // Update mid-panel every 6 seconds
const int UPDATE_BOT_PANEL_INTERVAL_SECS = 6;           // Update bottom-panel every 6 seconds

// flags changed in the ticker functions based on frequency above
bool readyForWeatherUpdate = false;
bool readyForForecastUpdate = false;
bool readyForAstronomyUpdate = false;
bool readyForMidPanelUpdate = false;
bool readyForBotPanelUpdate = false;

// For controlling rotating mid-panel
int midPanelNumber = 0;
int maxMidPanels = 3;

// For controlling rotating bottom-panel
int botPanelNumber = 0;
int maxBotPanels = 6;

// List, so that the downloader knows what to fetch
String wundergroundIcons [] = {"chanceflurries","chancerain","chancesleet","chancesnow","clear","cloudy","flurries","fog","hazy","mostlycloudy","mostlysunny","partlycloudy","partlysunny","rain","sleet","snow","sunny","tstorms","unknown"};

// Update for Thingspeak / Clarke Mods
//
// Pins for the ILI9341
// Pins for a WEMOS D1 R2 to ILI9341
// Pins for the ILI9341 using Squiz PCB : https://blog.squix.org/product/esp8266-wifi-color-display-kit-2-4

#define TFT_DC D2
#define TFT_CS D1
#define LED_PIN D8
#define TOUCH_CS D3
#define TOUCH_IRQ  D4

// Thingspeak Settings - Wifi Outside Sensor - Wifi Sensor 1 
// Modify to your channel 
// My Public Reference channel is here : https://thingspeak.com/channels/142888
//
const String THINGSPEAK_CHANNEL_ID = "xxxxxx";
const String THINGSPEAK_API_READ_KEY = "xxxxxx";

/***************************
 * End Settings
 **************************/
