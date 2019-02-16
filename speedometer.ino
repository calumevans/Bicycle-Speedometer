#include <LiquidCrystal.h>


//pin definitions
#define temp 0    //analog pin
#define magnet 6
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


//variable definitions
float tripDistance = 0;
unsigned long totalDistanceKM;
unsigned long previousTime;
float currentSpeed = 0;


void lcdStartup(){
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SPEEDOMETER 2000");
  lcd.setCursor(1, 1);
  lcd.print("BY CALUM EVANS");
  delay(2000);
  lcd.clear();
}

void lcdBackground(){
  lcd.setCursor(4, 0);
  lcd.print("km");
  lcd.setCursor(12, 0);
  lcd.print("km/h");
  lcd.setCursor(4, 1);
  lcd.print("km");
  lcd.setCursor(9,1);
  lcd.print((char)223);
  lcd.setCursor(10,1);
  lcd.print("C");
  lcd.setCursor(13, 1);
  lcd.print(":");
}

int readTemp(){
  int tempVoltage = analogRead(temp);
  double tempK = log(10000.0 * ((1024.0 / tempVoltage - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );
  float tempC = tempK - 273.15;       //converting Kelvin to Celcius
  int temperature = (int)tempC;       //this just truncates the temperature but it doesn't need to be precise

  return temperature;
}

void tripTime(){
  unsigned long milliseconds = millis();
  unsigned long seconds = milliseconds/1000;
  unsigned long minutes = seconds/60;
  
  //values to be used in the screen
  int tripHours = minutes/60;
  int tripMinutes = minutes % 60;

    lcd.setCursor(12, 1);      //hours
    lcd.print(tripHours);

  if(tripMinutes < 10){
    lcd.setCursor(15, 1);      //minutes
    lcd.print(tripMinutes);
    lcd.setCursor(14, 1);      //minutes
    lcd.print("0");
  }else if(tripMinutes >= 10){
    lcd.setCursor(14, 1);
    lcd.print(tripMinutes);
  }
}

void checkRotation(){
  int initialMagnet = digitalRead(magnet);
  
  if(!initialMagnet){
    calculations(previousTime, tripDistance);
    while(!digitalRead(magnet)){
      if(digitalRead(magnet))
        break;
    }
  }
  lcdData();        //to put the data from the wheel rotation onto the display
}


void calculations(unsigned long prevTime, float distance){
  unsigned long currentTime = millis();
  float deltaTime = (float)(currentTime - prevTime)/1000;

  distance = distance + 2.086;
  float speed = (2.086 / deltaTime)*3.6;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" m   Cycle Time: ");
  Serial.print(deltaTime);
  Serial.print(" s   Speed: ");
  Serial.print(speed);
  Serial.println(" km/h");

  previousTime = currentTime;
  tripDistance = distance;
  currentSpeed = speed;
}


void lcdData(){
//-----------------------------------------current speed
  if(currentSpeed < 10){         
    lcd.setCursor(11, 0);
    lcd.print((int)currentSpeed);
    lcd.setCursor(10, 0);
    lcd.print("0");
  }else if(currentSpeed < 100){
    lcd.setCursor(10, 0);
    lcd.print((int)currentSpeed);
  }

  
//-----------------------------------------trip distance
  float distanceKM = (tripDistance/1000);
  //Serial.println(distanceKM);
  if(distanceKM < 10){
    lcd.setCursor(0, 1);
    lcd.print("0");
    lcd.setCursor(1, 1);
    lcd.print(distanceKM);
    lcd.setCursor(4, 1);
    lcd.print("km");
  }else if(distanceKM < 100){
    lcd.setCursor(0, 1); 
    lcd.print(distanceKM);
    lcd.setCursor(4, 1);
    lcd.print("km");
  }


//-----------------------------------------total distance
  totalDistanceKM = (unsigned long)(totalDistanceKM + distanceKM);
  if(totalDistanceKM < 10){
    lcd.setCursor(3, 0);
    lcd.print(totalDistanceKM);
  }else if(totalDistanceKM < 100){
    lcd.setCursor(2, 0);
    lcd.print(totalDistanceKM);
  }else if(totalDistanceKM < 1000){
    lcd.setCursor(1, 0);
    lcd.print(totalDistanceKM);
  }else if(totalDistanceKM < 10000){
    lcd.setCursor(2, 0);
    lcd.print(totalDistanceKM);
  }

  
//-----------------------------------------temperature
    lcd.setCursor(7, 1);
    lcd.print(readTemp());  
  
}

void setup(){
  Serial.begin(9600);
  lcdStartup();
  lcdBackground();
  lcdData();
  pinMode(magnet, INPUT);
  previousTime = millis();

}

void loop(){
  tripTime();
  checkRotation();
}

