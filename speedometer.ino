#include <LiquidCrystal.h>
#include <EEPROM.h>


//pin definitions
#define temp 1    //analog pin
#define magnet 11
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);


//variable definitions
int tripHours;
int tripMinutes;

float circum = 2.086;
float tripDistanceM = 0;
float tripDistanceKM = 0;
float storedDistanceKM;
float displayDistanceKM;

unsigned long previousTime;

float currentSpeedKMH = 0;
float previousSpeedKMH;
float previousSpeedMS = 0;
float currentSpeedMS;
float averageSpeedKMH;
float averageSpeedMS;

float accelerationMSS;

int distanceAddress = 0;
int eepromCounter = 0;
unsigned long rotationCounter = 0;


//-----------------------------------------STARTUP SCREEN
void lcdStartup(){
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SPEEDOMETER 2000");
  lcd.setCursor(1, 1);
  lcd.print("BY CALUM EVANS");
  delay(2000);
  lcd.clear();
}

//-----------------------------------------BACKGROUND SCREEN
void lcdBackground(){
  lcd.setCursor(0,0);
  lcd.print("    km      km/h");
  lcd.setCursor(0,1);
  lcd.print("    km    C  :  ");
  lcd.setCursor(9,1);
  lcd.print((char)223);
}

//-----------------------------------------READING TEMPERATURE
int readTemp(){
  int tempVoltage = analogRead(temp);
  double tempK = log(10000.0 * ((1024.0 / tempVoltage - 1)));     //this is a really standard formula for thermoresistors
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );
  float tempC = tempK - 273.15;       //converting Kelvin to Celcius
  int temperature = (int)tempC;       //this just truncates the temperature but it doesn't need to be precise

  return temperature;
}

//-----------------------------------------TRIP TIME
void tripTime(){
  unsigned long milliseconds = millis();
  unsigned long seconds = milliseconds/1000;
  unsigned long minutes = seconds/60;
  
  //values to be used in the screen
  tripHours = minutes/60;
  tripMinutes = minutes % 60;
}

//-----------------------------------------CHECKING ROTATION
void checkRotation(){
  int initialMagnet = digitalRead(magnet);
 
  if(!initialMagnet){
    calculations(previousTime);
    rotationCounter++;
    Serial.print("Rotations #: ");
    Serial.print(rotationCounter);
    while(!digitalRead(magnet)){
      if(digitalRead(magnet))
        break;
    }
  }
}

//-----------------------------------------DOING THE MATH
void calculations(unsigned long prevTime){
  previousSpeedKMH = currentSpeedKMH;
  previousSpeedMS = currentSpeedMS;

//-----------------------------------------time
  unsigned long currentTime = millis();
  float deltaTime = (float)(currentTime - prevTime)/1000;

//-----------------------------------------distance
  tripDistanceM = tripDistanceM + circum;

//-----------------------------------------speed
  currentSpeedMS = circum / deltaTime;
  currentSpeedKMH = currentSpeedMS*3.6;
  averageSpeedMS = tripDistanceM/(millis()/1000);
  averageSpeedKMH = averageSpeedMS*3.6;

//-----------------------------------------acceleration
  accelerationMSS = (currentSpeedMS - previousSpeedMS)/deltaTime;

//-----------------------------------------serial monitor
  Serial.print("   Distance: ");
  Serial.print(tripDistanceM);
  Serial.print("m   Display D: ");
  Serial.print(displayDistanceKM);
  Serial.print(" km   Cycle Time: ");
  Serial.print(deltaTime);
  Serial.print(" s   Speed: ");
  Serial.print(currentSpeedKMH);
  Serial.print(" km/h   Accel: ");
  
  Serial.print(accelerationMSS);
  Serial.print(" m/s^2");
  Serial.print("   Average Speed: ");
  Serial.print(averageSpeedKMH);
  Serial.println(" km/h");


  previousTime = currentTime;

//-----------------------------------------stored distance
//only want to store new data for the odometer every 1km (EEPROM has limited read/write cycles)
  displayDistanceKM = displayDistanceKM + (circum/1000);
}

//-----------------------------------------WRITING DATA TO SCREEN
void lcdData(){
//-----------------------------------------current speed
  if(currentSpeedKMH < 10){         
    lcd.setCursor(11, 0);
    lcd.print((int)currentSpeedKMH);
    lcd.setCursor(10, 0);
    lcd.print("0");
  }else if(currentSpeedKMH < 100){
    lcd.setCursor(10, 0);
    lcd.print((int)currentSpeedKMH);
  }
  
//-----------------------------------------trip distance
  tripDistanceKM = (tripDistanceM/1000);
  //Serial.println(tripDistanceKM);
  if(tripDistanceKM < 10){
    lcd.setCursor(0, 1);
    lcd.print("0");
    lcd.setCursor(1, 1);
    lcd.print(tripDistanceKM);
    lcd.setCursor(4, 1);
    lcd.print("km");
  }else if(tripDistanceKM < 100){
    lcd.setCursor(0, 1); 
    lcd.print(tripDistanceKM);
    lcd.setCursor(4, 1);
    lcd.print("km");
  }

//-----------------------------------------total distance
  int wholeNumberDistanceKM = (int)displayDistanceKM;
  if(displayDistanceKM < 10){
    lcd.setCursor(3, 0);
    lcd.print(wholeNumberDistanceKM);
  }else if(displayDistanceKM < 100){
    lcd.setCursor(2, 0);
    lcd.print(wholeNumberDistanceKM);
  }else if(displayDistanceKM < 1000){
    lcd.setCursor(1, 0);
    lcd.print(wholeNumberDistanceKM);
  }else if(displayDistanceKM < 10000){
    lcd.setCursor(2, 0);
    lcd.print(wholeNumberDistanceKM);
  }
  
//-----------------------------------------temperature

  int temperature = readTemp();
  if(temperature >= 0){
    if(temperature <10){
      lcd.setCursor(8, 1);
      lcd.print(temperature);
    }else if(temperature < 100){
      lcd.setCursor(7, 1);
      lcd.print(temperature);
    }
  }else if(temperature < 0){
    if(temperature > -10){
      lcd.setCursor(7, 1);
      lcd.print(temperature);
    }else if(temperature > -100){
      lcd.setCursor(6, 1);
      lcd.print(temperature);
    }
  }

//-----------------------------------------trip time
  if(tripHours < 10){         //hours
    lcd.setCursor(12, 1);
    lcd.print(tripHours);
  }

  if(tripMinutes < 10){       //minutes
    lcd.setCursor(15, 1);
    lcd.print(tripMinutes);
    lcd.setCursor(14, 1);
    lcd.print("0");
  }else if(tripMinutes >= 10){
    lcd.setCursor(14, 1);
    lcd.print(tripMinutes);
  }
}

//-----------------------------------------WRITING TOTAL DISTANCE TO MEMORY
void writeMemory(){      //after ~1km it will store the total distance value to memory        
  if(rotationCounter == 480){     
     EEPROM.put(distanceAddress,displayDistanceKM);   //storing the total distance
     eepromCounter++;
     Serial.print("Data Written: ");
     Serial.println(eepromCounter);
     rotationCounter = 0;        //resetting the rotation counter
  }
}


//-----------------------------------------BRAKE LIGHT
void brakeLight(){    //this will probably be done with an Adafruit Neopixel Ring
  if(accelerationMSS <= 0){       //basically if the bike is slowing down, it will change tail light's look
    //put taillight into stopping sequence
  }else{
    //put taillight into regular blinking sequence
  }
}



//-----------------------------------------SETUP
void setup(){
  Serial.begin(9600);
  
  EEPROM.get(distanceAddress,storedDistanceKM);
  if(isnan(storedDistanceKM))
    storedDistanceKM = 0;
  displayDistanceKM = storedDistanceKM;
  lcdStartup();
  lcdBackground();
  lcdData();
  pinMode(magnet, INPUT);
  previousTime = millis();

}

//-----------------------------------------LOOP
void loop(){
  tripTime();
  checkRotation();
  lcdData();
  brakeLight();
  writeMemory();
}
