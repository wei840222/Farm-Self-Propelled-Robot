///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  switch (stage) {
    case 0:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      break;
    case 1:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測右方盆栽
      lcd.setCursor(0, 1);
      lcd.print("Pot:");
      lcd.print(count);
      if (count < 3 && ultR.distanceCM() < 40) {
        irrigateRightPot();
        count++;
      }
      //偵測換關
      if (ultF.distanceCM() < 50 && ultL.distanceCM() < 50) {
        motorLeftDefaut = 60;
        motorRightDefaut = 80;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 2:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 50 && ultL.distanceCM() < 50) {
        motorLeftDefaut = 60;
        motorRightDefaut = 80;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 3:
      motorLeftDefaut = 90;
      motorRightDefaut = 120;
      //偵測換關
      break;
    case 4:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 50 && ultR.distanceCM() < 50) {
        motorLeftDefaut = 60;
        motorRightDefaut = 80;
        rotateToAngle(-85);
        mpuInit();
        stage++;
      }
      break;
    case 5:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 50 && ultL.distanceCM() < 50) {
        motorLeftDefaut = 180;
        motorRightDefaut = 245;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 6:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 50 && ultL.distanceCM() < 50) {
        motorLeftDefaut = 180;
        motorRightDefaut = 245;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 7:
      motorLeftDefaut = 180;
      motorRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 50 && ultL.distanceCM() < 50) {
        stage++;
      }
      break;
    case 8:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clear!");
      lcd.setCursor(1, 0);
      lcd.print("Fuck all!");
      motL.stop();
      motR.stop();
  }
}

///////////////////////////////////   走直線   ///////////////////////////////////
void goStraight() {
  const int fixInterval = 1;                  //角度修正區間
  const int moreFixInterval = 20;             //加強角度修正區間
  const int fixMaxAngle = 90;                 //最大修正角度

  int angle = mpuGetAngle();

  if (abs(angle) > fixMaxAngle) angle = fixMaxAngle * angle / abs(angle);
  if (abs(angle) <= moreFixInterval) angle = moreFixInterval * angle / abs(angle);
  if (abs(angle) <= fixInterval) angle = 0;

  if (angle >= 0) {
    motL.output(motorLeftDefaut - angle * motorLeftDefaut / 90);
    motR.output(motorRightDefaut + angle * (255 - motorRightDefaut) / 90);
  }
  else {
    motL.output(motorLeftDefaut + abs(angle) * (255 - motorLeftDefaut) / 90);
    motR.output(motorRightDefaut - abs(angle) * motorRightDefaut / 90);
  }
}

///////////////////////////////////   左右避障   ///////////////////////////////////
bool avoidance() {
  const int fixDistance = 9;                   //開始閃牆距離
  const int minDistance = 4;                    //最小閃牆距離

  float distanceL = ultL.distanceCM();
  float distanceR = ultR.distanceCM();

  if (distanceL < minDistance) distanceL = minDistance;
  if (distanceR < minDistance) distanceR = minDistance;

  if (distanceL < fixDistance) {
    motL.output(motorLeftDefaut);
    motR.output(motorRightDefaut - motorRightDefaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    return true;
  }
  else if (distanceR < fixDistance) {
    motL.output(motorLeftDefaut - motorLeftDefaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motR.output(motorRightDefaut);
    return true;
  }
  else return false;
}

///////////////////////////////////   澆灌右邊盆栽   ///////////////////////////////////
void irrigateRightPot() {
  const int potFindDistance = 50;                //盆栽尋找距離
  const int potIrrigateDistance = 12;            //澆灌距離
  float distance;

  delay(300);
  //設定馬達基速
  motorLeftDefaut = 61;
  motorRightDefaut = 81;

  //逆時針旋轉尋找盆栽
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L-Rotate");
  lcd.setCursor(0, 1);
  lcd.print("Find Pot");
  motL.output(-motorLeftDefaut);
  motR.output(motorRightDefaut);
  while (ultB.distanceCM() > potFindDistance);

  //停車一秒鐘
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop 1s");
  motL.stop();
  motR.stop();
  delay(1000);

  //調整至澆灌距離
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fix Distance");
  do {
    distance = ultB.distanceCM();
    if (distance < potIrrigateDistance) {
      motL.output(motorLeftDefaut);
      motR.output(motorRightDefaut);
    }
    if (distance > potIrrigateDistance) {
      motL.output(-motorLeftDefaut);
      motR.output(-motorRightDefaut);
    }
  } while (distance != potIrrigateDistance);

  //停車一秒鐘
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop 1s");
  motL.stop();
  motR.stop();
  delay(1000);


  //澆水2秒
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Watering! 7s");
  watering(7000);
  delay(1000);

  //旋轉車體回0度
  rotateToAngle(0);

  //前進1秒
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Forwarding 1s");
  motL.output(motorLeftDefaut);
  motR.output(motorRightDefaut);
  delay(1000);
}

///////////////////////////////////   其他   ///////////////////////////////////
void waitForStart() {
  stage = 0;
  while (true) {
    delay(200);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stage:");
    lcd.print(stage);
    if (digitalRead(35)) {
      stage++;
      if (stage > 7) stage = 0;
      while (digitalRead(35));
    }
    if (digitalRead(36)) {
      while (digitalRead(36));
      break;
    }
  }
}

void watering(int time) {
  digitalWrite(34, HIGH);
  delay(time);
  digitalWrite(34, LOW);
}

void rotateToAngle(int rotationAngle) {
  const int missIvl = 5;
  int angle;
  do {
    angle = mpuGetAngle();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rotate to ");
    lcd.print(rotationAngle);
    lcd.setCursor(0, 1);
    lcd.print("Angle:");
    lcd.print(angle);
    if (angle > rotationAngle) {
      motL.output(-motorLeftDefaut);
      motR.output(motorRightDefaut);
    }
    if (angle < rotationAngle) {
      motL.output(motorLeftDefaut);
      motR.output(-motorRightDefaut);
    }
  } while (angle != rotationAngle);
  motL.stop();
  motR.stop();
  delay(1000);
}
