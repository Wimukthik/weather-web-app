#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WiFiClient.h>

// WiFi credentials
const char* ssid = "HUAWEI Y9 2019"; // Your WiFi SSID
const char* password = "krishan@098"; // Your WiFi password

// ThingSpeak configuration
const char* apiKey = "9RDNPS9HVPU5TMPZ"; // Your ThingSpeak Write API Key
const char* channelID = "2725755"; // Your channel ID

// DHT11 Sensor
#define DHTPIN D6     // Pin for DHT11 sensor
#define DHTTYPE DHT11 // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// LDR Sensor
#define LDR_PIN A0    // Pin for LDR

// Rain Sensor
#define RAIN_DIGITAL_PIN D7 // Rain sensor digital output pin

// Wind Speed Sensor
const int anemometerPin = D5; // Pin for anemometer pulse output
volatile int pulseCount = 0;  // Variable for counting pulses
const float CALIBRATION_FACTOR = 2.0; // Adjust based on your anemometer specifications
float windSpeed = 0.0; // in km/h
unsigned long lastTime = 0; // Last time speed was calculated
const int sampleTime = 1000; // Time interval to calculate speed in milliseconds

// Interrupt function to count pulses
void ICACHE_RAM_ATTR countPulse() {
    pulseCount++; // Increment pulse count
}

void setup() {
    Serial.begin(115200); // Initialize serial communication
    WiFi.begin(ssid, password); // Connect to WiFi

    // Wait for the connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Setup DHT11
    Serial.println("DHT11 sensor test!");
    dht.begin();

    // Setup LDR
    pinMode(LDR_PIN, INPUT); // Set LDR pin as input

    // Setup Rain Sensor
    pinMode(RAIN_DIGITAL_PIN, INPUT); // Set rain digital pin as input

    // Setup Wind Speed Sensor
    pinMode(anemometerPin, INPUT_PULLUP); // Use internal pull-up resistor
    attachInterrupt(digitalPinToInterrupt(anemometerPin), countPulse, RISING); // Attach interrupt for rising edge
}

void loop() {
    unsigned long currentMillis = millis();

    // Read and print DHT11 values
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(); // Celsius by default
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor! Check wiring or sensor.");
    } else {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
    }

    // Read and print LDR value
    int ldrValue = analogRead(LDR_PIN); // Read the analog value from the LDR
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    // Read and print Rain Sensor value
    int rainDigitalValue = digitalRead(RAIN_DIGITAL_PIN); // Read digital value from the rain sensor
    Serial.print("Rain Digital Value: ");
    Serial.println(rainDigitalValue == HIGH ? "Rain Detected" : "No Rain");

    // Calculate wind speed every second
    if (currentMillis - lastTime >= sampleTime) {
        windSpeed = (pulseCount / CALIBRATION_FACTOR); // Calculate wind speed (pulses per second)
        Serial.print("Wind Speed: ");
        Serial.print(windSpeed);
        Serial.println(" km/h");
        pulseCount = 0; // Reset pulse count after calculation
        lastTime = currentMillis; // Update last time
    }

    // Send data to ThingSpeak and local database every 15 seconds
    if (currentMillis % 15000 < 2000) {
        sendDataToThingSpeak(temperature, humidity, ldrValue, rainDigitalValue, windSpeed);
        saveDataToLocalhost(temperature, humidity, ldrValue, rainDigitalValue, windSpeed);
    }

    delay(2000); // Delay for 2 seconds between readings
}

void sendDataToThingSpeak(float temperature, float humidity, int ldrValue, int rainDigitalValue, float windSpeed) {
    WiFiClient client;

    if (client.connect("api.thingspeak.com", 80)) {
        // Construct the POST request string
        String postStr = String("api_key=") + apiKey +
                         "&field1=" + String(temperature) +
                         "&field2=" + String(humidity) +
                         "&field3=" + String(ldrValue) +
                         "&field4=" + String(rainDigitalValue) +
                         "&field5=" + String(windSpeed);

        // Send the HTTP POST request
        client.print(String("POST /update HTTP/1.1\r\n") +
                     String("Host: api.thingspeak.com\r\n") +
                     String("Connection: close\r\n") +
                     String("Content-Type: application/x-www-form-urlencoded\r\n") +
                     String("Content-Length: ") + String(postStr.length()) + "\r\n\r\n" +
                     postStr);

        Serial.println("Data sent to ThingSpeak:");
        Serial.println(postStr);
    } else {
        Serial.println("Connection to ThingSpeak failed");
    }
    client.stop();
}

void saveDataToLocalhost(float temperature, float humidity, int ldrValue, int rainDigitalValue, float windSpeed) {
    WiFiClient client;

    if (client.connect("192.168.43.16", 80)) { // Replace with your local server IP address
        // Construct the POST request string
        String postStr = String("temperature=") + temperature +
                         "&humidity=" + humidity +
                         "&ldr_value=" + ldrValue +
                         "&rain_detected=" + rainDigitalValue +
                         "&wind_speed=" + windSpeed;

        // Send the HTTP POST request
        client.print(String("POST /1/save_data.php HTTP/1.1\r\n") +
                     String("Host: 192.168.43.16\r\n") +
                     String("Connection: close\r\n") +
                     String("Content-Type: application/x-www-form-urlencoded\r\n") +
                     String("Content-Length: ") + String(postStr.length()) + "\r\n\r\n" +
                     postStr);

        // Wait for the response
        while (client.available() == 0) {
            // Wait for data to be available
        }

        // Read the response
        String response = client.readString();
        Serial.println("Response from local database: " + response);
    } else {
        Serial.println("Connection to local database failed");
    }
    client.stop();
}
