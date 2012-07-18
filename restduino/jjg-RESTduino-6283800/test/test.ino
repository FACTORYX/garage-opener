#define DEBUG true

#include <SPI.h>
#include <Ethernet.h>


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(80);
IPAddress ip;

int yellow = 8;
int green1 = 7;
int green2 = 9;
int relay = 6;
int sensor = 5;

void setup()
{
#if DEBUG
  //  turn on serial (for debuggin)
  Serial.begin(9600);
#endif

  // start the Ethernet connection and the server:
    if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    }
     Serial.print("My IP address: ");
  ip = Ethernet.localIP();
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  // start listening for clients
  server.begin();
  pinMode(green1, OUTPUT);   
  pinMode(green2, OUTPUT);   
  pinMode(yellow, OUTPUT);   
  pinMode(relay, OUTPUT);   
  pinMode(sensor,INPUT);
  digitalWrite(sensor,HIGH);
}

//  url buffer size
#define BUFSIZE 255
bool garageOpen = false;
bool garageOpenSensor = false;

void loop()
{
  
  
  if (!garageOpen)
  {
    digitalWrite(green1,HIGH);
  }
  else if (garageOpen)
  {
    digitalWrite(green1,LOW);
  }
  if (!garageOpenSensor)
  {
    digitalWrite(green2,HIGH);
  }
  else if (garageOpenSensor)
  {
    digitalWrite(green2,LOW);
  }
  
  if (digitalRead(sensor) == LOW)
  {
    garageOpenSensor = true;
  }
  else
  {
    garageOpenSensor = false;
  }
  if (Serial.available() > 0) {
     int inByte = Serial.read();
    Serial.println("Wobble");
   toggleGarage(0);
  }
  
  char clientline[BUFSIZE];
  int index = 0;
  EthernetClient client = server.available();
  
  if (client)
  {
    index = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(c != '\n' && c != '\r' && index < BUFSIZE){ // Reads until either an eol character is reached or the buffer is full
          clientline[index++] = c;
          continue;
        }  
        
        #if DEBUG
        Serial.print("client available bytes before flush: "); 
        Serial.println(client.available());
        Serial.print("request = "); 
        Serial.println(clientline);
        #endif

        // Flush any remaining bytes from the client buffer
        client.flush();

         #if DEBUG
        // Should be 0
        Serial.print("client available bytes after flush: "); 
        Serial.println(client.available());
        #endif
        
        String jsonOut = String();
           jsonOut += "{\"garageStatus\":";
            jsonOut += garageOpen;
            jsonOut += ",";
            jsonOut += "\"garageSensor\":";
            jsonOut += garageOpenSensor;
            jsonOut += "}";
            
        String urlString = String(clientline);
        urlString = urlString.substring(urlString.indexOf('/'));
        urlString = urlString.substring(0,urlString.indexOf(' '));
        //operation = operation.substring(0,urlString.indexOf(' '));
         Serial.println(urlString);
         if (urlString == "/") {
             Serial.println("default status request");
           //default url request
           //dump status
           client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.print("Garage State: ");
          client.print(garageOpen);
          client.println("<br />");
          client.print("Garage Sensor: ");
          client.print(garageOpenSensor);
          client.println("<br />");
          }
          else if (urlString == "/json") {
             Serial.println("json status request");
           //default url request
           //dump status
           client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Access-Control-Allow-Origin: *");
          client.println();
           client.println(jsonOut);
          }
          else if (urlString == "/toggle") {
             Serial.println("toggle request");
           //default url request
           //dump status'
           toggleGarage(0);
           client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
           client.println(jsonOut);
          }
            else if (urlString == "/favicon.ico") {
             Serial.println("icon request");
           //default url request
           //dump status'
        
          }
          else
          {
            //invalid request
            Serial.println("invalid  request");
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println();
          }
          break;
          
     }
    }
     
     // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
  
}


void toggleGarage(int wait)
{
  if (!garageOpen)
  {
    digitalWrite(relay, HIGH);
    digitalWrite(yellow,HIGH);
    delay(wait);
   // digitalWrite(relay,LOW);
    //digitalWrite(yellow,LOW);
    garageOpen = true;
  }
  else
  {
  //  digitalWrite(relay, HIGH);
   // digitalWrite(yellow,HIGH);
    delay(wait);
    digitalWrite(relay,LOW);
    digitalWrite(yellow,LOW);
    garageOpen = false;
  }
}

