//Git bash Test

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


//------------------  WiFi Credentials  ------------------
char wifissid[] = "pizerowap";
char wifipassword[] = "12345678abc";

//------------------  UDP Variables  ------------------
WiFiUDP Udp;
unsigned int localUdpPort = 5006;
#define MAXINCOMINGLENGTH 1001
char incomingPacket[MAXINCOMINGLENGTH];
String replyString = "";
#define REPLYLENGTH 40
char replyPacket[REPLYLENGTH] = "Hi there got";
int packetTracker = 0;

//------------------  Timing Variables  ------------------
unsigned long currentmillis = 0;
unsigned long lastmillis = 0;
unsigned long lastlooptime = 0;

//------------------  Information LED  ------------------
unsigned long ledholdtime = 1000; //Time to flash LED
boolean statusledstate = false;
int statusled = 16;

//------------------  LED Matrix  ------------------

#define MAXLEDMATRIXWIDTH 40
#define MAXLEDMATRIXHEIGTH 40
byte ledFrameBuffer[MAXLEDMATRIXWIDTH * MAXLEDMATRIXHEIGTH * 3] = {102}; //Width * Height * RGB
int pixelTracker = 0; //Keeps Track of the current pixel between UDP Packets
boolean newFrameReady = false;
int frameWidth = 0;
int frameHeigth = 0;

//------------------  Debug  ------------------
int loopspersec = 0;


//------------------  Setup  ------------------
void setup() {
  Serial.begin(115200);
  Serial.println();

  //WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid, wifipassword);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  delay(10);

  Udp.begin(localUdpPort); //open UDP Port
  pinMode(statusled, OUTPUT);
}

//------------------  Loop  ------------------
void loop() {

  static boolean onetime = true;
  if (onetime) {
    onetime = false;
    //printLEDFrameBuffer();
  }
  currentmillis = millis();

  int packetSize = Udp.parsePacket(); //Check if we have new data at our UDP port
  if (packetSize)
  {
    statusledstate = true; //Turn statusled on
    lastmillis = currentmillis; //and update time we turn the led on
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, MAXINCOMINGLENGTH - 1);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    handleUDP(len);
    //Serial.printf("UDP packet contents: %s\n", incomingPacket);

    //Send UDP Reply Packet
    Udp.beginPacket(Udp.remoteIP(), 5006);
    replyString = String("Hi there! Received ");
    replyString = String(replyString + packetSize);
    replyString = String(replyString + " bytes");
    replyString.toCharArray(replyPacket, REPLYLENGTH);
    Udp.write(replyPacket);
    Udp.endPacket();

  }

  //Loops per second counter
  loopspersec++;
  if ((currentmillis - lastlooptime) > 1000) {
    //Serial.print("Loops per Second: ");
    //Serial.println(loopspersec);
    lastlooptime = currentmillis;
    loopspersec = 0;
  }

  //Turn Statusled off after ledholdtime
  if ((currentmillis - lastmillis) > ledholdtime) {
    statusledstate = false;
  }

  digitalWrite(statusled, !statusledstate); //Leds on NodeMCU are active low
}

int handleUDP(int packetlength) {
  switch (incomingPacket[0]) {
    case 0x01: //LED Ball Inventory
      {

      }
      break;

    case 0x03: //Receiving new frame data
      {
        Serial.println("Receiving a new Package");
        int framePacketNo = incomingPacket[1]; //Pack Number
        int framePacketCount = incomingPacket[2]; //Number of Packets for the whole Frame
        frameWidth = incomingPacket[3]; //Width of total Transmitted Frame
        frameHeigth = incomingPacket[4]; //Heigth of total Transmitted Frame
        int pixelCountPacket = incomingPacket[5]; //Amount of RGB Pixels transmitted in this packet

        if (packetlength < pixelCountPacket * 3 + 6) {
          Serial.println("Not enogth data in this Frame");
          return -1;
        }

        if (framePacketNo > framePacketCount) { //We have more Packets than we should
          Serial.println("Too many Packages");
          newFrameReady = false;
          packetTracker = 0;
          pixelTracker = 0;
          return -1;
        }

        if (framePacketNo == 1) { //We are receving a new Frame
          Serial.println("Start of a new Frame");
          newFrameReady = false;
          pixelTracker = 0;
          packetTracker = 0;
        }

        if (!(packetTracker + 1 == framePacketNo)) {
          Serial.println("A Package got lost");
          return -1;
        }

        for (int i = 0; i < pixelCountPacket * 3; i++) { //Times 3 for RGB-Channels
          ledFrameBuffer[pixelTracker] = incomingPacket[6 + i];
          pixelTracker++;
        }
        packetTracker++;

        if ((packetTracker == framePacketCount) && (pixelTracker == frameHeigth * frameWidth * 3)) { //We received data for the whole Frame
          newFrameReady = true;          
          Serial.println("Received data for the whole Frame");
          printLEDFrameBuffer();
        }
      }
      break;

    default:
      {
        Serial.println("Unknown Command");
        break;
      }
  }
  return 1;
}

int printLEDFrameBuffer() {
  for (int i = 0; i < frameHeigth; i++) { //Rows
    for (int j = 0; j < frameWidth * 3; j++) { //Column
      Serial.print(ledFrameBuffer[i * frameWidth * 3 + j], HEX);
      Serial.print(":");
    }
    Serial.println("");
  }
  return 1;
}
