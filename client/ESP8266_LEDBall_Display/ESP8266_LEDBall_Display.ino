#define FASTLED_ALLOW_INTERRUPTS 0

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define FASTLED_ESP8266_DMA
#include <FastLED.h>

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
unsigned long lastMatrixUpdate = 0;

//------------------  Information LED  ------------------
unsigned long ledholdtime = 1000; //Time to flash LED
boolean statusledstate = false;
int statusled = 16;

//------------------  LED Matrix  ------------------
#define MATRIXPIN 3
#define LEDMATRIXWIDTH 16
#define LEDMATRIXHEIGHT 25
#define LEDCOUNT LEDMATRIXWIDTH * LEDMATRIXHEIGHT
#define ZICKZACK
CRGB leds[LEDCOUNT];
byte ledFrameBuffer[LEDCOUNT * 3] = {102}; //Width * Height * RGB
byte Buffer2D[LEDMATRIXWIDTH][LEDMATRIXHEIGHT][3];
int pixelTracker = 0; //Keeps Track of the current pixel between UDP Packets
boolean newFrameReady = false;
int frameWidth = 0;
int frameHeight = 0;
int maxFPS = 60;
int matrixUpdatePeriod = int(1.0 / maxFPS * 1000);
//------------------  Debug  ------------------
int loopspersec = 0;
extern const uint8_t gamma8[];

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

  pinMode(statusled, OUTPUT); //Debug Status LED

  LEDS.addLeds<WS2812, MATRIXPIN, RGB>(leds, LEDCOUNT);
  LEDS.setBrightness(255);
}

//------------------  Loop  ------------------
void loop() {

  static boolean onetime = true;
  if (onetime) {
    onetime = false;
    //printLEDFrameBuffer();
  }
  currentmillis = millis();



  //updateMatrix();
  simpleUpdateMatrix();

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
    Serial.print("Loops per Second: ");
    Serial.println(loopspersec);
    lastlooptime = currentmillis;
    loopspersec = 0;
  }

  //Turn Statusled off after ledholdtime
  if ((currentmillis - lastmillis) > ledholdtime) {
    statusledstate = false;
  }

  digitalWrite(statusled, !statusledstate); //Leds on NodeMCU are active low
}

void simpleUpdateMatrix() {
  if (newFrameReady) {
    newFrameReady = false;
    for (int x = 0; x < LEDMATRIXWIDTH; x++) { //Rows
      for (int y = 0; y < LEDMATRIXHEIGHT; y++) { //Column
        if (x % 2 == 0) { //even Rows
          leds[x * LEDMATRIXHEIGHT + y][0] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3]]; // RED
          leds[x * LEDMATRIXHEIGHT + y][1] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3 + 1]]; // GREEN
          leds[x * LEDMATRIXHEIGHT + y][2] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3 + 2]]; // BLUE
        }
        else {
          int reverseY = LEDMATRIXHEIGHT - y - 1;
          leds[x * LEDMATRIXHEIGHT + reverseY][0] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3]]; // RED
          leds[x * LEDMATRIXHEIGHT + reverseY][1] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3 + 1]]; // GREEN
          leds[x * LEDMATRIXHEIGHT + reverseY][2] = gamma8[ledFrameBuffer[x * 3 + LEDMATRIXWIDTH * y * 3 + 2]]; // BLUE
        }
      }
    }
  }

  if ((currentmillis - lastMatrixUpdate) > matrixUpdatePeriod) {
    lastMatrixUpdate = currentmillis;
    //Serial.println("LED Update");
    FastLED.show();
  }
}

int handleUDP(int packetlength) {
  switch (incomingPacket[0]) {
    case 0x01: //LED Ball Inventory
      {

      }
      break;

    case 0x03: //Receiving new frame data
      {
        //Serial.println("Receiving a new Package");
        int framePacketNo = incomingPacket[1]; //Pack Number
        int framePacketCount = incomingPacket[2]; //Number of Packets for the whole Frame
        frameWidth = incomingPacket[3]; //Width of total Transmitted Frame
        frameHeight = incomingPacket[4]; //Heigth of total Transmitted Frame
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
          //Serial.println("Start of a new Frame");
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

        if ((packetTracker == framePacketCount) && (pixelTracker == frameHeight * frameWidth * 3)) { //We received data for the whole Frame
          newFrameReady = true;
          //Serial.println("Received data for the whole Frame");
          //printLEDFrameBuffer();
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

void printLEDFrameBuffer() {
  for (int i = 0; i < frameHeight; i++) { //Rows
    for (int j = 0; j < frameWidth * 3; j++) { //Column
      Serial.print(ledFrameBuffer[i * frameWidth * 3 + j], HEX);
      Serial.print(":");
    }
    Serial.println("");
  }
}


const uint8_t gamma8[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
  2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
  5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

