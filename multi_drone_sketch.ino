#include <WiFi.h>
#include <WiFiUdp.h>

// Drone Config 
const char * wifiName = "TELLO-XXXXXX"; // CHANGE
const char * wifiPswd = "";
IPAddress ipTello(192, 168, 10, 1);
const int udpPortTello = 8889;

const int LED_PIN = 5;
const int BUTTON_PIN = 13;

bool isConnected = false;
int isExecuting = false;

int buttonState = LOW;
int execCounter = 0;

WiFiUDP udp;

void setup()
{
  // Serial begin for debug
  Serial.begin(115200);

  // Setup gpio modes
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  delay(100);
  digitalWrite(LED_PIN, HIGH); // turn LED on
  
  buttonState = digitalRead(BUTTON_PIN);
  Serial.print("default button pin state: ");
  
  Serial.println(buttonState);

  delay(100);

  digitalWrite(LED_PIN, LOW); // turn LED off
  
  // run setup
  Serial.print("WiFi network ID: ");
  Serial.println(wifiName);
  Serial.print("WiFi password: ");
  Serial.println(wifiPswd);
  connectToWifi(wifiName, wifiPswd); //connect to Tello's WiFi network connection
}

void loop()
{
  int tempRead = digitalRead(BUTTON_PIN);
  if(buttonState != tempRead)
  { // This is to prevent buttonState from being written to more than once.
    buttonState = tempRead;
    
    Serial.print("BUTTON STATE: ");
    Serial.println(buttonState);
  }
  
  if (buttonState == HIGH &&
      isConnected == true &&
      isExecuting == false) 
  { // When button is pushed and connected and not yet executing code
    execCounter++;
    Serial.print("Exec Counter: ");
    Serial.println(execCounter);
    isExecuting = true;
    
    Serial.println("Button pressed, sending commands");
    sendCommands();
  }
    
  isExecuting = false;
}

void sendCommands()
{ // start UDP stream
  Serial.print("ip of tello: ");
  Serial.println(ipTello.toString());
  Serial.print("port of tello:  ");
  Serial.println(udpPortTello);
  
  // Enter command mode
  udpSendMessage(ipTello, "command", udpPortTello);
  delay(5000);

  // List commands for drone to execute through Tello's API
  Serial.println("initiating take off"); 
  udpSendMessage(ipTello, "takeoff", udpPortTello);

  Serial.println("delaying 5 seconds");
  delay(5000);

  // Spin clockwise
  udpSendMessage(ipTello, "cw 360", udpPortTello);

  delay(10000);
  Serial.println("other way");
  // Spin counter clockwise
  udpSendMessage(ipTello, "ccw 360", udpPortTello);

  Serial.println("delaying 10 seconds");
  delay(10000);

  Serial.println("iniating landing.");
  udpSendMessage(ipTello, "land", udpPortTello);
}

// Borrowed from GitHub
bool udpSendMessage(IPAddress ipAddr, String udpMsg, int udpPort) {
  /** WiFiUDP class for creating UDP communication */
  WiFiUDP udpClientServer;

  // Start UDP client for sending packets
  int connOK = udpClientServer.begin(udpPort);

  if (connOK == 0) {
    Serial.println("UDP could not get socket");
    return false;
  }
  udpClientServer.begin(udpPort);
  int beginOK = udpClientServer.beginPacket(ipAddr, udpPort);

  if (beginOK == 0) { // Problem occured!
    udpClientServer.stop();
    Serial.println("UDP connection failed");
    return false;
  }
  int bytesSent = udpClientServer.print(udpMsg);
  if (bytesSent == udpMsg.length()) {
    Serial.println(udpMsg + " was sent in " + String(bytesSent) + " bytes");
    udpClientServer.endPacket();
    udpClientServer.stop();
    return true;
  } else {
    Serial.println("FAILED to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes");
    udpClientServer.endPacket();
    udpClientServer.stop();
    return false;
  }
}

void connectToWifi(const char * ssid, const char *password)
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //delete previous configs
  WiFi.disconnect(true);
  
  // register event handler
  WiFi.onEvent(wifiEvent);

  WiFi.begin(ssid, password);
  Serial.print("Waiting for WiFi connection to ");
  Serial.println(ssid);
}

// Event Handler:
void wifiEvent(WiFiEvent_t event)
{
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //connected
      Serial.println("Wifi connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

      //initialize UDP
      udp.begin(WiFi.localIP(), udpPortTello);
      isConnected = true;
      digitalWrite(LED_PIN, HIGH); // turn LED on
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      //disconnected
      Serial.print("WiFi is waiting for connection to ");
      Serial.println(wifiName);
      isConnected = false;
      digitalWrite(LED_PIN, LOW); // turn LED off
      break;
  }
}
