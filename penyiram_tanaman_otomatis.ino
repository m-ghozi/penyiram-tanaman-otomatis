#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
// Replace with your network credentials
const char* ssid = "rell";
const char* password = "12345678888";

// Initialize Telegram BOT
#define BOTtoken "6683503979:AAHN2JfsBZooCKDn5nmEVgJnBpAH5CfzAH4"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can message you
#define CHAT_ID "1200713317"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Setup relay pin for pump
const int relayPin = 2;
bool ledState = LOW;

// Setup soil Moisture Sensor
#define soilSensor 35
int _moisture;
String kondisi_tanah;
String status_pompa = "Mati";

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Hallo, " + from_name + ".\n";
      welcome += "Gunakan perintah-perintah berikut.\n\n";
      welcome += "/siram untuk menyiram secara manual \n";
      welcome += "/status untuk melihat status tanah dan pompa \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/siram") {
      bot.sendMessage(chat_id, "Tanaman disiram secara Manual", "");
      ledState = HIGH;
      digitalWrite(relayPin, ledState);
      lcd.setCursor(0,1);
      lcd.print("Pompa Nyala");
      status_pompa = "Menyala";
      delay(5000);
      digitalWrite(relayPin,LOW);
      lcd.setCursor(0,1);
      lcd.print("Pompa Mati ");
      status_pompa = "Mati";
    }
    
    
    if (text == "/status") {
      String _status = "Kondisi Tanah saat ini : " + kondisi_tanah + ", " + _moisture + "%" + ".\n";
      _status += "Kondisi Pompa : " + status_pompa + ".\n";
      bot.sendMessage(chat_id, _status, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  readSoil();
}

void readSoil() {
  int value = analogRead(soilSensor); // read the analog value from sensor
  _moisture = ( 100 - ( (value/4095.00) * 100));
  Serial.print("Moisture = ");
  Serial.print(_moisture);  /* Print Temperature on the serial window */
  Serial.println("%");
  delay(200);              /* Wait for 1000mS */
  if (_moisture > 30) {
    lcd.setCursor(0,0);
    kondisi_tanah = "Basah";
    lcd.print("Basah ");
    lcd.print(_moisture);
    lcd.print("%  ");
  } else {
    lcd.setCursor(0,0);
    kondisi_tanah = "Kering";
    lcd.print("Kering ");
    lcd.print(_moisture);
    lcd.print("%  ");
  }
}
