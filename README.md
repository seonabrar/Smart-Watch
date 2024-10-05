# SMART WATCH – HEALTH AND FITNESS MONITORING SYSTEM

## ABSTRACT
This project presents the design and implementation of a multi-functional embedded system integrating various sensors and modules for environmental monitoring, user interaction, and health metrics. The core of the system is the Seeed Studio Xiao NRF52840 microcontroller, which orchestrates the operation of several connected components. 

## INTRODUCTION
The purpose of this document is to specify the requirements, implementation, and testing plan for Health and Fitness Monitoring System, also referred to as the "Smart Watch." This system is designed to cater to outdoor enthusiasts by providing real-time tracking and monitoring of vital health metrics and activity levels. The document aims to outline what the device is intended to achieve, how it has implemented, and the testing procedures to ensure that it meets the specified requirements. The ultimate goal is to enhance performance, safety, and overall well-being during outdoor activities such as hiking, trail running, and mountain biking.

## Block Diagram

This block diagram illustrates the connections between the Seeed Xiao Nrf52840 microcontroller and various sensors used in the Health and Fitness Monitoring System. The sensors connected via I2C are shown in blue arrows (input sensors), while the display connected via SPI is shown with a red arrow (output sensor).

![Alt text](/images/image1.png)

## Schematic

![Alt text](/images/image2.png)

## PCB

![Alt text](/images/image3.png)
            Figure:  2D - PCB Layout for the Device.

![Alt text](/images/image4.png)

             Figure: 3D - PCB Layout for the Device.
             
             
## Software 
The software for the Seeed Xiao Based Health and Fitness Monitoring System was designed to manage sensor data acquisition, processing and display management. The software architecture was modular to facilitate ease of development, testing, and future enhancements.

##Software Architecture
The software is divided into several modules, each responsible for specific functionalities:
• Sensor Data Acquisition: Manages communication with sensors (MAX30105, BME280, LSM6DS3, DS1307) using I2C and SPI protocols to collect raw data.
• Data Processing and Analysis: Processes raw sensor data to derive meaningful health metrics and activity data.
• Display Management: Updates the 1.69 Inch LCD IPS screen with the latest health metrics and activity data.
• Power Management: Implements various power-saving strategies to extend battery life, including using deep sleep mode during periods of inactivity.

## Calculations
Seeed Xiao Nrf52840 with the connected sensors (LSM6DS3, MAX30105, DS1307, BME280) when using a 3.7V 300mAh LiPo battery, we need to consider the current draw of each component and the microcontroller itself.
### Step 1: Current Consumption of Each Component
Let's break down the current consumption of each component: Seeed Xiao Nrf52840
• Active Mode (without BLE): ~6 mA
• Idle Mode: ~1.5 mA
• Deep Sleep: ~0.1 mA
LSM6DS3 (Accelerometer + Gyroscope)
• Active Mode: ~0.67 mA
• Power Down Mode: ~1 µA
MAX30105 (Heart Rate Monitor)
• Typical: ~1.6 mA (continuous monitoring)
• Standby: ~0.7 µA
DS1307 (RTC)
• Typical: ~1.5 mA (when accessing data)
• Standby: ~1.1 µA (when only keeping time)
BME280 (Temperature, Humidity, Pressure)
• Active Mode: ~0.68 mA
• Sleep Mode: ~0.1 µA
ST7789 Display
• Typical: ~7 mA (with backlight on)
• Standby: ~0.2 mA
### Step 2: Total Current Consumption Calculation
Assuming all components are active all the time (no sleep Mode):
• Seeed Xiao Nrf52840 (Active): 6 mA
• LSM6DS3 (Active Mode): 0.67 mA
• MAX30105 (Typical): 1.6 mA
• DS1307 (Typical): 1.5 mA
• BME280 (Active Mode): 0.68 mA
• ST7789 Display (Active Mode with backlight): 7 mA
Total current consumption: 6 mA + 0.67 mA + 1.6 mA + 1.5 mA + 0.68 mA +7 mA = 17.45 mA
### Step 3: Battery Life Calculation
Using a 3.7V, 300mAh LiPo battery:
Battery capacity in mAh: 300 mAh
Battery capacity in Ah: 300 mAh = 0.3 Ah
Battery life (in hours):
Battery Life = Battery Capacity (Ah) / Total Current Consumption (A) = 0.3 Ah / 0.01745 A ≈ 17.2 hours
The Seeed Xiao Nrf52840 with the LSM6DS3, MAX30105, DS1307, and BME280 connected and all components active, excluding BLE, would consume approximately 17.45 mA. Using a 3.7V 300mAh LiPo battery, the system would run for approximately 17.2 hours.
