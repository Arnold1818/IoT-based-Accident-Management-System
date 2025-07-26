#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPS module connections
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // RX, TX

// SD card pin (CS pin)
const int chipSelect = 10;

// Collision switch sensor pin
const int collisionSwitchPin = 2;

// LCD display (16x2) with I2C address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables to store the latest GPS coordinates
double currentLatitude = 0.0;
double currentLongitude = 0.0;

// Function to calculate the distance between two points (Haversine formula)
double haversine(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371e3; // Earth radius in meters
  double phi1 = lat1 * PI / 180;
  double phi2 = lat2 * PI / 180;
  double deltaPhi = (lat2 - lat1) * PI / 180;
  double deltaLambda = (lon2 - lon1) * PI / 180;

  double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
             cos(phi1) * cos(phi2) *
             sin(deltaLambda / 2) * sin(deltaLambda / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c; // Distance in meters
}

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1); // Halt the program if SD card initialization fails
  }
  Serial.println("Card initialized.");

  // Initialize the collision switch pin
  pinMode(collisionSwitchPin, INPUT);

  // Initialize the LCD
  lcd.begin(16, 2); // Specify the number of columns and rows
  lcd.backlight();
  lcd.print("System Ready");
  delay(2000); // Display "System Ready" for 2 seconds
  lcd.clear();
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      currentLatitude = gps.location.lat();
      currentLongitude = gps.location.lng();
      Serial.print("Latitude: ");
      Serial.println(currentLatitude, 6);
      Serial.print("Longitude: ");
      Serial.println(currentLongitude, 6);
    }
  }

  if (digitalRead(collisionSwitchPin) == LOW) {
    // Collision detected
    lcd.clear();
    lcd.print("Alert Detected");
    Serial.println("Alert Detected");

    // Fetch and display GPS location
    Serial.print("Latitude: ");
    Serial.println(currentLatitude, 6);
    Serial.print("Longitude: ");
    Serial.println(currentLongitude, 6);

    findNearestHospital(currentLatitude, currentLongitude);
    findNearestPoliceStation(currentLatitude, currentLongitude);
    delay(10000); // Wait for 10 seconds before the next check
  }
}

void findNearestHospital(double latitude, double longitude) {
  File dataFile = SD.open("HOSPIT~1.CSV");

  if (!dataFile) {
    Serial.println("Error opening HOSPIT~1.CSV");
    return;
  }

  String nearestHospitalName;
  String nearestHospitalPhone;
  double shortestDistance = -1;

  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    if (line.length() == 0) continue;

    // Parse the line
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    int thirdComma = line.indexOf(',', secondComma + 1);

    String name = line.substring(0, firstComma);
    double lat = line.substring(firstComma + 1, secondComma).toDouble();
    double lon = line.substring(secondComma + 1, thirdComma).toDouble();
    String phone = line.substring(thirdComma + 1);

    double distance = haversine(latitude, longitude, lat, lon);

    if (shortestDistance == -1 || distance < shortestDistance) {
      shortestDistance = distance;
      nearestHospitalName = name;
      nearestHospitalPhone = phone;
    }
  }
  dataFile.close();

  if (nearestHospitalName != nullptr) {
    Serial.println("Nearest hospital:");
    Serial.print("Name: ");
    Serial.println(nearestHospitalName);
    Serial.print("Phone Number: ");
    Serial.println(nearestHospitalPhone);
    Serial.print("Distance: ");
    Serial.print(shortestDistance / 1000, 2); // Convert to km
    Serial.println(" km");
  }
}

void findNearestPoliceStation(double latitude, double longitude) {
  File dataFile = SD.open("POLICE~1.CSV");

  if (!dataFile) {
    Serial.println("Error opening POLICE~1.CSV");
    return;
  }

  String nearestPoliceStationName;
  String nearestPoliceStationPhone;
  double shortestDistance = -1;

  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    if (line.length() == 0) continue;

    // Parse the line
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    int thirdComma = line.indexOf(',', secondComma + 1);

    String name = line.substring(0, firstComma);
    double lat = line.substring(firstComma + 1, secondComma).toDouble();
    double lon = line.substring(secondComma + 1, thirdComma).toDouble();
    String phone = line.substring(thirdComma + 1);

    double distance = haversine(latitude, longitude, lat, lon);

    if (shortestDistance == -1 || distance < shortestDistance) {
      shortestDistance = distance;
      nearestPoliceStationName = name;
      nearestPoliceStationPhone = phone;
    }
  }
  dataFile.close();

  if (nearestPoliceStationName != nullptr) {
    Serial.println("Nearest police station:");
    Serial.print("Name: ");
    Serial.println(nearestPoliceStationName);
    Serial.print("Phone Number: ");
    Serial.println(nearestPoliceStationPhone);
    Serial.print("Distance: ");
    Serial.print(shortestDistance / 1000, 2); // Convert to km
    Serial.println(" km");
  }
}
