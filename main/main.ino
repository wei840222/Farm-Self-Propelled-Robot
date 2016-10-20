#include <Servo.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "LiquidCrystal_I2C.h"
#include "QuickStats.h"

///////////////////////////////////   硬體類別   ///////////////////////////////////
class Motor {
  private:
    int pinA, pinB, pinPWM;
    int reverse;
    const static int powerMax = 255;

  public:
    int defaut;

    void init(int A, int B, int PWM) {
      pinA = A;
      pinB = B;
      pinPWM = PWM;
      reverse = 1;
      defaut = 0;
      pinMode (pinA, OUTPUT);
      pinMode (pinB, OUTPUT);
      pinMode (pinPWM, OUTPUT);
    }

    void setReverse() {
      reverse *= -1;
    }

    void setDefaut(int D) {
      defaut = D;
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

    void fwd() {
      output(defaut);
    }

    void back() {
      setReverse();
      output(defaut);
      setReverse();
    }
};

class Ultrasonic {
  private:
    int pinTrig, pinEcho;
    int duration;
    float distance;
    const static float distanceMax = 100;

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

///////////////////////////////////   關卡控制   ///////////////////////////////////
int stage = 0;                  //關卡編號
int stageAngle = 0;             //關卡角度
int potCount = 0;               //計數盆栽
int skipPot = 0;                //略過澆灌幾個盆栽

///////////////////////////////////   建立裝置物件   ///////////////////////////////////
QuickStats stats;
Servo serB, serL, serR;
MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Motor motLF, motLB, motRF, motRB;
Ultrasonic ultL, ultR, ultF, ultB;

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

  //初始化左右輪馬達並設定基值
  motLF.init(22, 23, 10);
  motRF.init(24, 25, 11);
  motLB.init(37, 38, 8);
  motRB.init(39, 40, 9);
  motLF.setDefaut(90);
  motLB.setDefaut(90 / 6.5 * 12);
  motRF.setDefaut(113);
  motRB.setDefaut(113 / 6.5 * 12);

  //初始化抽水馬達
  pinMode (34, OUTPUT);

  //連接伺服馬達並旋轉至初始角度
  serB.attach(5);
  serL.attach(6, 500, 2400);
  serR.attach(7, 500, 2400);
  serB.write(140);
  serL.write(85);
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
/*
  Serial.print("A:");
  Serial.print(calculateAngle());
  Serial.print(" L:");
  Serial.print(ultL.distanceCM());
  Serial.print(" R:");
  Serial.println(ultR.distanceCM());
*/
  stageEvent();
  if (!avoidance()) fixStraight();
  waitForPause();
}
