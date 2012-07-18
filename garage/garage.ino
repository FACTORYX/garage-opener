#include <SPI.h>
#include <Ethernet.h>

//CONFIGURATION IS UP TO DATE
#define RED_PIN 9
#define GREEN_PIN 8
#define YELLOW_PIN 7
#define RELAY_PIN 5
#define SWITCH_PIN 6

bool red_state = false;
bool yellow_state = false;
bool green_state = false;
bool garage_state = false;
bool relay_state = false;
int switch_state = 0;
unsigned long triggertime;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//byte ip[] = {192,168,1,177};

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
#if defined(ARDUINO) && ARDUINO >= 100
EthernetServer server(80);
#else
Server server(80);
#endif

void setup()
{
  triggertime =  millis();
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN, HIGH);
  Serial.begin(9600);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
}

//  url buffer size
#define BUFSIZE 255
// Toggle case sensitivity
#define CASESENSE true

void loop()
{
  char clientline[BUFSIZE];
  readSwitch();
  int index = 0;
  // listen for incoming clients
#if defined(ARDUINO) && ARDUINO >= 100
  EthernetClient client = server.available();
#else
  Client client = server.available();
#endif
  if (client) {
    //  reset input buffer
    index = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //  fill url the buffer
        if(c != '\n' && c != '\r' && index < BUFSIZE){ // Reads until either an eol character is reached or the buffer is full
          clientline[index++] = c;
          continue;
        }  
       // Serial.print("client available bytes before flush: "); 
       // Serial.println(client.available());
        Serial.print("request = "); 
        Serial.println(clientline);
        // Flush any remaining bytes from the client buffer
        client.flush();
        // Should be 0
       // Serial.print("client available bytes after flush: "); 
        //Serial.println(client.available());
        String urlString = String(clientline);
        urlString = urlString.substring(urlString.indexOf('/'), urlString.indexOf(' ', urlString.indexOf('/')));
        urlString.toUpperCase();
        Serial.print("URL STRING: "); 
        Serial.println(urlString);
        int urlStringLength = urlString.length();
        int slashCount = 0;
        for (int i = 0; i < urlStringLength; i++)
        {
          if (urlString.charAt(i) == '/')
          {
            slashCount++;
          }
        }
        //  get the first two parameters
        int firstSlash = urlString.indexOf('/');
        int lastSlash = urlString.lastIndexOf('/');
        
        if (slashCount == 1)
        {
          if (urlString == "/")
          {
          //default / request
          //Print Status
          String jsonOut = String();
          jsonOut += "{\"";
          jsonOut += "Garage Status";
          jsonOut += "\":\"";
          jsonOut += garage_state;
          jsonOut += "\"}";
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Access-Control-Allow-Origin: *");
          client.println();
          client.println(jsonOut);
          }
          else if (urlString == "/FAVICON.ICO")
          {
          //do nothing
          }
          else {
             Serial.println("erroring");
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println();
          }
        }
        else if (slashCount == 2)
        {
          //We have a command
          Serial.println("Command");
          if (urlString == "/TOGGLE/")
          {
            if (triggerRelay()){
            String jsonOut = String();
            jsonOut += "{\"";
            jsonOut += "Garage Status";
            jsonOut += "\":\"";
            jsonOut += garage_state;
            jsonOut += "\"}";
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Access-Control-Allow-Origin: *");
            client.println();
            client.println(jsonOut);
            }
            else
            {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Access-Control-Allow-Origin: *");
            client.println();
            client.println("Please wait 10 seconds before toggling garage door again.");
            }
          }
        }
        else if (slashCount == 3)
        {
          //We have a command + parameter
          Serial.println("Command + Parameter");
        }
 
        else
        {
          //Error
           Serial.println("erroring");
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

bool triggerRelay()
{
  unsigned long x =  millis();
  unsigned long diff = x - triggertime;
  if (diff < 10000)
  {
    //too soon
    //Serial.println("Wait 10 seconds before toggling relay again."); 
    //return false;
  }
  
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(YELLOW_PIN, HIGH); 
  delay(1000);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW); 

  triggertime =  millis();
  return true;
  
  

 /* if (garage_state == false){
    garage_state = true;
      //digitalWrite(RED_PIN, true); 
    //digitalWrite(GREEN_PIN, false); 
    Serial.println("Garage open."); 
    return;
  }
  else if (garage_state == true);
  {
    garage_state = false;
       //digitalWrite(RED_PIN, false); 
    //digitalWrite(GREEN_PIN, true);
    Serial.println("Garage closed."); 
    return;
  }
  */
}
void readSwitch()
{
  switch_state = digitalRead(SWITCH_PIN);
  if (switch_state == 1)
  {
    garage_state = true;
   //  Serial.println("Garage open."); 
    digitalWrite(RED_PIN, true); 
    digitalWrite(GREEN_PIN, false); 
  }
  else if (switch_state == 0)
  {
    garage_state = false;
   // Serial.println("Garage closed."); 
    digitalWrite(RED_PIN, false); 
    digitalWrite(GREEN_PIN, true); 
  }
}

