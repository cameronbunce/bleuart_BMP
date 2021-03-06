/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution

*********************************************************************
 This modification to the Bluefruit starter files is an attempt to 
 use the bluefruit app to poll data from a BMP sensor connected to the 
 Feather M0 using i2c

 
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>

#include "BluefruitConfig.h"

#include <Wire.h>
#include <Adafruit_BMP085.h>


/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//create the sensor instance
Adafruit_BMP085 bmp;
bool debug = 0    ; // global debug flag for serial output

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  if ( debug )
  {
  
	Serial.println(F("Adafruit Bluefruit Command Mode Example"));
	Serial.println(F("---------------------------------------"));
	Serial.print(F("Initialising the Bluefruit LE module: "));
  }

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  if(debug){Serial.println( F("OK!") );}

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    if(debug)
	{
		Serial.println(F("Performing a factory reset: "));
	}
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  ble.println("AT+GAPDEVNAME=Snake Finder");

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  if(debug)
  {
	Serial.println("Requesting Bluefruit info:");
  }
  /* Print Bluefruit information */
  ble.info();
  
  if(debug)
  {
	Serial.println(F("Starting BMP180 connection"));
  }
  if (!bmp.begin()){
	if(debug)
	{
		Serial.println(F("Could Not connect to BMP180 sensor."));
		Serial.println(F("Please check wiring and reboot."));
	}
    while (1){}
  }
  if(debug)
  {
	Serial.println(F("Connected to sensor."));
	Serial.print("Temperature = ");
	Serial.print(bmp.readTemperature());
	Serial.println(" *C");
		
	Serial.print("Pressure = ");
	Serial.print(bmp.readPressure());
	Serial.println(" Pa");
	Serial.println(F("*******************************"));
	Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
	Serial.println(F("Then Enter characters to send to Bluefruit"));
	Serial.println();
  }

  ble.verbose(false);  // debug info is a little annoying after this point!
  
  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  } 
  
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  // Check for user input
  char inputs[BUFSIZE+1];

  if ( getUserInput(inputs, BUFSIZE) )
  {
	if(debug)
	{
		// Send characters to Bluefruit
		Serial.print("[Send] ");
		Serial.println(inputs);
	}
    ble.print("AT+BLEUARTTX=");
    ble.println(inputs);

    // check response stastus
    if (! ble.waitForOK() ) {
		if(debug)
		{
			Serial.println(F("Failed to send?"));
		}
    }
  }

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  if(debug)
  {
	Serial.print(F("[Recv] ")); 
	Serial.println(ble.buffer);
  }
  if ( strcmp(ble.buffer, "get") == 0 )
  {
      WritePressure();
  }
  else if (strcmp(ble.buffer, "Get") == 0 )
  {
      WritePressure();
  }
  else if (strcmp(ble.buffer, "debug") ==0 )
  {
    debug=1;
    Serial.println(F("[Debug mode enabled from BLEUART]"));
  }
  else if (strcmp(ble.buffer, "quiet") == 0)
  {
    debug=0;
    Serial.println(F("[Debug mode disabled from BLEUART]"));
  }
  else if (strcmp(ble.buffer, "help") ==0)
  {
    printHelp();
  }
  else if (strcmp(ble.buffer, "name") ==0)
  {
    GAPDEVrenamer();
  }
  
  ble.waitForOK();
  
}


/**************************************************************************/
/*!
    @brief  Writes pressure information from sensor to everywhere
*/
/**************************************************************************/
void WritePressure()
{
  ble.print("AT+BLEUARTTX=Temperature = ");
  ble.print(((bmp.readTemperature()*1.8)+32));
  ble.println(" *F           ");
  ble.waitForOK();
  if(debug)
  {
	Serial.print("[SENT] Temperature = ");
	Serial.print(bmp.readTemperature());
	Serial.println(" *C");
  }
  ble.print("AT+BLEUARTTX=Pressure = ");
  ble.print(((float)bmp.readPressure()/3386.375258));
  ble.println(" inHg");
  ble.println();
  ble.waitForOK();
  if(debug)
  {
	Serial.print("[SENT] Pressure = ");
	Serial.print(bmp.readPressure());
	Serial.println(" Pa");
  }
}
void GAPDEVrenamer()
{
  /*something in this makes the whole thing hang
  bothPrintln("enter text to set device name");
  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  if(debug)
  {
  Serial.print(F("[Recv] ")); 
  Serial.println(ble.buffer);
  }
  ble.waitForOK();
  ble.println(sprintf("AT+GAPDEVNAME=", "%s", ble.buffer));
  */
}
void bothPrintln( char printthis[])
{
  ble.println(sprintf("AT+BLEUARTTX=", "%s", printthis));
  if(debug){Serial.println(printthis);}
}
void printHelp()
{
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=******************          "));
  ble.println(F("AT+BLEUARTTX=This is the help file for the BLE-UART interfaced BMP180 "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=Barometric pressure sensor built with components from Adafruit. "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=More information is available at the GitHub page for this project "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX= https://github.com/cameronbunce/bleuart_BMP "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX="));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=The current command set is limited, but includes the following:"));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=----------------------------"));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=help  - shows this help file  "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=get   - returns current temperature and pressure sensor readings  "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=debug - prints all BLE communication and diagnostics to USB/Serial  "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=quiet - toggles debug mode off  "));
  ble.waitForOK();
  ble.println(F("AT+BLEUARTTX=----- none of the commands take any arguments  "));
  ble.waitForOK();
  Serial.println(F("This is the help file for the BLE-UART interfaced BMP180"));
  Serial.println(F("Barometric pressure sensor built with components from Adafruit"));
  Serial.println(F("More information is available at the GitHub page for this project"));
  Serial.println(F("https://github.com/cameronbunce/bleuart_BMP"));
  Serial.println(F(""));
  Serial.println(F("current command set is limited, but includes the following:"));
  Serial.println(F("----------------------------"));
  Serial.println(F("help  - shows this help file"));
  Serial.println(F("get   - returns current temperature and pressure sensor readings"));
  Serial.println(F("debug - prints all BLE communication and diagnostics to USB/Serial"));
  Serial.println(F("quiet - toggles debug mode off"));
  Serial.println(F("----- none of the commands takes any arguments"));
}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
bool getUserInput(char buffer[], uint8_t maxSize)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!Serial.available()) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (Serial.available()) );

  return true;
}
