#include <Arduino_FreeRTOS.h>
//#include <Servo.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//move
int x = A0;
int y = A1;
//target up down
MPU6050 mpu;
float Y, angle_y;
float y1, y2, y3;
//reload
int can_reload;
int trig = 7;
int echo = 6;
long duration, dis;
//led
int r = 11;
int g = 10;
int b = 9;
//lcd
LiquidCrystal_I2C lcd(0x27,16,2);
//light
int p1 = A2;
int p2 = A3;
int max1, max2, t1, t2, start;

void task_target(void *pvParameters);
void task_move(void *pvParameters);
void task_reload(void *pvParameters);
void task_lcd(void *pvParameters);
void task_light(void *pvParameters);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  

  //target
  pinMode(x, INPUT);
  pinMode(y, INPUT);

  // set 6050 offset
  mpu.setYAccelOffset(-2042);

  //shoot
  attachInterrupt(0, handle_click, RISING);

  //reload
  can_reload = 0;
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  //led
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);

  //lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("bullet:9"));

  //light
  pinMode(p1, INPUT);
  pinMode(p2, INPUT);
  start = 1;

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1<<CS12)| (1<<CS10);
  OCR1A = 15624;
  TIMSK1 |= (1<<OCIE1A);
  sei();

  //task
  xTaskCreate(task_target, "task_target", 128, NULL, 1, NULL);
  xTaskCreate(task_move, "task_move", 128, NULL, 1, NULL);
  xTaskCreate(task_reload, "task_reload", 128, NULL, 1, NULL);
  xTaskCreate(task_lcd, "task_lcd", 64, NULL, 1, NULL);
  xTaskCreate(task_light, "task_light", 64, NULL, 1, NULL);
  vTaskStartScheduler();
}

ISR(TIMER1_COMPA_vect){
  if(can_reload == 1){
    can_reload = 0;
    analogWrite(r, 0);
    analogWrite(g, 255);
    analogWrite(b, 0);
    Serial.println("reload");
  }
}

void handle_click(){
  static unsigned long last_int_time = 0;
  unsigned long int_time = millis();

  if (int_time - last_int_time > 200 ) {  
    Serial.println("shoot");
  }

  last_int_time = int_time;
}

//get angle of target
void task_target(void *pvParameters){
  (void)pvParameters;
  while(1){
    y1 = mpu.getAccelerationY(); delay(5);
    y2 = mpu.getAccelerationY(); delay(5);
    y3 = mpu.getAccelerationY();
    Y = (y1 + y2 + y3)/3.0;
    angle_y = asin(Y / 16384.0)*180/PI;
    if(isnan(angle_y) && Y>0) angle_y = 90;
    if(isnan(angle_y) && Y<0) angle_y = -90;
    Serial.println("target_v");
    Serial.println(angle_y);
    delay(100);
  }
}

void task_move(void *pvParameters){
  (void)pvParameters;
  while(1){
    int xx = analogRead(x);
    int yy = analogRead(y);
    Serial.println("move_h");
    Serial.println(xx);
    Serial.println("move_v");
    Serial.println(yy);
    delay(100);
  }
}

void task_reload(void *pvParameters){
  (void)pvParameters;
  while(1){
    digitalWrite(trig, LOW);
    delay(2);
    digitalWrite(trig, HIGH);
    delay(10);
    digitalWrite(trig, LOW);
    duration = pulseIn(echo, HIGH);
    dis = duration*0.034/2;
    if(dis < 10){
      analogWrite(r, 255);
      analogWrite(g, 0);
      analogWrite(b, 0);
      can_reload = 1;
    }else if(dis < 100){
      analogWrite(r, 0);
      analogWrite(g, 0);
      analogWrite(b, 255);
      can_reload = 0;
      TCNT1 = 0;
    }
    delay(100);
  }
}

void task_lcd(void *pvParameters){
  (void)pvParameters;
  while(1){
    if(Serial.available()){
      char num1 = Serial.read();
      lcd.setCursor(7, 0);
      lcd.print(num1);
    }
    delay(100);
  }
}

void task_light(void *pvParameters){
  (void)pvParameters;
  while(1){
    if(start == 1){
      start = 0;
      int a1 = analogRead(p1);
      int a2 = analogRead(p1);
      int a3 = analogRead(p1);
      max1 = (a1 + a2 + a3)/3;
      int b1 = analogRead(p2);
      int b2 = analogRead(p2);
      int b3 = analogRead(p2);
      max2 = (b1 + b2 + b3)/3;
    }else{
      t1 = analogRead(p1);
      t2 = analogRead(p2);
      if(t1 < max1/3){
        Serial.println("dark");
      }
      if(t2 < max2/3){
        Serial.println("light");
      }
    }
    delay(500);
  }
}
  

void loop() {
  
}
