#include <WiFi.h>
#include <ThingSpeak.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26

HardwareSerial GSM(1);
HardwareSerial gpsSerial(2);

TinyGPSPlus gps;

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

unsigned long channelID = YOUR_CHANNEL_ID;
const char* writeAPIKey = "YOUR_WRITE_API_KEY";

WiFiClient client;

void setup() {
  Serial.begin(115200);

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, 32, 33);

  // GSM Power ON
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_POWER_ON, HIGH);
  delay(100);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(3000);

  GSM.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ThingSpeak.begin(client);
}

void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {

    float lat = gps.location.lat();
    float lng = gps.location.lng();
    int satellites = gps.satellites.value();

    Serial.println("Location Updated");

    ThingSpeak.setField(1, lat);
    ThingSpeak.setField(2, lng);
    ThingSpeak.setField(3, satellites);
    ThingSpeak.writeFields(channelID, writeAPIKey);

    // Send SMS once when GPS detected
    sendSMS(lat, lng);

    delay(20000);  // wait before next update
  }
}

void sendSMS(float lat, float lng) {

  GSM.println("AT+CMGF=1");
  delay(1000);

  GSM.println("AT+CMGS=\"+91XXXXXXXXXX\"");
  delay(1000);

  GSM.print("Alert! Location: ");
  GSM.print(lat, 6);
  GSM.print(", ");
  GSM.print(lng, 6);

  GSM.write(26);
  delay(5000);
}