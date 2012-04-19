/*
Dieser Code basiert auf den Arduino-libs und ihren Beispielen.
*/

#define DS1631_ADDR 0x4D //7bit I2C addr des DS1631

#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

// MAC und IP
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,178,200);

EthernetServer server(80); // Ethershield inintialisieren

void setup()
{
  Wire.begin();             //I2C init

  // DS1631 stoppen um ihn konfigurieren zu koennen
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write((int)(0x22)); // Stop conversion
  Wire.endTransmission();  

  // Konfig schreiben
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0xAC); // in das Configbyte schreibe
  Wire.write(0x0C); // continuous conversion und 12 bit
  Wire.endTransmission();

  // endless conversion starten
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write((int)(0x51));
  Wire.endTransmission();

  Ethernet.begin(mac, ip); // IP und MAC setzen
  server.begin();
}

void loop()
{

  // auf HTTP-request warten
  EthernetClient client = server.available();
  if (client) {
    // http request will nix weiter als root
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // Temperatur auslesen
        Wire.beginTransmission(DS1631_ADDR);
        Wire.write((int)(0xAA)); // Temperatur
        Wire.endTransmission();
        Wire.requestFrom(DS1631_ADDR,2); // 2 bytes anfordern
        Wire.available(); // 1. byte
        int Th = Wire.read(); // hi byte lesen
        Wire.available(); // 2. byte
        int Tl = Wire.read(); // lo byte lesen
        // Temperatur umrechnen
        if(Th>=0x80) //sig detect um negatives vorzeichen der 12bit zu erkennen
          Th = Th - 256;
        int T_dec=(10*(100*(Tl/16)))/16; // Kommastelle
        //
        if (c == '\n' && currentLineIsBlank) {
          // einfachen HTTP-responseheader senden + Temperaturline
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.print("Temperature: ");
          client.print(Th);   
          client.print(".");
          if (T_dec<10)   client.print("0");
          if (T_dec<100)   client.print("0");
          client.print(T_dec);
          client.print("&#176;C");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1); // warten bis alles gesendet ist
    client.stop(); // connection zu machen
  }
}




