/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN D4

// Parameter 1 = aantal LEDs in de strip
// Parameter 2 = pin nummer
// Parameter 3 = RGB LED vlaggen, combineer indien nodig:
//   NEO_KHZ800  800 KHz bitstream (de meeste NeoPixel producten met WS2812 LEDs)
//   NEO_KHZ400  400 KHz (klassieke 'v1' (niet v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     RGB LED volgens GRB bitstream (de meeste NeoPixel produkten)
//   NEO_RGB     RGB LED volgens RGB bitstream (v1 FLORA pixels, niet v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "DJOAMERSFOORT";
const char* password = "l4anp4r7y";


const char* html = "<html>\n \
  <head> \
      <title>LedStrip</title> \
  </head> \
  <body>\n \
</style>\n \
    <a href='/gpio/1'>Aan</a>\n \
    <a href='/gpio/0'>Uit</a>\n \
  </body>\n \
</html>\n";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  strip.begin();
  strip.show(); // initialisatie van alle LEDs (resulteert in UIT zetten hier)
  allesUit();
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  client.print(html);
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  if (req.indexOf("/gpio/0") != -1)
    val = 0;
  else if (req.indexOf("/gpio/1") != -1)
    val = 1;
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  client.flush();
  client.stop();

  delay(1);
  Serial.println("Client disonnected");
  // Set leds according to the request
  if (val) {
    doeLeds();
  } else {
    allesUit();
  }
  
  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void doeLeds() {
  // Een paar voorbeelden:
  colorWipe(strip.Color(255, 0, 0), 50); // Rood
  colorWipe(strip.Color(0, 255, 0), 50); // Groen
  colorWipe(strip.Color(0, 0, 255), 50); // Blauw

  // Zogenaamde thearter chase voorbeeld ...
  theaterChase(strip.Color(127, 127, 127), 50); // Wit
  theaterChase(strip.Color(127,   0,   0), 50); // Rood
  theaterChase(strip.Color(  0,   0, 127), 50); // Blauw

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
}


void allesUit() {
  // Schakel LED na LED aan
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, 0);    
  }
  strip.show();
}

// Schakel LED na LED aan
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Iets andere variatie van de rainbow functie maar dan iets gelijkmatiger
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style kruipende LEDs.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //10 x doorlopen
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //zet elke 3de RGD LED aan
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //zet elke 3de RGB LED uit
      }
    }
  }
}

//Theatre-style kruipen met regenboog effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // gebruik alle 256 kleuren in het wiel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //zet elke 3de LED aan
        }
        strip.show();

        delay(wait);

        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //zet elke 3de LED uit
        }
    }
  }
}

// Voer een waarde van 0 tot 255 in voor berekening van kleur waarden.
// De kleuren zijn een r - g - b combinatie.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
