/**
 * Gets coffee data from the Arduino.
 * Author: Eric Weinstein <eric.q.weinstein@gmail.com>
 * See: https://learn.sparkfun.com/tutorials/
 *   wifly-shield-hookup-guide/setting-up-a-simple-server;
 *   based on work by Joel Bartlett.
 * See: http://kronosapiens.github.io/;
 *   based on work by Daniel Kronovet.
 */

#include <SPI.h>
#include "WiFly.h"

// The RGB LED is attached to pins 2, 4, & 6.
static const int RED   = 2;
static const int GREEN = 4;
static const int BLUE  = 6;

// The pressure sensor is connected to pin A0.
static const int SENSOR = A0;

// We'll take several measurements and average them.
int reading1 = 0;
int reading2 = 0;
int reading3 = 0;
int reading4 = 0;
int reading5 = 0;
int reading  = 0;

// Initialize threshholds for the coffee pot.
static const int FULL   = 800;
static const int HALF   = 775;
static const int EMPTY  = 700;
static const int NODATA = 100;

// Listen on port 80.
static const int PORT = 80;

// We'll want a static IP address.
static const char *IP_ADDRESS = "flit-ip-address-here";

// The LAN we want Flit to join.
static const char *WIFI_NETWORK = "your-wifi-network-here";

// Initialize the server.
WiFlyServer server(PORT);

void setup()
{
  // Set up the LED.
  initializeLED();

  // Set the pressure sensor as input.
  pinMode(SENSOR, INPUT);

  // Start chattin' up the WiFly.
  WiFly.begin();
  WiFly.join(WIFI_NETWORK);

  // Begin serial communication at 9600 baud.
  Serial.begin(9600);

  // Set the IP address.
  setIP(IP_ADDRESS);

  // Ensure the IP address is correct
  // (uncomment to view serial output).
  // Serial.print("IP: ");
  // Serial.println(WiFly.ip());

  // End serial communication.
  Serial.end();

  // Start 'er up.
  server.begin();
}

void loop()
{
  // Ensure server is ready.
  WiFlyClient client = server.available();

  // Read from the pressure sensor.
  reading1 = analogRead(SENSOR);
  delay(500);

  reading2 = analogRead(SENSOR);
  delay(500);

  reading3 = analogRead(SENSOR);
  delay(500);

  reading4 = analogRead(SENSOR);
  delay(500);

  reading5 = analogRead(SENSOR);

  reading = ((reading1 + reading2 + reading3 + reading4 + reading5) / 5);

  // Set the LED color.
  setLED(reading);

  if (client)
  {
    // HTTP requests end with a blank line.
    boolean currentLineIsBlank = true;
    boolean endOfCode          = true;
    char c;

    while (client.connected())
    {
      if (client.available())
      {
        c = client.read();
        delay(10);
        // Uncomment this line to see the HTTP response.
        // Serial.print(c);

        // If we've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so we can send a reply.
        if (!client.available())
        {
          endOfCode = true;
        }
        else if (c == '\n')
        {
          // We're starting a new line!
          currentLineIsBlank = true;
        } else if (c != '\r')
        {
          // We've gotten a character on the current line.
          currentLineIsBlank = false;
          endOfCode          = false;
        }

        if ((c == '\n' && currentLineIsBlank && !client.available()) || endOfCode)
        {
          // Once the page has been refreshed, we're no
          // longer on the first loop through the program.
          endOfCode = false;

          // Send a standard HTTP response header.
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");

          client.println();

          // Send the last reading from the pressure sensor.
          client.print("[{\"state\": ");
          client.print(reading);
          client.println("}]");

          delay(500);
          break;
        }
      }
    }

    // Give the browser time to receive the data.
    delay(100);
    client.flush();
    client.stop();
  }
}

void initializeLED()
{
  // Set LED pins as outputs.
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  // Ensure the LED is off.
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
}

void setIP(const char *address)
{
  // Enter command mode.
  SpiSerial.begin(9600);

  // Set static IP address.
  SpiSerial.println("set ip dhcp 0");
  delay(500);

  SpiSerial.print("set ip address ");
  SpiSerial.println(address);
  delay(500);

  // Exit command mode.
  SpiSerial.println("");
  SpiSerial.println("exit");
}

void setLED(int coffeeLevel)
{
  if (coffeeLevel <= NODATA)
  {
    // Off the sensor; LED is off.
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
  }
  else if (coffeeLevel <= EMPTY)
  {
    // Coffee is low; LED is red.
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
  }
  else if (coffeeLevel <= HALF)
  {
    // About half a pot; LED is blue.
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, HIGH);
  }
  else
  {
    // There's plenty of coffee! LED is green.
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, LOW);
  }
}
