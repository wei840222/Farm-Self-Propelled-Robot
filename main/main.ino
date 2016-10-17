#include <Servo.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "LiquidCrystal_I2C.h"

///////////////////////////////////   硬體類別   ///////////////////////////////////
class Motor {
  private:
    int pinA, pinB, pinPWM;
    int reverse = 1;
    const static int powerMax = 255;

  public:
    void init(int A, int B, int PWM) {
      pinA = A;
      pinB = B;
      pinPWM = PWM;
      pinMode (pinA, OUTPUT);
      pinMode (pinB, OUTPUT);
      pinMode (pinPWM, OUTPUT);
    }

    void setReverse() {
      reverse = -1;
    }

    void stop() {
      digitalWrite(pinA, LOW);
      digitalWrite(pinB, LOW);
      analogWrite(pinPWM, 0);
    }

    void output(int power) {
      power *= reverse;
      if (power > powerMax) power = powerMax;
      if (power < -powerMax)power = -powerMax;
      if (power >= 0) {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, LOW);
        analogWrite(pinPWM, abs(power));
      }
      else {
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        analogWrite(pinPWM, abs(power));
      }
    }
};

class Ultrasonic {
  private:
    int pinTrig, pinEcho;
    int duration, distance;
    const static int distanceMax = 100;

  public:
    void init(int trig, int echo) {
      pinTrig = trig;
      pinEcho = echo;
      pinMode (pinTrig, OUTPUT);
      pinMode (pinEcho, INPUT);
    }

    float distanceCM() {
      digitalWrite(pinTrig, LOW);
      delayMicroseconds(2);
      digitalWrite(pinTrig, HIGH);
      delayMicroseconds(10);
      digitalWrite(pinTrig, LOW);
      duration = pulseIn(pinEcho, HIGH, 5820);
      if (duration == 0) return distanceMax;
      else {
        distance = duration / 58.2;
        if (distance > distanceMax) return distanceMax;
        else return distance;
      }
    }
};

///////////////////////////////////   全域變數   ///////////////////////////////////
int motorForwardLeftDefaut;            //左前馬達基速
int motorForwardRightDefaut;           //右前馬達基速
int motorBackLeftDefaut;               //左後馬達基速
int motorBackRightDefaut;              //右後馬達基速

int stage = 0;                  //關卡編號
int count = 0;                  //計數盆栽
///////////////////////////////////   建立裝置物件   ///////////////////////////////////
MPU6050 mpu;
Ultrasonic ultL, ultR, ultF, ultB;
Motor motFL, motFR, motBL, motBR;
Servo serB, serL, serR;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
  //設定鮑率
  Serial.begin(38400);

  //初始化陀螺儀
  mpuInit();
  //mpuOffset();

  //初始化超音波
  ultL.init(26, 27);
  ultR.init(28, 29);
  ultF.init(30, 31);
  ultB.init(32, 33);

  //初始化左右輪馬達
  motFL.init(22, 23, 3);
  motBL.init(37, 38, 8);
  motFR.init(24, 25, 4);
  motBR.init(39, 40, 9);

  //初始化抽水馬達
  pinMode (34, OUTPUT);

  //連接伺服馬達並旋轉至初始角度
  serB.attach(5);
  serL.attach(6, 500, 2400);
  serR.attach(7, 500, 2400);
  serB.write(140);
  serL.write(90);
  serR.write(90);

  //初始化按鈕
  pinMode (35, INPUT);
  pinMode (36, INPUT);

  //初始化LCD
  lcd.begin(16, 2);

  //等待選關準備開始
  waitForStart();
}

void loop() {
  pause();
  /*
  stageEvent();
  if (!avoidance()) fixStraight();
  */
}
