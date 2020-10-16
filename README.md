# Migraine Weather Tracker

__Overview:__ Track migraine weather triggers including low barometric pressure and high cloud cover via IoT. Written in C++ (with Arduino libraries) for use with Feather HUZZAH ESP8266, MPL115A sensor, and a tri-color LED. Uses RESTful API calls.

## Purpose

I created an ambient migraine tracker that incorporates atmospheric pressure info from the MPL115A2 temperature + pressure sensor connected to an ESP8266 with cloud cover percentage information gathered from the OpenWeatherMap API. I combined the collected information to create an informative ambient light display to warn me if migraine weather conditions were present.
<br /><br />

## What this Setup Does

Depending on the information gathered, an RGB LED lights up with one of four colors to indicate the status of the two types of migraine triggers measured:
*	**Green:** If pressure is above or equal to 100.7 kPa (below this pressure level commonly causes migraines) and is less than 85% cloudy (high levels of cloudiness also commonly triggers migraines). I.e., pressure is ok and not too cloudy!
*	**Yellow:** If pressure is above or equal to 100.7 kPa and is greater than or equal to 85% cloudy. I.e., pressure is ok, but pretty cloudy.
*	**Purple:** If pressure is below 100.7 kPa and is less than 85% cloudy. I.e., low pressure, but not too cloudy.
*	**Red:** If pressure is below 100.7 kPa and is greater than or equal to 85% cloudy. I.e., low pressure AND cloudy. Possible migraine conditions!

This information is also reported to a MQTT feed (topic/feed; if you're interested in MQTT functionality, change this to your own topic and feed name(s)). The setup can be battery-powered (as pictured below) for placement indoors/outdoors (you may want to place the setup in an enclosure if you're exposing the hardware to the elements) or it can stay connected to a more permanent power source, such as a computer.
<br /><br />

## Hardware Setup

For reference, this is the breadboard hardware setup corresponding to the C++ code: 

![Hardware Setup](https://raw.githubusercontent.com/michelle-k-lee/MigraineWeatherTracker/master/MigraineTracker_Breadboard.png)

If you're looking to newly purchase these components, they likely can be found on [Adafruit](https://www.adafruit.com) or [DigiKey](https://www.digikey.com).
<br /><br />

## Software Setup

### For the .ino File:
Please be sure to download and install any Arduino libraries that you don't already have installed. The libraries are listed at the top of the code file. Libraries start with "#include" followed by the name of the library in <>. For example, `#include <ESP8266WiFi.h>`. For information on how to download and install Arduino libraries, see here: [Arduino Libraries](https://www.arduino.cc/en/Reference/Libraries)

Additionally, make sure that if you want to make use of the Arduino IDE's serial monitor, match the baud of your serial monitor settings to that in the code (115200).

Make sure you replace the placeholder text with your own OpenWeather API key and location information here:
`weatherClient.begin("http://api.openweathermap.org/data/2.5/weather?zip=yourZipgoeshere,US&appid=yourAPIkeygoeshere");`

For example, if your zip code was 12345 and your API key was 9876Lotsofdata, you would change the above to: 
`weatherClient.begin("http://api.openweathermap.org/data/2.5/weather?zip=12345,US&appid=9876Lotsofdata");`

For more specific details, have a look at the commented code!

### For the config.h File:
Be sure to add your own WiFi credentials and MQTT server credentials in the config.h file to replace placeholders!
For example, if your WiFi network name was FastestWiFi, you would change `#define wifi_ssid "YourWiFiNetwork"` to `#define wifi_ssid "FastestWiFi"`.

### OpenWeather API Documentation
Official OpenWeatherAPI documentation can be found here if you want to incorporate additional weather information or just want to learn more: 

* API starting point; you'll need to sign up and get an API key from OpenWeather to be able to call data: [How to Start](https://openweathermap.org/appid)
* Current weather API documentation: [Current Weather Data](https://openweathermap.org/current)
* Weather condition classification specifics: [Weather Conditions](https://openweathermap.org/weather-conditions)
<br /><br />

## Information on weather-related migraine triggers

### Information on cloud coverage as a migraine and/or headache trigger:

Information on how cloudiness can trigger headaches (it’s also a personal migraine trigger) gathered from this UK NHS website article: [“10 Headache Triggers”](https://www.nhs.uk/conditions/headaches/10-headache-triggers/).

Key information from the above article:
>“If you're prone to getting headaches, you could find that grey skies, high humidity, rising temperatures and storms can all bring on head pain.”

I set the cloud cover percentage threshold to greater than or equal to 85%, as 85% or higher cloud coverage is defined as "overcast clouds" by OpenWeather, but if your tolerance for high cloud cover is higher, you can adjust this. For further reference, the US National Weather Service defines 70-87% cloud coverage as "Mostly Cloudy/Considerable Cloudiness" and 88-100% cloud coverage as "Cloudy/Overcast" according to this document: [Anatomy of a Forecast](https://www.weather.gov/media/pah/ServiceGuide/A-forecast.pdf).

### Information on low barometric pressure as a migraine trigger:

The pressure level threshold was determined based on findings from this article found on the US National Library of Medicine National Institutes of Health website (https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4684554/).

Key information from this article:
>“It was found that the atmospheric pressure when the patients developed a migraine was within 1003–1007 hPa in the approach of low atmospheric pressure.”

Full citation: 
Okuma, H., Okuma, Y., & Kitagawa, Y. (2015). Examination of fluctuations in atmospheric pressure related to migraine. SpringerPlus, 4, 790. https://doi.org/10.1186/s40064-015-1592-4
