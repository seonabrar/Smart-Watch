#include <st7789v2.h>
#include "SPI.h"
#include "Logo.h"

#include "Wire.h"

#include "MAX30105.h"
#include "heartRate.h"

#include <DS1307.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "LSM6DS3.h"

DS1307 rtc;

MAX30105 particleSensor;

#define CLEAR_STEP      true
#define NOT_CLEAR_STEP  false

//Create a instance of class LSM6DS3
LSM6DS3 pedometer(I2C_MODE, 0x6A);    //I2C device address 0x6A

const byte RATE_SIZE = 5; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];    // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

st7789v2 Display;

#define BUTTON_PIN D7

unsigned long lastDisplayUpdate = 0;

// Variable to track the button press count
volatile int Screen = 0;
volatile int publishImage = 0;

uint8_t sec, mins, hour, day, month;
uint16_t year;

Adafruit_BME280 bme; // I2C

unsigned long delayTime;
#define SEALEVELPRESSURE_HPA (1013.25)

// Interrupt Service Routine (ISR) for button press
void handleButtonPress() {
  publishImage = 0;
  Screen++;
}

//Setup pedometer mode
int config_pedometer(bool clearStep) {
    uint8_t errorAccumulator = 0;
    uint8_t dataToWrite = 0;  //Temporary variable

    //Setup the accelerometer******************************
    dataToWrite = 0;

    //  dataToWrite |= LSM6DS3_ACC_GYRO_BW_XL_200Hz;
    dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
    dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_26Hz;


    // Step 1: Configure ODR-26Hz and FS-2g
    errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, dataToWrite);

    // Step 2: Set bit Zen_G, Yen_G, Xen_G, FUNC_EN, PEDO_RST_STEP(1 or 0)
    if (clearStep) {
        errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3E);
    } else {
        errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3C);
    }

    // Step 3:	Enable pedometer algorithm
    errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x40);

    //Step 4:	Step Detector interrupt driven to INT1 pin, set bit INT1_FIFO_OVR
    errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_INT1_CTRL, 0x10);

    return errorAccumulator;
}

void setup() {
  Serial.begin(115200);
  // Configure the button pin as input with an internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Attach the interrupt to the button pin
  // Trigger the ISR on the falling edge (button press)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  Serial.println("Button interrupt setup complete.");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED

  Serial.println("Init RTC...");
  rtc.begin();

  //only set the date+time one time
  rtc.set(0, 11, 3, 5, 6, 2024); //08:00:00 24.12.2014 //sec, min, hour, day, month, year

  //stop/pause RTC
  // rtc.stop();

  //start RTC
  rtc.start();

// default settings
  unsigned status = bme.begin();  
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
    
  Serial.println("-- Default Test --");
  delayTime = 1000;

  if (pedometer.begin() != 0) {
      Serial.println("Device error");
  } else {
      Serial.println("Device OK!");
    }

    //Configure LSM6DS3 as pedometer
  if (0 != config_pedometer(NOT_CLEAR_STEP)) {
      Serial.println("Configure pedometer fail!");
  }
  Serial.println("Success to Configure pedometer!");

  Display.SetRotate(0);
  Display.Init();
  Display.SetBacklight(100);
  Display.Clear(DARKBLUE);
}

void loop() {
  if (Screen == 0){
    DateAndTime();
  }

  if (Screen == 1){
    HRApp();
  }  
  
  if (Screen == 2){
    TempAndHum();
  }

   if (Screen == 3){
    AltAndPress();
  } 

  if (Screen == 4){
    StepCounter();
  } 

  if (Screen >= 5){
    
    Screen = 0;
  }
  
}

void HRApp() {
  long irValue = particleSensor.getIR(); // Reading the IR value to know if there's a finger on the sensor

  if (checkForBeat(irValue) == true) { // If a heartbeat is detected
    long delta = millis() - lastBeat; // Measure duration between two beats
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0); // Calculating the BPM

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
      rateSpot %= RATE_SIZE;                    // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }

  if (millis() - lastDisplayUpdate > 500) {
    if (publishImage == 0){
        Display.Clear(DARKBLUE);
    Display.DrawString_EN(0, 50, "  Heart Rate    ", &Font24, DARKBLUE, WHITE);
    Display.DrawString_EN(70, 110, " BPM    ", &Font24, DARKBLUE, WHITE);
    Display.DrawImage(gImage_heart, 0, 140, 240, 280);
  //  Display.DrawNum(100, 220, 123456, &Font24, RED, BRED);
      publishImage = 1;
    }
    if (publishImage == 1){
      Display.DrawNum(100, 80, beatAvg, &Font24, DARKBLUE, WHITE);
    }
    lastDisplayUpdate = millis();
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  if (irValue < 60000) {
    Serial.print(" No finger?");
  }
  if (millis() - lastBeat > 5000) {
    beatsPerMinute = 0;
    beatAvg = 0;
  }
  Serial.println();
}


void DateAndTime(){
    if (publishImage == 0){
      Display.Clear(DARKBLUE);

    Display.DrawString_EN(40, 20, "  Date ", &Font24, DARKBLUE, WHITE);
    Display.DrawString_EN(40, 80, "  Time ", &Font24, DARKBLUE, WHITE);
    //Display.DrawImage(gImage_heart, 0, 140, 240, 280);
      //Display.DrawImage(gImage_heart, 0, 140, 240, 280);
      Display.DrawImage(gImage_time, 0, 140, 240, 280);
    //  Display.DrawNum(100, 220, 123456, &Font24, RED, BRED);

      publishImage = 1;
    }
    if (publishImage == 1){
        rtc.get(&sec, &mins, &hour, &day, &month, &year);
        Display.DrawNum(50, 50, day, &Font24, DARKBLUE, WHITE);
        Display.DrawString_EN(65, 50, ":", &Font24, DARKBLUE, WHITE);
        Display.DrawNum(80, 50, month, &Font24, DARKBLUE, WHITE);
        Display.DrawString_EN(95, 50, ":", &Font24, DARKBLUE, WHITE);        
        Display.DrawNum(110, 50, year, &Font24, DARKBLUE, WHITE);

        Display.DrawNum(50, 110, hour, &Font24, DARKBLUE, WHITE);
        Display.DrawString_EN(70, 110, ":", &Font24, DARKBLUE, WHITE);
        Display.DrawNum(90, 110, mins, &Font24, DARKBLUE, WHITE);
        Display.DrawString_EN(130, 110, ":", &Font24, DARKBLUE, WHITE);        
        Display.DrawNum(150, 110, sec, &Font24, DARKBLUE, WHITE);

    }
}

void TempAndHum(){
  if (publishImage == 0){
      Display.Clear(DARKBLUE);

    Display.DrawString_EN(0, 20, "  Temperature ", &Font24, DARKBLUE, WHITE);
    Display.DrawString_EN(20, 80, "  Humidity ", &Font24, DARKBLUE, WHITE);
    //Display.DrawImage(gImage_heart, 0, 140, 240, 280);
    Display.DrawImage(gImage_temp, 0, 140, 240, 280);
  
      publishImage = 1;
    }
    if (publishImage == 1){
         Display.DrawFloatNum(80, 50, bme.readTemperature(), 3, &Font24, WHITE, DARKBLUE);
         Display.DrawFloatNum(80, 110, bme.readHumidity(), 3, &Font24, WHITE, DARKBLUE);
    }
}


void AltAndPress(){
    if (publishImage == 0){
      Display.Clear(DARKBLUE);

    Display.DrawString_EN(20, 20, "  Altitude ", &Font24, DARKBLUE, WHITE);
    Display.DrawString_EN(20, 80, "  Pressure ", &Font24, DARKBLUE, WHITE);
    //Display.DrawImage(gImage_heart, 0, 140, 240, 280);
    Display.DrawImage(gImage_alt, 0, 140, 240, 280);
  
      publishImage = 1;
    }
    if (publishImage == 1){
         Display.DrawFloatNum(80, 50, bme.readAltitude(SEALEVELPRESSURE_HPA), 2, &Font24, WHITE, DARKBLUE);
         Display.DrawFloatNum(80, 110, bme.readPressure()/ 100.0F, 2, &Font24, WHITE, DARKBLUE);
    }

}


void StepCounter(){
     uint8_t dataByte = 0;
    uint16_t stepCount = 0;

    pedometer.readRegister(&dataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);
    stepCount = (dataByte << 8) & 0xFFFF;

    pedometer.readRegister(&dataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);
    stepCount |=  dataByte;

    Serial.print("Step: ");
    Serial.println(stepCount);

    

    if (publishImage == 0){
      Display.Clear(DARKBLUE);

      Display.DrawString_EN(0, 50, "  Steps Count ", &Font24, DARKBLUE, WHITE);
      Display.DrawString_EN(80, 110, "Steps", &Font24, DARKBLUE, WHITE);
      //Display.DrawImage(gImage_heart, 0, 140, 240, 280);
      Display.DrawImage(gImage_stepscount, 0, 140, 240, 280);
    //  Display.DrawNum(100, 220, 123456, &Font24, RED, BRED);

      publishImage = 1;
    }
    if (publishImage == 1){
      Display.DrawNum(110, 80, stepCount, &Font24, DARKBLUE, WHITE);
    }

    delay(500);
}
