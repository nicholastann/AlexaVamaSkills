#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
//#include <HttpClient.h>
//#include <aJSON.h>
#include <WiFi.h>
#include <SPI.h>

WiFiClient client;
char ssid[] = "Erik";
char password[] = "password";
int keyIndex = 0;
int lastStatus = 0;
const int buttonPin = PUSH2;     // the number of the pushbutton pin
const int butt2 = PUSH1;
const int ledPin =  RED_LED;      // the number of the LED pin
const int LED = GREEN_LED;
int butt2state = 0;
int change = 0;
int buttonState = 0;         // variable for reading the pushbutton status
int status = 3;

// Name address for Open Weather Map API
const char* server = "vama.herokuapp.com";

// Replace with your url for other devices: change id=3 for switch, id=2 for lock
const char* resource = "/api/view.php?id=3";
const char* poster = "/api/update.php";

const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// The type of data that we want to extract from the page
struct clientData {
  char name[12];
  char Status[8];
  char channel[12];
  char volume[12];
};

void setup() {
   //check for wifi connection
  Serial.begin(115200);      // initialize serial communication
  pinMode(RED_LED, OUTPUT);      // set the LED pin mode
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while waiting to connect
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nYou're connected to the network");
  pinMode(9, OUTPUT);//phase
  pinMode(5, OUTPUT);//enable 
  pinMode(7, INPUT);//"locked" status = 0 input 
  pinMode(8, INPUT);//"unlocked" status = 1 input
  delay(1000);
  
}

void loop() {
  if(connect(server)) {
    if(sendRequest(server, resource) && skipResponseHeaders()) {
      clientData x; //initialize a clientData object named x
      if(readReponseContent(&x)) {
      printclientData(&x);
  }
  disconnect();
  wait();
  Serial.print(digitalRead(8));
  Serial.print(lastStatus);
  Serial.println(digitalRead(7));
  //state 1 8 HIGH
  if(digitalRead(7)==HIGH){
    if(lastStatus==1){
      //Serial.print("post 0");
      if(connect(server)){
      if(sendChange(server, poster,&x,0)){
        Serial.println("success");
          }
      }
      disconnect();
    }
  }
  if(digitalRead(8)==HIGH){
    if(lastStatus==0){
      //Serial.print("post 1");
      if(connect(server)){
      if(sendChange(server, poster,&x,1)){
        Serial.println("success");
          }
      }
      disconnect();
    }
  }
      
  /*
  if(connect(server)){
      if(sendChange(server, poster,&x)){
        Serial.println("success");
    }
  }
  disconnect();
  */
  //wait();
}
}
}

// Open connection to the HTTP server
bool connect(const char* hostName) {
  //Serial.print("Connect to ");
  //Serial.println(hostName);

  bool ok = client.connect(hostName, 80);

  //Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
  //Serial.print("GET ");
  //Serial.println(resource);

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
  DynamicJsonDocument doc(1000);
  DeserializationError error = deserializeJson(doc,client);
  if (error) {
    Serial.println("JSON parsing failed!");
    return false;
  }
  const char* name = doc["name"];
  const char* Status = doc["status"];
  const char* channel = doc["channel"];
  const char* volume = doc["volume"];
  strcpy(clientData->name, doc["name"]);
  strcpy(clientData->Status, doc["status"]);
  strcpy(clientData->channel, doc["channel"]);
  strcpy(clientData->volume, doc["volume"]);
  
  
  return true;
}

// Print the data extracted from the JSON
void printclientData(struct clientData* clientData) {
  /*
  Serial.print("Name = ");
  Serial.println(clientData->name);
  Serial.print("Status = ");
  Serial.println(clientData->Status);
  Serial.print("lastStatus = ");
  Serial.println(lastStatus);
  */
  if (atoi(clientData->Status)==0){                     //if the server status of the appliance is off or locked 
    if (atoi(clientData->Status) != lastStatus){        //if the server status does not match the status of the appliance currently
      while (digitalRead(7) == LOW){                    // loop until the system reaches a new state
        digitalWrite(9, HIGH);                          // phase set to high to turn motor clockwise
        digitalWrite(5, HIGH);                          // enable set to high to enable motor
        
      }
      digitalWrite(5,LOW);                              //enable set to low to brake the motor
      change = change +1;
      Serial.print(change);
      Serial.print(", ");
      Serial.print(clientData->Status);
      Serial.print(", ");
      Serial.print(clientData->channel);
      Serial.print(", ");
      Serial.print(clientData->volume);
      Serial.println();
      lastStatus = 0;     
    }
                                    //update status of appliance
  }
  else if(atoi(clientData->Status)==1){                 //if the server status is on or unlocked
    if (atoi(clientData->Status) != lastStatus){        //if the server status does not match the status of the appliance currently
    while (digitalRead(8) == LOW){                      //loop until system reaches a new state
      digitalWrite(9,LOW);                              //phase set to low to turn motor counterclockwise
      digitalWrite(5,HIGH);                             //enable set to high to enable motor
      
    }
    digitalWrite(5,LOW);                                //enable set to low to brake the motor
    change = change +1;
    Serial.print(change);
    Serial.print(", ");
    Serial.print(clientData->Status);
    //Serial.prinln();
    Serial.print(", ");
    Serial.print(clientData->channel);
    Serial.print(", ");
    Serial.print(clientData->volume);
    Serial.println();
    lastStatus =1; 
    
  }
  //lastStatus =1;                                        //update status of appliance
  }
  else{
    change = change +1;
    Serial.print(change);
    Serial.print(", ");
    Serial.print(clientData->Status);
    Serial.print(", ");
    Serial.print(clientData->channel);
    Serial.print(", ");
    Serial.print(clientData->volume);
    Serial.println();
    lastStatus = atoi(clientData->Status);
  }
}
bool sendChange(const char* host, const char* poster, struct clientData* clientData, int state) {
  DynamicJsonDocument doc(1000);
  doc["id"] = 3;
  doc["name"] = "Bedroom Light";
  doc["type"] = "light";
  if(state==0){
    doc["status"] = "0";
  }
  if(state==1){
    doc["status"] = "1";
  }
  //doc["status"] = state;
  doc["channel"] = "";
  doc["volume"] = "";

  serializeJsonPretty(doc,Serial);
  client.println("POST /api/update.php HTTP/1.0");
  client.println("Host: vama.herokuapp.com");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(measureJson(doc));
  
  client.println();
  serializeJson(doc,client);
  Serial.println(client);
  return true;
}

// Close the connection with the HTTP server
void disconnect() {
  //Serial.println("Disconnect");
  client.stop();
}

// Pause for a 1 minute
void wait() {
  //Serial.println("Wait 5 seconds");
  delay(0500);
}
