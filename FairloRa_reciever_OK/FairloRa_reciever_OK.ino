/*
input DC --> 5V
SDA <--> 41
SCL <--> 42
*/
#include <TridentTD_LineNotify.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here
String LINE_TOKEN;

#define NODE_NAME                                   "M001"  //-------------------------------------------

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
int16_t rssi,rxSize;
bool lora_idle = true;

WiFiUDP ntpUDP;
const long offsetTime = 25200;       // หน่วยเป็นวินาที จะได้ 7*60*60 = 25200
NTPClient timeClient(ntpUDP, "pool.ntp.org", offsetTime);

float adj_temp;
float adj_humid;
float adj_co2;
bool firsttime = true;

float computeHeatIndex(float temperature, float humidity, bool isFahrenheit) {
  // Constants for the heat index calculation
  const float c1 = -42.379;
  const float c2 = 2.04901523;
  const float c3 = 10.14333127;
  const float c4 = -0.22475541;
  const float c5 = -0.00683783;
  const float c6 = -0.05481717;
  const float c7 = 0.00122874;
  const float c8 = 0.00085282;
  const float c9 = -0.00000199;
  float heatIndex;
  if (!isFahrenheit) {
    // Convert temperature to Fahrenheit for calculation
    temperature = temperature * 1.8 + 32;
  }
  heatIndex = c1 + (c2 * temperature) + (c3 * humidity) + (c4 * temperature * humidity) + 
              (c5 * temperature * temperature) + (c6 * humidity * humidity) + 
              (c7 * temperature * temperature * humidity) + (c8 * temperature * humidity * humidity) + 
              (c9 * temperature * temperature * humidity * humidity);
  if (!isFahrenheit) {
    // Convert back to Celsius
    heatIndex = (heatIndex - 32) / 1.8;
  }
  return heatIndex;
}

// Function to extract the value for a given key
String getValue(String data, String key) {
  int startIndex = data.indexOf(key) + key.length() + 1;
  int endIndex = data.indexOf(',', startIndex);
  if (endIndex == -1) {
    endIndex = data.length();
  }
  return data.substring(startIndex, endIndex);
}

void parseJSON(const char* jsonString) {
  // Parse JSON object
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  // Check for parsing errors
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  // Extract values
  const char* id = doc["id"];
  const char* unit = doc["unit"];
  adj_temp = doc["adj_temp"];
  adj_humid = doc["adj_humid"];
  adj_co2 = doc["adj_co2"];
  const char* line1 = doc["line1"];
  const char* line2 = doc["line2"];
  const char* line3 = doc["line3"];

  LINE_TOKEN = String(line1);
   
}

void setup() {
    Serial.begin(115200);

    //config wifi
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("myiot")) {
      Serial.println("Failed to connect and hit timeout");
      ESP.restart();
    }
    Serial.println("Connected to WiFi!");
    Serial.println(WiFi.localIP());

    delay(500);

    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
    rssi=0;
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}

void loop()
{
  if((WiFi.status() == WL_CONNECTED) && (firsttime==true)){
    //get config
    String Read_Data_URL = "https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=getConfig&id=F001";
    HTTPClient http;
    http.begin(Read_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload : " + payload);  
      parseJSON(payload.c_str());
    }
    http.end();
    if (httpCode == 200) {
      Serial.println("Payload : " + payload); 
     
    }
    Serial.print("adj_temp");  Serial.println(adj_temp);
    Serial.print("adj_humid");  Serial.println(adj_humid);
    Serial.print("adj_co2");  Serial.println(adj_co2);
    Serial.print("LINE_TOKEN");  Serial.println(LINE_TOKEN);

    LINE.setToken(LINE_TOKEN);
    String m = String(NODE_NAME) + "--online!";
    LINE.notify(m);

    firsttime = false;
  
  }
  
  if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);

    String dataString = String(rxpacket);

    // Extract values using the getValue function
    String id = getValue(dataString, "id");
    float temp = getValue(dataString, "temp").toFloat();
    float humid = getValue(dataString, "humid").toFloat();
    float hic = getValue(dataString, "hic").toFloat();
    float co2 = getValue(dataString, "co2").toFloat();

    temp += adj_temp;
    humid += adj_humid;
    co2 += adj_co2;
    hic = computeHeatIndex(temp,humid,false);

    Serial.println(id);
    Serial.println(temp);
    Serial.println(humid);
    Serial.println(hic);
    Serial.println(co2);

    //post api
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient https;
      timeClient.update();
      String date = String(timeClient.getEpochTime());
      String m;
      m += "DeviceId: " + id +"\n";
      m += "•from: " + String(NODE_NAME) + "\n";
      m += "•temp: " + String(temp, 2) + " °C\n";
      m += "•humid: " + String(humid, 2) + "%\n";
      m += "•hic: " + String(hic, 2) + " °C\n";
      m += "•co2: " + String(co2, 0) + "\n";
      m += "•timestamp: " + date + "\n";
    
      LINE.notify(m);

      String sheet_api = "https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=addData";

      if(https.begin(sheet_api.c_str())) { 
           
        String p = "{\"date\":\"" + date + "\",\"id\":\"" + id + "\",\"temp\":" + String(temp, 2) + ",\"humid\":" + String(humid, 2) + ",\"hic\":" + String(hic, 2) + ",\"co2\":" + String(co2, 0) + ",\"flag\":\"" + NODE_NAME + "\"}";
        Serial.print("p:"); Serial.println(p);

        https.addHeader("Content-Type", "application/json");
        int httpResponseCode = https.POST(p);
        String content = https.getString();
        Serial.print("\nhttpResponseCode"); Serial.println(httpResponseCode);
        // Serial.print("content"); Serial.println(content);
        https.end();
      }
    }

    lora_idle = true;
}