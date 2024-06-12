/*
input DC --> 5V
SDA <--> 41
SCL <--> 42
*/
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             5        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

String id = "F001";  //====================
int SEC_SLEEP_TIME = 10*60;  //====================

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
bool lora_idle=true;
static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

unsigned long date;
float temp;
float humid;
float hic;
float co2;

SCD4x mySensor;

void setup() {

    Serial.begin(115200);
    Serial.println("WakeUp........");

    //setup lora
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

    //setup SCD41 and lowpower
    Wire.begin();
    if (mySensor.begin() == false)
    {
      Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
      while (1)
        ;
    }
    if (mySensor.stopPeriodicMeasurement() == true)
    { Serial.println(F("Periodic measurement is disabled!")); }  
    if (mySensor.startLowPowerPeriodicMeasurement() == true)
    { Serial.println(F("Low power mode enabled!")); }

    //read scd41
    while(!mySensor.readMeasurement()){
      Serial.print(F("."));
      delay(1000);
    }
    temp = mySensor.getTemperature();
    humid = mySensor.getHumidity();
    co2 = mySensor.getCO2();
    // hic = computeHeatIndex(temp,humid,false);
    hic = -1;
    Serial.print(F("\nTemperature(C):")); Serial.print(temp);
    Serial.print(F("\tHumidity(%RH):")); Serial.print(humid);
    Serial.print(F("\thic:")); Serial.println(hic);

   }

void loop()
{
	if(lora_idle == true)
	{
    delay(1000);
		
    sprintf(txpacket,"id:%s,temp:%0.2f,humid:%0.2f,hic:%0.2f,co2:%0.2f",id,temp,humid,hic,co2);  //start a package
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    
	}
  Radio.IrqProcess( );
}

void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
  Serial.println("Sleep........");
  esp_sleep_enable_timer_wakeup(SEC_SLEEP_TIME*1000*(uint64_t)1000);  //20 sec
	esp_deep_sleep_start();
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}