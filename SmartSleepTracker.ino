#include <SparkTime.h>
#include <Grove_LCD_RGB_Backlight.h>
#include <SparkFunRHT03.h>
#include "application.h"
#include <HttpClient.h>
#include <string>

#define SOUND_SENSOR A0
#define LIGHT_SENSOR A1
#define ALARM_BUTTON A6
#define EDIT_BUTTON TX
#define BUTTON_3 A5
#define BUZZER D7
#define SOUND_THRESHOLD_VALUE 400//The threshold to turn the led on 400.00*5/1024 = 1.95v
#define PRINT_RATE 750 // Time in ms to delay between prints.
#define DT 5
#define CLK 6
#define SW 8

const int RHT03_DATA_PIN = RX; // RHT03 data pin

const int RED_PIN = 2;
const int GREEN_PIN = 3;
const int BLUE_PIN = 4;

unsigned int minimumLight = 65536;
unsigned int maximumLight = 0;
float minimumTempC = 5505;
float maximumTempC = 0;
float minimumTempF = 9941;
float maximumTempF = 0;
float minimumHumidity = 100;
float maximumHumidity = 0;

int alarmButtonState = 0; 
int oldAlarmButtonState = 0;
int newAlarmButtonState = 0;

int editButtonState = 0;
int oldEditButtonState = 0; 
int newEditButtonState = 0; 

int buttonState = 0; 

int counter = 0;
int currentStateCLK;
int lastStateCLK;

int hours = 12;
int minutes = 0;
String AMorPM = "AM";

bool buzzerOn;

unsigned long sleepTime = 8000;
unsigned long sleepAlarm;
bool sleeping = false;


UDP UDPClient;
SparkTime rtc;

unsigned long currentTime;
unsigned long lastTime = 0UL;
String timeStr;
String dateStr;

RHT03 rht; //humidity sensor
rgb_lcd lcd;
HttpClient http; 



http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headerswill NULL
};

http_request_t request;
http_response_t response;





//how often to send to AWS
unsigned long startMillis; 
unsigned long currentMillis;
const unsigned long period = 60000; 


///////////////////////SETUP FUNCTION//////////////////////
void setup() {
    Serial.begin(9600);
    
    rtc.begin(&UDPClient, "north-america.pool.ntp.org");
    rtc.setTimeZone(-8); 
    
    rht.begin(RHT03_DATA_PIN); // Initialize the RHT03 sensor
    
    
    pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	lastStateCLK = digitalRead(CLK);
	
    pinMode(BUZZER, OUTPUT); //buzzer
    pinMode(ALARM_BUTTON, INPUT);// initialize the pushbutton pin as an input
    pinMode(EDIT_BUTTON, INPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(CLK,INPUT_PULLUP);
	pinMode(DT,INPUT_PULLUP);
    digitalWrite(BUZZER, LOW);
    lcd.begin(16, 2);
    lastStateCLK = digitalRead(CLK);
    
    
}




/////////////////////LOOP FUNCTION/////////////////////
void loop() {
    
    request.path = "/send?";
    
    getTime();
    checkAlarmState();
    checkEditState();
    checkAlarmTime();
    checkBuzzer();
    getSound();
    getLight();
    getHumidity();
        
        

    
    //SENDING TO AWS EC2
    request.hostname = "18.217.215.102";
    request.port = 5000;
    
    Serial.println(request.path);
    
    currentMillis = millis();  
    if (currentMillis - startMillis >= period)  
    {
        http.get(request, response, headers);
        Serial.println("SENDING TO AWS");
        startMillis = currentMillis; 
        Serial.println(response.status);
    }
    
    delay(PRINT_RATE);
}






void checkBuzzer(){
    if (buzzerOn){
        digitalWrite(BUZZER, HIGH);
        delay(100);
        digitalWrite(BUZZER, LOW);
    }
}


void checkAlarmTime(){
    currentTime = rtc.now();
    Serial.print(String(hours) + String(minutes) + AMorPM);
    int tempHour = rtc.hour(currentTime);
    if (tempHour>12) {
        tempHour -= 12;
    }
    if (tempHour == 0) {
        tempHour = 12;
    }
    if(hours == int(tempHour) && minutes == int(rtc.minute(currentTime)) && AMorPM == rtc.AMPMString(currentTime) 
        && rtc.second(currentTime) == 0 && alarmButtonState == 1){
        Serial.println(rtc.second(currentTime));
        Serial.println("BUZZER ON");
        buzzerOn = true;
    }
}


bool buttonPress() {
    newEditButtonState = digitalRead(EDIT_BUTTON);
    
    if (newEditButtonState == HIGH && oldEditButtonState == LOW) {
        oldEditButtonState = newEditButtonState;
        return true;
    } else {
        oldEditButtonState = newEditButtonState;
        return false;
    }
}


int readEncoder(){
	currentStateCLK = digitalRead(CLK);
    int number = 0;

	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
		if (digitalRead(DT) != currentStateCLK) {
			number = -1;
		} else {
			number = 1;
		}
	}
	lastStateCLK = currentStateCLK;
	
	return number;
}


void checkAlarmState(){
    newAlarmButtonState = digitalRead(ALARM_BUTTON);
    if (newAlarmButtonState == HIGH){ 
        if (alarmButtonState == 0) {
            digitalWrite(RED_PIN, HIGH);
            digitalWrite(GREEN_PIN, HIGH);
            digitalWrite(BLUE_PIN, HIGH);
            alarmButtonState = 1;
        } else {
            digitalWrite(RED_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
            digitalWrite(BLUE_PIN, LOW);
            alarmButtonState = 0;
            buzzerOn = false;
        }
    }
}


//EDIT BUTTON SENSOR
void checkEditState(){
    editButtonState = 1;
    
    if (buttonPress())
    {
        lcd.setCursor(0,0);
        lcd.print(String(hours)); 
        lcd.setCursor(3,0);
        lcd.print("00");
        lcd.setCursor(6,0);
        lcd.print("00");
        lcd.setCursor(9,0);
        lcd.print(AMorPM);
        
        while (editButtonState < 4) {
            if (buttonPress())
            {
                editButtonState++;
            }
            Serial.print(editButtonState);
            
            if (editButtonState == 1) { //hours
                Serial.print("hours");
                lcd.setCursor(0,0);
                lcd.blink();
                int n = readEncoder();
                
                if(n == 1){
                    if (hours == 12){
                        hours = 1;
                    }
                    else {
                        hours += n;
                    }
                } else if (n == -1){
                    if (hours == 1){
                        hours = 12;
                    }
                    else {
                        hours += n;
                    }
                } 
                if(hours < 10){
                    lcd.print("0");
                }
                lcd.print(String(hours));
                lcd.setCursor(0,0);
            }
            
            if (editButtonState == 2) { //minutes
                lcd.setCursor(3,0);
                lcd.blink();
                int n = readEncoder();
                
                 if(n == 1){
                    if (minutes == 59){
                        minutes = 0;
                    }
                    else {
                        minutes += 1;
                    }
                 } else if (n == -1){
                    if (minutes == 0){
                        minutes = 59;
                    }
                    else {
                        minutes -= 1;
                    }
                }
                if(minutes < 10){
                    lcd.print("0");
                }
                lcd.print(String(minutes));
                lcd.setCursor(3,0);
            }
            
            if (editButtonState == 3) { //AM or PM
                lcd.setCursor(9,0);
                lcd.blink();
                int n = readEncoder();
                
                if(n == 1){
                    if (AMorPM == "AM"){
                        AMorPM = "PM";
                    }
                    else {
                        AMorPM = "AM";
                    }
                } else if (n == -1){
                    if (AMorPM == "AM"){
                        AMorPM = "PM";
                    }
                    else {
                        AMorPM = "AM";
                    }
                }
                lcd.print(AMorPM);
                lcd.setCursor(9,0);
            }
            
            delay(5);
        }
        lcd.noBlink();
    }
}
    

//TIME LIBRARY
void getTime(){
    currentTime = rtc.now();
    if (currentTime != lastTime) {
        // Build Date String
        dateStr = "";
        dateStr += rtc.monthString(currentTime);
        dateStr += "/"; 
        dateStr += rtc.dayString(currentTime);
        dateStr += "/";
        dateStr += rtc.yearString(currentTime).substring(2);
        Serial.println(dateStr);
        // Build Time String
        timeStr = "";
        timeStr += rtc.hour12String(currentTime);
        timeStr += ":";
        timeStr += rtc.minuteString(currentTime);
        timeStr += ":";
        timeStr += rtc.secondString(currentTime);	
        timeStr += " ";	
        timeStr += rtc.AMPMString(currentTime);
        Serial.println(timeStr);
        lastTime = currentTime;
        
        lcd.setCursor(0, 0);
        lcd.print(timeStr);
        lcd.setCursor(0, 1);
        lcd.print(dateStr);
        
        //request.path += "date=" + String(dateStr) + "&" + "time=" + String(timeStr) + "&";
    }
}


void getHumidity(){
    // Use the RHT03 member function `update()` to read new humidity and temperature values from the sensor.
    // 1 = success, 0 = fail
    int update = rht.update();
    
    if (update == 1) {  
        // The `humidity()` RHT03 member function returns a float value -- a percentage of RH between 0-100.
        int humidity = int(rht.humidity());
        if (humidity > maximumHumidity) maximumHumidity = humidity;
        if (humidity < minimumHumidity) minimumHumidity = humidity;
        
        // The `tempF()` RHT03 member function returns a float variable equal to the temperature in Farenheit.
        int tempF = int(rht.tempF());
        if (tempF > maximumTempF) maximumTempF = tempF;
        if (tempF < minimumTempF) minimumTempF = tempF;
        
        // `tempC()` works just like `tempF()`, but it returns the temperature value in Celcius
        //float tempC = rht.tempC();
        //if (tempC > maximumTempC) maximumTempC = tempC;
        //if (tempC < minimumTempC) minimumTempC = tempC;
        
        lcd.setCursor(12, 0);
        lcd.print(int(tempF));
        lcd.print(" F");
        lcd.setCursor(12, 1);
        lcd.print(int(humidity));
        lcd.print("%H");
        lcd.setCursor(0, 0);
        
        request.path += "humidity=" + String(humidity) + "&" + "temp=" + String(tempF) + "&";
        
    } 
    else 
    {
        Serial.println("Error reading from the RHT03."); // Print an error message
        Serial.println(); // Print a blank line
    }
}


void getSound(){
    int soundValue = analogRead(SOUND_SENSOR);
    //Serial.print("soundValue: ");
    //Serial.println(soundValue);
    
    request.path += "soundValue=" + String(soundValue) + "&";
}


void getLight(){
    //LIGHT SENSOR
    int lightValue = analogRead(LIGHT_SENSOR);//use A1 to read the electrical signal
    //Serial.print("lightValue: ");
    //Serial.println(lightValue);
    
    request.path += "lightValue=" + String(lightValue) + "&";
    
}