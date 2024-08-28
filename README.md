# SmartSleepTracker
## Introduction
Sleep is one of the most important things in life, and getting the proper amount leads to numerous benefits for both the mind and the body. Unfortunately, many people tend to sacrifice sleep far too willingly. Many people, including myself, find themselves deprived of the necessary amount of sleep and in some cases, are unable to identify potential issues that directly cause sleep deprivation.

The goal for our project was to create an IOT device that enhances your sleep. My partner Rushil and I have both have struggled with maintaining a solid sleep schedule and have learned of the impact it has on our lives. To do this, we designed and implemented a smart sleep tracker that works just like a normal alarm clock, but with a few added functionalities. With the use of sensors on our tracker, we can measure and record environmental data during your sleep that can inform you about factors that may be  impacting your sleep. Users can view the recorded data, such as sound levels and temperature levels, on a customized dashboard compiled after every night of sleep.

## Implementation
Photon Redboard provides a built-in WiFi module to connect the board to the Internet. The Photon Redboard with the Grove Shield is equipped with the Humidity/Temperature Sensor(RHT03), the Grove Light Sensor and the Grove Sound Sensor. This data is then sent through HTTP protocol using wifi to the Amazon EC2 Instance where Flask handles the data and stores it in the SQLite database. From there, the data is displayed using matplotlib onto a webpage as a data analytics dashboard.
<p align="center">
  <img src="https://github.com/trums99/SmartSleepTracker/blob/master/images/SmartSleepTrackerArchitecture.png" width="700px"/>
</p>

## How to use this program
https://drive.google.com/file/d/1RvpC8PWvsbB4JY0AhXD6wioGA6FRhzqC/view
