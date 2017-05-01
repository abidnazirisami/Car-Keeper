#include <SoftwareSerial.h>
#define wrongLed 13
#define rightLed A5
#define green 6
#define yellow 5
#define enable 3
#define trigOut 9
#define echoOut 8
#define mindis 6
#define outdis 8
#define trigIn 4
#define echoIn 2
#define dur 1200
#define dur2 1000
#define lightUp A3
#define lightDown A4
#define motorSpeed 100
SoftwareSerial ss(10,11);//RX,TX
int number,ind,cnt, savedPW[4], typedPW[4];
char ch;
void setup()
{
    pinMode(trigOut, OUTPUT);
    pinMode(echoOut, INPUT);
    pinMode(wrongLed, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(yellow, OUTPUT);
    pinMode(enable, OUTPUT);
    digitalWrite(green, LOW);
    digitalWrite(yellow, HIGH);
    while(!Serial);
    Serial.begin(9600);
    ss.begin(9600);
}
int i=0;
bool pwSaved=false, openDoor=true, doorOpen = false, carInside=false, gettingOut=false;
void loop()
{
    long duration, distance;
    digitalWrite(trigOut, LOW);
    delayMicroseconds(2);
    digitalWrite(trigOut, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigOut, LOW);
    duration = pulseIn(echoOut, HIGH);
    distance = (duration * 0.034 )/2;
    if(!doorOpen) {
      Serial.print(distance);
      Serial.println(" cm");
    }
    if(distance>outdis && !doorOpen) // Initial check for sensor
    {
      delay(250);
    }
    else if(!doorOpen && distance<outdis && distance > 0) // Sensor detected the car
    {
      digitalWrite(green, LOW);
      digitalWrite(yellow, HIGH);
      analogWrite(enable, motorSpeed);
      Serial.println("Up\n");
      analogWrite(lightUp, 255);
      delay(dur);
      analogWrite(enable, 0);
      doorOpen=true;          
    }

    if(carInside) // The car is inside, close the door
    {
      digitalWrite(green, HIGH);
      digitalWrite(yellow, LOW);
      analogWrite(enable, motorSpeed);
      Serial.println("Down\n");
      delay(dur2);
      analogWrite(lightUp, 0);
      analogWrite(enable, 0);
      carInside=false;
    }

    if(gettingOut && distance<outdis) // The car is out, close the door and reset everything
    {
      delay(5000);
      digitalWrite(green, HIGH);
      digitalWrite(yellow, LOW);
      analogWrite(enable, motorSpeed);
      Serial.println("Down\nResetting\n");
      delay(dur2);
      analogWrite(enable, 0);
      analogWrite(lightUp, 0);
      gettingOut=false;
      pwSaved=false;
      doorOpen=false;
      openDoor=true;
      carInside=false;
      delay(3000);
    }
    
    if(!pwSaved && ss.available()>0 && doorOpen) // No password is saved yet, wait for a password to be saved
    {
        cnt=0;
        while(ss.available()>0)
        {
            ch = ss.read();
            cnt++;
        }
        Serial.print(cnt);
        savedPW[i++]=cnt;

        if(i==4) // Password is saved
        {
            Serial.println(" is the new saved password");
            i=0;
            pwSaved=true;
            analogWrite(rightLed, 255);
            delay(1000);
            analogWrite(rightLed, 0);
            while(true) // This checks the sensor inside to find out if the car is inside yet
            {
              long duration2, distance2;
              digitalWrite(trigIn, LOW);
              delayMicroseconds(2);
              digitalWrite(trigIn, HIGH);
              delayMicroseconds(10);
              digitalWrite(trigIn, LOW);
              duration2 = pulseIn(echoIn, HIGH);
              distance2 = (duration2 * 0.034 )/2;
              Serial.print(distance2);
              Serial.println(" cm");
              if(distance2<mindis && distance2 > 0) // The car is inside
              {
                delay(5000);
                break;
              }
              delay(250);
            }
            carInside=true;
        }
    }
    else if(pwSaved && ss.available()>0 && !gettingOut && doorOpen) // The car is inside, password is saved.
                                                        // Check for incoming password
    {
        cnt=0;
        while(ss.available()>0)
        {
            ch = ss.read();
            cnt++;
        }
        Serial.print(cnt);
        typedPW[i++]=cnt;
        if(typedPW[i-1] != savedPW[i-1]) openDoor = false;
        if(i==4)
        {
            i=0;
            if(openDoor)  // The password is right. Open the door.
            {
                Serial.println(" Welcome");
                analogWrite(rightLed, 255);
                digitalWrite(green,LOW);
                digitalWrite(yellow, HIGH);
                analogWrite(enable, motorSpeed);
                Serial.println("Up\n");
                analogWrite(lightUp, 255);
                delay(dur);
                analogWrite(enable, 0);
                analogWrite(rightLed, 0);
                digitalWrite(green, LOW);
                gettingOut=true;      
            }
            else if(!openDoor)
            {
                Serial.println(" Wrong password, try again"); // The password is incorrect. Retry.
                digitalWrite(wrongLed, HIGH);
                delay(500);
                digitalWrite(wrongLed, LOW);
                delay(500);
                digitalWrite(wrongLed, HIGH);
                delay(500);
                digitalWrite(wrongLed, LOW);
                delay(500);
            }
            openDoor=true;
        }
    }
    delay(100);
}

