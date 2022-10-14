/*
  RoboBuni v3 firmware.
  
  Chip: Esp8266

  Uses websocket to receive instructions from the serverside component to move the robot.

*/

#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <string.h>

const char* ssid = "alex_dd312"; //Enter SSID
const char* password = "zzzatomic"; //Enter Password
const char* websockets_server_host = "wss://robi2.herokuapp.com/robi"; //Enter server adress

const int M1P1 = 5;  //D1
const int M1P2 = 4;  //D2
const int M2P1 = 14; //D5
const int M2P2 = 12; //D6

const int MAX_PIN_PWR = 255;

using namespace websockets;

WebsocketsClient client;
void setup() {
    
    initComms();
    
    initMotorPins();
}



void initMotorPins(){
  pinMode(M1P1, OUTPUT);
  pinMode(M1P2, OUTPUT);
  pinMode(M2P1, OUTPUT);
  pinMode(M2P2, OUTPUT);

}


void initComms(){
  Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi.");
    
    
   connectToWS();

}


void connectToWS(){
  
  Serial.printf("Connecting to websocket (%s)...",websockets_server_host);
   // try to connect to Websockets server
   
    bool connected = client.connect(websockets_server_host);
    if(connected) {
        Serial.println("Connected!");
        client.send("ping");
        
    } else {
        Serial.println("Not Connected!");
    }
    
    // run callback when messages are received
    client.onMessage([&](WebsocketsMessage message) {
        Serial.print("Got Message: ");
        Serial.println(message.data());
        
       char command[2000];
       strncpy(command, message.data().c_str(), 2000);
       
       interpretCommandSequence(command);
       
    });
}

void interpretCommandSequence(char * command)
{

  char *p = command;
  
  char cmd[200];
  
  char *tok;
  
  while ( (tok = strtok_r(p,"|", &p))!=NULL )
  {  
     strncpy(cmd, tok, 200);
     interpretCommand(cmd);
  }
  
}


void interpretCommand(char * command){

  Serial.printf("interpreting command:%s\n", command);

  char * saveptr;
  char * pch = strtok_r(command,",", &saveptr);
  
  char *cmd = pch;

  
  if (0==strcmp(cmd, "motors-set")){

                pch = strtok_r(NULL, ",", &saveptr);
                
                int port = 0;
                
                while (pch != NULL) 
                {
                  
                  int pwr = atoi(pch);
                  
                  if (pwr > MAX_PIN_PWR)
                    pwr = MAX_PIN_PWR;
              
                  if (pwr < 0){
                     pwr = 0;
                  }
                  
                  switch(port){
                    case 0:
                      analogWrite(M1P1, pwr);
                      break;
                    case 1:
                      analogWrite(M1P2, pwr);
                      break;
                    case 2:
                      analogWrite(M2P1, pwr);
                      break;
                    case 3:
                      analogWrite(M2P2, pwr);
                      break;
                  }
                  
                  pch = strtok_r(NULL, ",", &saveptr);
                  port++;
                }
  }

   if (0==strcmp(cmd, "delay")){
      
       pch = strtok_r(NULL, ",", &saveptr);

       if (pch!=NULL)
       {
        int interval = atoi(pch);
        
        delay(interval);
       }
   }
  
}

int loopCount=0;
void loop() {
    // let the websockets client check for incoming messages
    if(client.available()) {
        client.poll();
    }
    else
    {
      connectToWS();
    }
    delay(50);
    
    loopCount++;
    if (loopCount == 200) {
      client.send("ping");
      loopCount=0;
    }
}
