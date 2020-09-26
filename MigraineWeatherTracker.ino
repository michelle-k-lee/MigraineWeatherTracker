/*
Track migraine weather triggers including low barometric pressure and high cloud cover.
Written in Arduino (C++) for use with ESP8266, MPL115A2 sensor, and a tri-color LED. Uses API calls.
Sends data via MQTT for lightweight IoT messaging with publish and subscribe functionality.
*/

#include <ESP8266WiFi.h>    // Loads libraries
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MPL115A2.h>

#include "config.h" // Credentials for WiFi, MQTT are in config.h file

#define MQTT_ENABLE 1  // Set to 1 to enable posting data

// Create objects
WiFiClient espClient;
PubSubClient mqtt(espClient);
Adafruit_MPL115A2 mpl115a2;
StaticJsonDocument<256> outputDoc;

typedef struct { // Create a struct to hold data
  String wm;     // For each name:value pair coming in from the service, create a slot in our structure to hold our data
  String wd;
  String ca;
  String na;
} WeatherData;     // This gives the data structure a name, WeatherData

WeatherData weather; // The WeatherData type has been created, but not an instance of that type, so this creates variable "weather" of type WeatherData

char mac[6]; // MAC address is a unique ID for each device; we can use part of that as our user ID
char buffer[256];   // Stores the data for the JSON document

unsigned long timer;  // A timer for the MQTT server; we don't want to flood it with data

float pressureKPA = 0; // Declare type: float name variable of pressureKPA, which stores data of float

int cloudiness = 0; // Declares integer variable cloudiness

int delayTime = 1000;  // Define delayTime to be 1 second

float migrainePressureLevel = 100.7; // Define variable migrainePressureLevel as float 100.7

int redPin = 14;
int greenPin = 12;
int bluePin = 13;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));

  mpl115a2.begin();

  if (MQTT_ENABLE) { // If MQTT posting is enabled
    setup_wifi(); // Connect to WiFi
    mqtt.setServer(mqtt_server, 1883); // Uses the MQTT server address defined in the config file, port 1883 (Note: not secure, sends in plain text)
    //mqtt.setCallback(callback); // Sets the callback function so we can receive MQTT messages
  }

  timer = millis(); // Starts timer

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  if (millis() - timer > 5000) { // If 5 seconds have passed since last message
    checkSensor();    // then check the sensor
    outputDoc["Pressure_kPa"] = pressureKPA; // Outputs "Pressure_kPa" and the pressure to the MQTT server
    outputDoc["Cloudiness_Percentage"] = cloudiness; // Outputs "Cloudiness_Percentage" and the cloud level to the MQTT server
    serializeJson(outputDoc, buffer);
    timer = millis(); // Resets 5-second timer

    if (MQTT_ENABLE) {  // If posting is turned on, post the data
      if (!mqtt.connected()) { // If not connected, then...
        reconnect(); // Reconnect
      }
      mqtt.publish(feed1, buffer);  // Posts data to feed1 (defined in the config file)
      Serial.println("Posted:"); // Prints "Posted:" and starts next printing on a new line
      serializeJsonPretty(outputDoc, Serial);
      Serial.println(); // Starts on new line of serial monitor

      mqtt.loop(); // Keeps the MQTT connection active
    }

    if (pressureKPA >= migrainePressureLevel && cloudiness < 85) {
      // If pressure is above or equal to 100.7 kPa (below this causes migraines according to research) and is less than 85% cloudy (high cloudiness also causes migraines),
      Serial.println("Green: Pressure is ok and it's not too cloudy!"); // Print in serial monitor and go to new line
      setColor(80, 0, 80);  // Sets LED color to green
      delay(delayTime); // Delay of 1 second
    }
    else if (pressureKPA >= migrainePressureLevel && cloudiness >= 85) {
      // If pressure is above or equal to 100.7 kPa and is more than or equal to 85% cloudy,
      Serial.println("Yellow: Pressure is ok, but it's very cloudy (greater than 85% cloudiness)!"); // Print in serial monitor and go to new line
      setColor(0, 0, 255);  // Sets LED color to yellow
      delay(delayTime); // Delay of 1 second
    }
    else if (pressureKPA < migrainePressureLevel && cloudiness < 85) {
      // If pressure is below 100.7 kPa and is less than 85% cloudy,
      Serial.println("Purple: Low pressure, but it's not too cloudy!"); // Print in serial monitor and go to new line
      setColor(0, 255, 0);  // Sets LED color to purple
      delay(delayTime); // Delay of 1 second
    }
    else if (pressureKPA < migrainePressureLevel && cloudiness >= 85) {
      // If pressure is below 100.7 kPa and is less than 85% cloudy,
      Serial.println("Red: Low pressure AND very cloudy. Keep your migraine remedies handy!"); // Print in serial monitor and go to new line
      setColor(0, 255, 255);  // Sets LED color to red
      delay(delayTime); // Delay of 1 second
    }
  }

  getWeather(); // Calls the getWeather function

  Serial.print("The date and time is: "); // Prints "The date and time is: " in serial monitor
  Serial.println(F(__DATE__ " " __TIME__)); // Prints the date and time and goes to new line
  Serial.println("Here's where we're at: " + weather.na); // Prints location in serial monitor and goes to new line
  Serial.println("The weather is: " + weather.wm + ". Here's more detail: " + weather.wd); // Prints "The weather is: [main weather]. Here's more detail: [description]" in serial monitor and goes to new line
  Serial.println("Cloud level is: " + weather.ca); // Prints "Cloud level is: [cloud level]" in serial monitor and goes to new line

  delay(5000); // 5 second delay between requests
}

void setColor(int red, int green, int blue) { // Function for setting the color value for each pin of the RGB LED
  digitalWrite(redPin, red);
  digitalWrite(greenPin, green);
  digitalWrite(bluePin, blue);
}

void checkSensor() { // Defines function checkSensor for checking barometric pressure via MPL115A2
  Serial.println("------------------------------"); // Prints "------------------------------" and starts next printing on a new line
  pressureKPA = mpl115a2.getPressure(); // Variable pressureKPA has value of the pressure reading from the sensor
  Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.println(" kPa");
  // Prints "Pressure (kPa): " + [the pressure reading value] + " kPa" in the serial monitor
}

void getWeather() { // Defines function getWeather for making API calls to OpenWeather
  HTTPClient weatherClient;
  Serial.println("Making HTTP request"); // Prints "Making HTTP request" in serial monitor and goes to new line
  weatherClient.begin("http://api.openweathermap.org/data/2.5/weather?zip=yourZipgoeshere,US&appid=yourAPIkeygoeshere");  // Connects to OpenWeather API

  int httpCode = weatherClient.GET();

  Serial.println(httpCode);

  if (httpCode > 0) { // If the HTTP response status code is greater than 0, then...
    if (httpCode == 200) { // If the HTTP response status code is 200 (which is a successful status code), then
      Serial.println("Received HTTP payload."); // Prints "Received HTTP payload." in serial monitor and goes to new line
      // Alternatively use:  DynamicJsonDocument doc(1024); // Specifies JSON document and size (1024)
      StaticJsonDocument<1024> doc; // Specifies JSON document with 1024 byte size
      String payload = weatherClient.getString();
      Serial.println("Parsing..."); // Prints "Parsing..." in serial monitor and goes to new line
      deserializeJson(doc, payload);

      DeserializationError error = deserializeJson(doc, payload);
      // Test if parsing succeeds
      if (error) { // If there's an error...
        Serial.print("deserializeJson() failed with error code "); // Prints "deserializeJson() failed with error code " in serial monitor
        Serial.println(error.c_str());
        Serial.println(payload); // Prints the payload (data that was requested from the API)
        return;
      }

      // Using .dot syntax, we refer to the variable "weather" which is of type WeatherData, and place our data into the data structure.

      // Values as strings because the "slots" in WeatherData are strings
      weather.wm = doc["weather"][0]["main"].as<String>(); // Weather main description
      weather.wd = doc["weather"][0]["description"].as<String>(); // Weather further description
      weather.ca = doc["clouds"]["all"].as<String>(); // Cloud percentage
      weather.na = doc["name"].as<String>(); // Name of location (city)

      cloudiness = weather.ca.toInt(); // Makes value of variable cloudiness what weather.ca is (cloudiness percentage from API) as an integer from a string
    } else {
      Serial.println("Something went wrong with connecting to the endpoint."); // Prints "Something went wrong with connecting to the endpoint." in serial monitor and goes to new line
    }
  }
}

// Defines function for setting up WiFi connection
void setup_wifi() {
  delay(10); // Delay of 10 ms
  // We start by connecting to a WiFi network
  Serial.println(); // Starts on new line in serial monitor
  Serial.print("Connecting to "); // Prints "Connecting to " in serial monitor
  Serial.println(wifi_ssid); // Prints WiFi network name in serial monitor and goes to new line
  WiFi.begin(wifi_ssid, wifi_password); // Uses WiFi network name and password to connect
  while (WiFi.status() != WL_CONNECTED) { // While WiFi isn't connected...
    delay(500); // Wait .5 seconds
    Serial.print("."); // Print "."
  }
  Serial.println(""); // Prints a blank line and starts on a new line in serial monitor
  Serial.println("WiFi connected."); // Prints "WiFi connected." in serial monitor
  String temp = WiFi.macAddress();  // Gets the unique MAC address to use as MQTT client ID
  temp.toCharArray(mac, 6);         // .macAddress() returns a 6 byte array representing the MAC address
}

// Defines function for connecting/monitoring/reconnecting to MQTT server connection
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) { // While not connected,
    Serial.print("Attempting MQTT connection..."); // Prints "Attempting MQTT connection..." in serial monitor
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { // Uses MAC as client ID
      Serial.println("Connected!"); // Prints "Connected!" in serial monitor and goes to new line

      /// Subscribe to feed!
      //mqtt.subscribe(feed1); // Feed should be specified in config.h

    } else {
      Serial.print("failed, rc="); // Prints "failed, rc=" in serial monitor
      Serial.print(mqtt.state()); // Prints the MQTT state in serial monitor
      Serial.println(" try again in 5 seconds"); // Prints " try again in 5 seconds" in serial monitor and goes to new line
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
