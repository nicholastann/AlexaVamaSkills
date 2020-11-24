#include <ArduinoJson.h>
#include <IRRemote.h>
#include <WiFi.h>

#define LED1 RED_LED
#define LED2 GREEN_LED
#define IRLEDpin  19              //the arduino pin connected to IR LED to ground. HIGH=LED ON

#define BITtimeNEC   562            //length of the carrier bit in microseconds
#define BITtimeSony   1200
#define BITtimeRC5   889
#define BITtimeRC6   444

unsigned long powerCode = 0b00000000000000000000000000000000;
unsigned long chanUp =   0b00000000000000000000000000000000;
unsigned long chanDown = 0b00000000000000000000000000000000;
unsigned long volUp =    0b00000000000000000000000000000000;
unsigned long volDown = 0b00000000000000000000000000000000;

//connect to wifi
WiFiClient client; 
char ssid[] = "Brandon's iPhone";
char password[] = "bv0976814";
int keyIndex = 0;

const char* server = "vama.herokuapp.com";

// Replace with url for other devices: change id=2 for switch, id=3 for lock
const char* resource = "/api/view.php?id=1";

const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// The type of data that we want to extract from the page
struct clientData {
  char name[8];
  char Status[8];
};

const int buttonPin1 = 5;
const int buttonPin2 = 8;
const int buttonPin3 = 17;
const int buttonPin4 = 31;
const int buttonPin5 = 11;
const int buttonPin5 = 12;

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;
int buttonState5 = 0;
int buttonState6 = 0;

void setup() {
pinMode(buttonPin1,INPUT);
pinMode(buttonPin2,INPUT);
pinMode(buttonPin3,INPUT);
pinMode(buttonPin4,INPUT);
pinMode(buttonPin5,INPUT);
pinMode(buttonPin6,INPUT);
pinMode(LED1, OUTPUT);
pinMode(LED2, OUTPUT);
pinMode(IRLEDpin, OUTPUT);
digitalWrite(IRLEDpin, LOW);    //turn off IR LED to start
Serial.begin(115200);
pinMode(RED_LED, OUTPUT);      // set the LED pin mode
// attempt to connect to Wifi network:
Serial.print("Attempting to connect to Network named: ");
// print the network name (SSID);
Serial.println(ssid); 
// Connect to WPA/WPA2 network.
WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while waiting to connect
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nYou're connected to the network");
  digitalWrite(RED_LED, HIGH); //start led off
  delay(1000);
}

void IRcarrierNEC(unsigned int IRtimemicroseconds)
{
  for(int i=0; i < (IRtimemicroseconds / 26); i++)
    {
    digitalWrite(IRLEDpin, HIGH);   //turn on the IR LED
    //NOTE: digitalWrite takes about 3.5us to execute, so factor that into the timing.
    delayMicroseconds(13);          //delay for 13us (9us + digitalWrite), half the carrier frequnecy
    digitalWrite(IRLEDpin, LOW);    //turn off the IR LED
    delayMicroseconds(13);          //delay for 13us (9us + digitalWrite), half the carrier frequnecy
    }
}

void IRcarrierSony(unsigned int IRtimemicroseconds)
{
  for(int i=0; i < (IRtimemicroseconds / 25); i++)
    {
    digitalWrite(IRLEDpin, HIGH);   //
    delayMicroseconds(11);      
    digitalWrite(IRLEDpin, LOW);    
    delayMicroseconds(11);         
    }
}
void IRcarrierRC5(unsigned int IRtimemicroseconds)
{
  for(int i=0; i < (IRtimemicroseconds / 28); i++)
    {
    digitalWrite(IRLEDpin, HIGH);   
    delayMicroseconds(13);          
    digitalWrite(IRLEDpin, LOW);    
    delayMicroseconds(13);          
    }
}

void IRcarrierRC6(unsigned int IRtimemicroseconds)
{
  for(int i=0; i < (IRtimemicroseconds / 28); i++)
    {
    digitalWrite(IRLEDpin, HIGH); 
    delayMicroseconds(13);         
    digitalWrite(IRLEDpin, LOW);    
    delayMicroseconds(13);         
    }
}

void IRsendCodeNEC(unsigned long code)
{
  //send the leading pulse
  IRcarrierNEC(9000);            //9ms of carrier
  delayMicroseconds(4500);    //4.5ms of silence
  
  //send the user defined 4 byte/32bit code
  for (int i=0; i<32; i++)            //send all 4 bytes or 32 bits
    {
    IRcarrierNEC(BITtimeNEC);               //turn on the carrier for one bit time
    if (code & 0x80000000)            //get the current bit by masking all but the MSB
      delayMicroseconds(3 * BITtimeNEC); //a HIGH is 3 bit time periods
    else
      delayMicroseconds(BITtimeNEC);     //a LOW is only 1 bit time period
     code<<=1;                        //shift to the next bit for this byte
    }
  IRcarrierNEC(BITtimeNEC);                 //send a single STOP bit.
}

void IRsendCodeSony(unsigned long code)
{
  //send the leading pulse
  IRcarrierSony(2400);          
  delayMicroseconds(600);    
  
  for (int i=0; i<32; i++)           
    {
    IRcarrierSony(BITtimeSony);              
    if (code & 0x80000000)           
      delayMicroseconds(BITtimeSony); 
    else
      delayMicroseconds(BITtimeSony/2);     
     code<<=1;                        
    }
  IRcarrierSony(BITtimeSony);                
}

void IRsendCodeRC5(unsigned long code)
{
  //send the leading pulse
  IRcarrierRC5(9000);       
  delayMicroseconds(4500);    
  
  //send the user defined 4 byte/32bit code
  for (int i=0; i<32; i++)           
    {
    IRcarrierRC5(BITtimeRC5);              
    if (code & 0x80000000)        
      delayMicroseconds(BITtimeRC5*2); 
    else
      delayMicroseconds(BITtimeRC5);    
     code<<=1;                       
    }
  IRcarrierSony(BITtimeRC5);               
}

void IRsendCodeRC6(unsigned long code)
{
  //send the leading pulse
  IRcarrierRC6(2666);           
  delayMicroseconds(889);    

  for (int i=0; i<32; i++)           
    {
    IRcarrierRC6(BITtimeRC6);             
    if (code & 0x80000000)           
      delayMicroseconds(BITtimeRC6*2); 
    else
      delayMicroseconds(BITtimeRC6);     
     code<<=1;                       
    }
  IRcarrierRC6(BITtimeRC6);            
}

// Open connection to the HTTP server
bool connect(const char* hostName) {
  Serial.print("Connect to ");
  Serial.println(hostName);

  bool ok = client.connect(hostName, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
  Serial.print("GET ");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.0");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();
  return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  return ok;
}

bool readReponseContent(struct clientData* clientData) {

  const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
  
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(client);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }
  strcpy(clientData->name, root["name"]);
  strcpy(clientData->Status, root["status"]);
  return true;
}

// Print the data extracted from the JSON
void printclientData(const struct clientData* clientData) {
  Serial.print("Name = ");
  Serial.println(clientData->name);
  Serial.print("Status = ");
  Serial.println(clientData->Status);
  if (atoi(clientData->Status)==0){
    Serial.println("111");
  }
  else if(atoi(clientData->Status)==1){
      IRsendCodeNEC(powerCode); 
      Serial.println("sent5");
      delay(200);
  }
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause
void wait() {
  Serial.println("Wait 5 seconds");
  delay(5000);
}

void loop() {
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);
  buttonState4 = digitalRead(buttonPin4);
  buttonState5 = digitalRead(buttonPin5);
  buttonState6 = digitalRead(buttonPin6);
  
  if (buttonState1==HIGH){
      IRsendCodeRC6(volDown); 
      Serial.println("sent1");
      delay(200);
    }

  if (buttonState2==HIGH){
      IRsendCodeRC5(volUp); 
      Serial.println("sent2");
      delay(200);
    }

  if (buttonState3==HIGH){ //sony requires code to be sent 3 times
      IRsendCodeSony(chanDown); 
      IRsendCodeSony(chanDown); 
      IRsendCodeSony(chanDown); 
      Serial.println("sent3");
      delay(200);
    }

  if (buttonState4==HIGH){
      IRsendCodeNEC(chanUp); 
      Serial.println("sent4");
      delay(200);
    }
 
  if (buttonState5==HIGH){
      IRsendCodeNEC(powerCode); 
      Serial.println("sent5");
      delay(200);
    }     
   
   if (buttonState6==HIGH){
    //clear current codes
      powerCode = 0b00000000000000000000000000000000;
      chanUp =   0b00000000000000000000000000000000;
      chanDown = 0b00000000000000000000000000000000;
      volUp =    0b00000000000000000000000000000000;
      volDown = 0b00000000000000000000000000000000;
      
      if (irrecv.decode()) { //if a signal is picked up
        powerCode = irrecv.results()
        irrecv.resume();
        delay(150);
      }

       if (irrecv.decode()) { 
        chanUp = irrecv.results()
        irrecv.resume();
        delay(150);
      }
       if (irrecv.decode()) {
        chanDown = irrecv.results()
        irrecv.resume();
        delay(150);
      }
       if (irrecv.decode()) { 
        volUp = irrecv.results()
        irrecv.resume();
        delay(150);
      }
      if (irrecv.decode()) { 
        volDown = irrecv.results()
        irrecv.resume();
        delay(150);
      }
    }   

  //server ping
  if(connect(server)) {
    if(sendRequest(server, resource) && skipResponseHeaders()) {
      clientData x;
      if(readReponseContent(&x)) {
      printclientData(&x);
      }
    }
  }
  disconnect();
  wait();
}
