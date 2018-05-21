// Prototype Code for BeeHive Datalogger
// Modified by Frank Grey

// Required Libraries
#include <DHT.h> // Used for DHT22 Temp & Humidity Sensors
#include <OneWire.h> // Used for Dallas Temp Sensors
#include <DallasTemperature.h> // Used for Dallas Temp Sensors
#include <Wire.h> // Used for I2C connection for the clock
#include <SPI.h> // Used for SPI connection for the SD card
#include <SD.h> // Used for interacting with the SD card

// *******************DHT Sensor***************************

#define DHTPIN 2     // Digital pin that DHT sensor is connected to

// ********************Dallas Temp Sensors******************

#define ONE_WIRE_BUS 6 // Data wire of Dallas Temp Sensors are plugged into port 6 on the Arduino
#define TEMPERATURE_PRECISION 9

// ***********************RTC********************************

// Define I2C pins
// Make sure to connect a 10k resister to 5v for SCL and SDA individually as this is required for I2C
#define SCL A5 // Clock line for I2C
#define SDA A4 // Data line for I2C
//Define Clock address
#define clockAddr 0b1101000 // The address for the clock is binary 1101000, 0b signifies a binary number
// Define the time
#define MONTH 1 // The month to set the clock to
#define DATE 1 // The date to set the clock to
#define YEAR 18 // The year to set the clock to (00-99)
#define DOW 1 // Day of the week to set the clock to
#define HOURS 00 // Hour to set the clock to (in 24hr clock)
#define MINUTES 00 // Minute to set the clock to
#define SECONDS 00 // Second to set the clock to

// ********************SD Card Datalogger**************************

// Define SPI pins
#define SD_CS 10 // SD card chip select pin for SPI
#define MOSI 11 // Master Out Slave In pin for SPI
#define MISO 12 // Master In Slave Out pin for SPI
#define SCK 13 // Serial Clock pin

void SetupClock(byte month, byte date, byte year, byte dow, byte hour, byte minute, byte second); // Set the time for the clock
String GetDate(); // Returns a string of the date


bool sd = true; // Defaults to having an SD card inserted, if SD cannot be initalized then it will be ignored

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
//DeviceAddress insideThermometer, outsideThermometer;

// Assign address manually. The addresses below will beed to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)
DeviceAddress insideThermometer = { 0x28, 0xFF, 0x64, 0xB5, 0x33, 0x18, 0x01, 0xA8 };
DeviceAddress outsideThermometer   = { 0x28, 0xFF, 0x51, 0x83, 0x33, 0x18, 0x01, 0xBF };

// ******************DHT*********************
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

//****************Sound & Light************************

#define Light A0 // Set A0 as light level input
#define Sound A1 // Set A1 as sound level input 

void setup() {
  // Initalize communication protocols
  Serial.begin(9600);
  Serial.println("Beehive Datalogger: by Frank Grey");
  Wire.begin(); // Initialize I2C
  SetupClock(MONTH, DATE, YEAR, DOW, HOURS, MINUTES, SECONDS); // Set the clock date, run this once and then reupload with it commented out
  // If left uncommented, the clock will always start at that specific date when the arduino gets rebooted

  SPI.begin(); // Initialize SPI
  if (!SD.begin(SD_CS)) // Initialize SD
  {
    Serial.println("SD failed to initialize!"); // If it couldn't be initialized, then print an error message
    sd = false;
  }

  pinMode(SD_CS, OUTPUT); // Set the SD card chip select pin as an output
  
  dht.begin();
  sensors.begin();
  // locate devices on the bus
  Serial.print("Locating OneWire devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).

  // method 1: by index
  // if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  // if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order

  // Must be called before search()
  // oneWire.reset_search();
  // assigns the first address found to insideThermometer
  // if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");
  // assigns the seconds address found to outsideThermometer
  // if (!oneWire.search(outsideThermometer)) Serial.println("Unable to find address for outsideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(outsideThermometer);
  Serial.println();

  // set the resolution to 9 bit per device
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(outsideThermometer), DEC);
  Serial.println();
}

void loop() {
  String date = GetDate(); // Get the date from the clock in text form

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  // Serial.print("Hive Humidity: ");
  // Serial.print(h);
  // Serial.print(" %\t");
  // Serial.print("Hive Temperature: ");
  // Serial.print(t);
  // Serial.print(" *C ");
  // Serial.print(f);
  // Serial.println(" *F\t");

  // Serial.print("Heat index: ");
  // Serial.print(hic);
  // Serial.print(" *C ");
  // Serial.print(hif);
  // Serial.println(" *F");
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  // Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  // Serial.println("DONE");

  // print the device information
  // printData(insideThermometer);
  // printData(outsideThermometer);

  float light = analogRead(Light);
  float sound = analogRead(Sound);

  // Serial.print("All sensor readings: ");
  Serial.println(CSVData(date,h,f,sensors.getTempFByIndex(0),sensors.getTempFByIndex(1),light,sound));
  

  if (sd) // If we have an sd card
  {
    File sdData = SD.open("data.txt", FILE_WRITE); // open data.txt so that we can write to it
    if (sdData)
    {
      sdData.println(CSVData(date,h,f,sensors.getTempFByIndex(0),sensors.getTempFByIndex(1),light,sound)); // Print the date to the data.txt on the sd card
      sdData.close(); // Close the file to save the changes
    }
    else
    {
      Serial.println("Couldn't open file from SD card"); // If we couldn't open the file on the sd card, print an error

      //delay(2000);
      return;
    }
  }


  // Wait a few seconds between measurements.
  delay(600000);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  //Serial.print("Temp C: ");
  //Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

byte BCDtoDecimal(byte bcd) // Converts a 1 byte binary coded decimal number to its equivalent decimal number
{
  byte tens = (bcd & 0b11110000) >> 4; // Get the tens digit
  byte ones = bcd & 0b00001111; // Get the ones digit
  byte result = (tens * 10) + ones; // Add them to get the decimal equivalent
  return result;
}

byte DecimalToBCD(byte decimal)
{
  byte tens, ones, result;
  if (decimal >= 10)
  {
    tens = decimal / 10; // Get the value in the tens place
    ones = decimal - (tens * 10); // Get the value in the ones place
    tens = tens << 4; // Shift the binary value of the tens place into the tens position for BCD

  }
  else
  {
    tens = 0;
    ones = decimal;
  }

  result = tens | ones; // or both values together to get the complete BCD value
  return result;
}

byte GetOneByte(byte addr, byte reg) // Since all clock data is only 1 byte long, we only need to request 1 byte of data at a time
// addr is the address of the device and reg is the register that the data is held in
{
  Wire.beginTransmission(addr); // Begin communication with the device
  Wire.write(reg); // Specify which register to retrive date from
  Wire.endTransmission(); // Stop sending to the device

  Wire.requestFrom(addr, 1); // Request 1 byte of data from the device
  while (Wire.available() == 0); // Wait until there is data being sent
  byte data = Wire.read(); // Read the data being recived

  return data;
}


void GetClockTime(byte data[7]) // Uses a 7 byte array to store the clock data
{
  // From 0-6, the order is Year, Month, Date, Day of the week, Hours, Minutes, Seconds
  data[0] = BCDtoDecimal(GetOneByte(clockAddr, 0x06)); // Get Year and convert it to decimal
  data[1] = BCDtoDecimal(GetOneByte(clockAddr, 0x05)); // Get Month and convert it to decimal
  data[2] = BCDtoDecimal(GetOneByte(clockAddr, 0x04)); // Get Date and convert it to decimal
  data[3] = BCDtoDecimal(GetOneByte(clockAddr, 0x03)); // Get Day of the week and convert it to decimal
  data[4] = BCDtoDecimal(GetOneByte(clockAddr, 0x02)); // Get Hours and convert it to decimal
  data[5] = BCDtoDecimal(GetOneByte(clockAddr, 0x01)); // Get Minutes and convert it to decimal
  data[6] = BCDtoDecimal(GetOneByte(clockAddr, 0x00)); // Get Seconds and convert it to decimal
}

String GetDate() // Returns a string of the date
{
  byte clockData[7]; // Create an array to hold the data from the clock
  GetClockTime(clockData); // Get the data from the clock

  String date;
  for (byte c = 0; c < 7 ; c++) // For each section of data...
  {
    date = date + String(clockData[c]); // Add each part onto the end of date, after being converted from BCD to decimal
    if (c != 6) // If it isn't the last part of the data, put a ':' after to separate values
    {
      date = date + ':';
    }
  }
  return date;
}

void WriteI2C(byte addr, byte reg, byte data) // Writes data to the register of the device
{
  Wire.beginTransmission(addr); // Begin communication with the device
  Wire.write(reg); // Specify the register to write to
  Wire.write(data); // Write the data
  Wire.endTransmission(); // Stop sending to the device
}

void SetupClock(byte month, byte date, byte year, byte dow, byte hour, byte minute, byte second) // Set the time for the clock
{
  WriteI2C(clockAddr, 0x06, DecimalToBCD(year)); // Write the year to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x05, DecimalToBCD(month)); // Write the month to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x04, DecimalToBCD(date)); // Write the date to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x03, DecimalToBCD(dow)); // Write the day of the week to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x02, DecimalToBCD(hour)); // Write the hour to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x01, DecimalToBCD(minute)); // Write the minute to the clock after being converted to BCD
  WriteI2C(clockAddr, 0x00, DecimalToBCD(second)); // Write the second to the clock after being converted to BCD
}

String CSVData(String date, float h, float f, float insideTemp, float outsideTemp, float light, float sound)
{
  String dataStream;
  dataStream = date + ',' + String(h) + ',' + String(f) + ',' + String(insideTemp) + ',' + String(outsideTemp) + ',' + String(light) + ',' + String(sound);
  return dataStream;
}


