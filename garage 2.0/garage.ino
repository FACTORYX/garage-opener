#include <SPI.h>
#include <Ethernet.h>

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

unsigned long triggerTimestamp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server[] = { 10,0,0,50 };

EthernetClient client;

void setup() {
  triggerTimestamp = millis();
  // start the serial library:
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
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server,1337)) {
    Serial.println("connected");
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
 
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN, HIGH);
}

void loop()
{
  //if you are connected and data is available
  if (client.connected()) {
    readSwitch();
  }
  
  if (client.available()) {
    char c = client.read();
    if (c == 'S') {
      if (readSwitch()) {
        client.print('OPEN'); //open
      }
      else {
         client.print('CLOSED'); //closed
      }
    }
    else if (c == 'T') {
      if (triggerRelay(500)) {
        client.print('1'); //success
      }
      else {
        client.print('0'); //fail (spammy)
      }
    } 
  }
  delay(250);
}

bool triggerRelay(int ms)
{
  Serial.print("[TRIGGER] ");
  if (millis() - triggerTimestamp < 5000) {
    Serial.println("FAIL");
    return false;
  }
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(YELLOW_PIN, HIGH); 
  delay(ms);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW); 
  triggerTimestamp = millis();
  Serial.println("SUCCESS");
  return true;
}

bool readSwitch()
{
  switch_state = digitalRead(SWITCH_PIN);
  unsigned long now = millis();
  Serial.print(now);
  Serial.print(" [STATUS] "); 
  Serial.print(switch_state); 
  if (switch_state == 1)
  {
    garage_state = true;
    Serial.println("OPEN"); 
    client.print("OPEN");
    digitalWrite(RED_PIN, true); 
    digitalWrite(GREEN_PIN, false); 
    return true;
  }
  else if (switch_state == 0)
  {
    garage_state = false;
    Serial.println("CLOSED");
    client.print("CLOSED");
    digitalWrite(RED_PIN, false); 
    digitalWrite(GREEN_PIN, true); 
    return false;
  }
}
