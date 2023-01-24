 #include <Wire.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include "MAX30100_PulseOximeter.h"
#include "esp_adc_cal.h"

 
#define LM35_Sensor1    25
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;

int LM35_Raw_Sensor1 = 0;
float LM35_TempC_Sensor1 = 0.0;
float LM35_TempF_Sensor1 = 0.0;
float Voltage = 0.0;
int beatavg;
 
uint32_t tsLastReport = 0;

void onBeatDetected()
{
    //Serial.println("Beat!");
}

 
uint32_t readADC_Cal(int ADC_Raw)
{
  esp_adc_cal_characteristics_t adc_chars;
  
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}

//wifi dan pass

const char* ssid="sin45";   //ssid wifi
const char* pass="12345678";  //password wifi

//host
const char* host="192.168.1.6";  //IP addres komputer  
                                 //hosting url

void setup()
{
    Serial.begin(115200);
/*
    //koneksi ke jaringan wifi
    WiFi.begin(ssid, pass);
    Serial.println("connecting");
    while(WiFi.status()!= WL_CONNECTED){
      Serial.print("not connected!");
      delay(1000);
      Serial.println();
    }
    Serial.println("connected");
    
*/
    
    Serial.print("Initializing pulse oximeter..");

    
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  LM35_Raw_Sensor1 = analogRead(LM35_Sensor1);  
  Voltage = readADC_Cal(LM35_Raw_Sensor1);
  LM35_TempC_Sensor1 = Voltage / 10;
  LM35_TempF_Sensor1 = (LM35_TempC_Sensor1 * 1.8) + 32;
  // Make sure to call update as fast as possible
  
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate: ");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm | SpO2: ");
        Serial.print(pox.getSpO2());
        Serial.print("% | Suhu: ");
        Serial.print(LM35_TempC_Sensor1);
        Serial.println(" °C ");
        tsLastReport = millis();
    }
        //webserver 
        WiFiClient client;
        const int httpPort = 80 ; //inisialisasi port web server
        if(! client.connect(host,httpPort)){
          Serial.println("connection failed");
          return;
        }
        String Link;
        HttpClient http;

        Link = "http;//"+String (host)+ "/multisensor/kirimdata.php?HeartRate="+ String(pox.getHeartRate)+"&SpO2="+ String(pox.getSpO2);
        //eksekusi alamat link
        http.begin(Link);
        http.GET();

         //respon berhasil dikirim
        String respon = http.getString();
        Serial.println(respon);
        http.end(); 
    }
    
}
