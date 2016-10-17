///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  switch (stage) {
    case 0:
      setupMotorSpeed(180, 245, 0, 0);
      break;
    case 1:
      setupMotorSpeed(180, 245, 0, 0);
      //偵測右方盆栽
      lcd.setCursor(0, 1);
      lcd.print("Pot:");
      lcd.print(count);
      if (count < 3 && ultR.distanceCM() < 40) {
        irrigateRightPot();
        count++;
      }
      //偵測換關
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        motorForwardLeftDefaut = 61;
        motorForwardRightDefaut = 81;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 2:
      setupMotorSpeed(180, 245, 0, 0);
      //偵測換關
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        motorForwardLeftDefaut = 61;
        motorForwardRightDefaut = 81;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 3:
      motorForwardLeftDefaut = 90;
      motorForwardRightDefaut = 120;
      //偵測換關
      break;
    case 4:
      motorForwardLeftDefaut = 180;
      motorForwardRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 35 && ultR.distanceCM() < 50) {
        motorForwardLeftDefaut = 61;
        motorForwardRightDefaut = 81;
        rotateToAngle(-85);
        mpuInit();
        stage++;
      }
      break;
    case 5:
      motorForwardLeftDefaut = 180;
      motorForwardRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        motorForwardLeftDefaut = 180;
        motorForwardRightDefaut = 245;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 6:
      motorForwardLeftDefaut = 180;
      motorForwardRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        motorForwardLeftDefaut = 180;
        motorForwardRightDefaut = 245;
        rotateToAngle(85);
        mpuInit();
        stage++;
      }
      break;
    case 7:
      motorForwardLeftDefaut = 180;
      motorForwardRightDefaut = 245;
      //偵測換關
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        stage++;
      }
      break;
    case 8:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clear!");
      lcd.setCursor(1, 0);
      lcd.print("Fuck all!");
      motFL.stop();
      motFR.stop();
  }
}

///////////////////////////////////   走直線   ///////////////////////////////////
void fixStraight() {
  const int fixInterval = 1;                  //角度修正區間
  const int moreFixInterval = 20;             //加強角度修正區間
  const int fixMaxAngle = 90;                 //最大修正角度
  int angle = mpuGetAngle();

  //顯示訊息
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FixingStraight");
  lcd.setCursor(0, 1);
  lcd.print("Angle:");
  lcd.print(mpuGetAngle());

  if (abs(angle) > fixMaxAngle) angle = fixMaxAngle * angle / abs(angle);
  if (abs(angle) <= moreFixInterval) angle = moreFixInterval * angle / abs(angle);
  if (abs(angle) <= fixInterval) angle = 0;

  if (angle >= 0) {
    motFL.output(motorForwardLeftDefaut - angle * motorForwardLeftDefaut / 90);
    motFR.output(motorForwardRightDefaut + angle * (255 - motorForwardRightDefaut) / 90);
  }
  else {
    motFL.output(motorForwardLeftDefaut + abs(angle) * (255 - motorForwardLeftDefaut) / 90);
    motFR.output(motorForwardRightDefaut - abs(angle) * motorForwardRightDefaut / 90);
  }
}

///////////////////////////////////   左右避障   ///////////////////////////////////
bool avoidance() {
  const int fixDistance = 9;                   //開始閃牆距離
  const int minDistance = 4;                    //最小閃牆距離
  float distanceL = ultL.distanceCM();
  float distanceR = ultR.distanceCM();

  //顯示訊息
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avoidancing");
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(distanceL);
  lcd.print(" R:");
  lcd.print(distanceR);

  if (distanceL < minDistance) distanceL = minDistance;
  if (distanceR < minDistance) distanceR = minDistance;

  if (distanceL < fixDistance) {
    motFL.output(motorForwardLeftDefaut);
    motFR.output(motorForwardRightDefaut - motorForwardRightDefaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    return true;
  }
  else if (distanceR < fixDistance) {
    motFL.output(motorForwardLeftDefaut - motorForwardLeftDefaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motFR.output(motorForwardRightDefaut);
    return true;
  }
  else return false;
}

///////////////////////////////////   澆灌右邊盆栽   ///////////////////////////////////
void irrigateRightPot() {
  const int potFindDistance = 50;                //盆栽尋找距離
  const int potIrrigateDistance = 20;            //澆灌距離
  float distance;

  delay(300);
  //設定馬達基速
  motorForwardLeftDefaut = 61;
  motorForwardRightDefaut = 81;

  //逆時針旋轉尋找盆栽
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L-Rotate");
  lcd.setCursor(0, 1);
  lcd.print("Find Pot");
  motFL.output(-motorForwardLeftDefaut);
  motFR.output(motorForwardRightDefaut);
  while (ultB.distanceCM() > potFindDistance);

  //停止
  motorAllStop();
  delay(1000);

  //調整至澆灌距離
  do {
    distance = ultB.distanceCM();
    if (distance < potIrrigateDistance)
      goForward();
    if (distance > potIrrigateDistance)
      goBack();
  } while (distance != potIrrigateDistance);

  //停車
  motorAllStop();
  delay(1000);

  //澆水
  watering(7000);
  delay(1000);

  //旋轉車體回0度
  rotateToAngle(0);

  //前進
  goForward();
  delay(1000);
}

///////////////////////////////////   其他   ///////////////////////////////////
void waitForStart() {
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

void pause() {
  if (digitalRead(36)) {
    while (digitalRead(36));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pause");
    motorAllStop();
    while (!digitalRead(36));
    while (digitalRead(36));
  }
}

void watering(int time) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water");
  digitalWrite(34, HIGH);
  delay(time);
  digitalWrite(34, LOW);
}

void setupMotorSpeed(int FL, int FR, int BL, int BR) {
  motorForwardLeftDefaut = FL;
  motorForwardRightDefaut = FR;
  motorBackLeftDefaut = BL;
  motorBackRightDefaut = BR;
}

void goForward() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Forward");
  motFL.output(motorForwardLeftDefaut);
  motFR.output(motorForwardRightDefaut);
  motBL.output(motorBackLeftDefaut);
  motBR.output(motorBackRightDefaut);
}

void goBack() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Back");
  motFL.output(-motorForwardLeftDefaut);
  motFR.output(-motorForwardRightDefaut);
  motBL.output(-motorBackLeftDefaut);
  motBR.output(-motorBackRightDefaut);
}

void motorAllStop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop");
  motFL.stop();
  motFR.stop();
  motBL.stop();
  motBR.stop();
}

void rotateToAngle(int rotationAngle) {
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
      motFL.output(-motorForwardLeftDefaut);
      motFR.output(motorForwardRightDefaut);
    }
    if (angle < rotationAngle) {
      motFL.output(motorForwardLeftDefaut);
      motFR.output(-motorForwardRightDefaut);
    }
  } while (angle != rotationAngle);
  motorAllStop();
  delay(1000);
}
