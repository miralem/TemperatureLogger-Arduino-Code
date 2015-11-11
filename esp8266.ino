#include <DallasTemperature.h> //http://milesburton.com/Dallas_Temperature_Control_Library
#include <OneWire.h>
#include <ESP8266WiFi.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

//AP definitions
#define AP_SSID "YOUR SSID"
#define AP_PASSWORD "YOUR PASSWORD"

#define EIOT_IP_ADDRESS  "188.226.133.251"
#define EIOT_PORT        8081

#define SEND_DATA_INTERVAL 60000 

OneWire  oneWire(ONE_WIRE_BUS);  //a 4.7K resistor is necessary

DeviceAddress tempDeviceAddress;

int deviceCount;

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup(void) {
  // Start up the library
  sensors.begin();
  
  deviceCount = sensors.getDeviceCount();
  
  Serial.begin(9600);
}

void loop(void) {
  
  sensors.requestTemperatures();
  String queryString = "";
  
  int i = 0;
  for(i = 0; i <= deviceCount; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){   
      String addr = getAdderss(tempDeviceAddress);
      float temp = sensors.getTempCByIndex(i);

      queryString += "temps%5B";
      queryString += i;
      queryString += "%5D%5Bid%5D=";
      queryString += addr;
      queryString += "&temps%5B";
      queryString += i;
      queryString += "%5D%5Btemp%5D=";
      queryString += temp;
      queryString += "&";
    }
  }

  sendTemperature(queryString);
  delay(SEND_DATA_INTERVAL);
}

void sendTemperature(String queryString)
{  
   WiFiClient client;
   
   while(!client.connect(EIOT_IP_ADDRESS, EIOT_PORT)) {
    Serial.println("connection failed");
    wifiConnect(); 
  }
 
  String url = "/api?";
  url += queryString;
  
  Serial.print("POST data to URL: ");
  Serial.println(url);
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(EIOT_IP_ADDRESS) + "\r\n" + 
               "Connection: close\r\n" + 
               "Content-Length: 0\r\n" + 
               "\r\n");

  delay(100);
    while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("Connection closed");
}

String getAdderss(DeviceAddress deviceAddress)
{
  String str = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) str += "0";    
    str += String(deviceAddress[i], HEX);
  }

  return str;
}

void wifiConnect()
{
    Serial.print("Connecting to AP");
    WiFi.begin(AP_SSID, AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
}
