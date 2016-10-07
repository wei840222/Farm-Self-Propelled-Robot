///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  const int rightPot = 25;
  switch (stage) {
    case 0:
      break;
    case 1:
      if (ultR.distanceCM() < rightPot) irrigateRightPot();
      break;
  }
}

///////////////////////////////////   走直線   ///////////////////////////////////
void goStraight() {
  const int fixInterval = 1;                  //角度修正區間
  const int moreFixInterval = 10;             //加強角度修正區間
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
  const int fixDistance = 10;                   //開始閃牆距離
  const int minDistance = 5;                    //最小閃牆距離

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
  const int potFindDistance = 25;                //盆栽尋找距離
  const int potIrrigateDistance = 10;            //澆灌距離
  int distance, potDistance;                     //量測到的距離
  char rotate;                                   //旋轉方向

  //逆時針轉
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L-Rotate");
  motL.output(-75);
  motR.output(90);
  rotate = 'L';

  //尋找盆栽
  lcd.setCursor(0, 1);
  lcd.print("Find Pot");
  while (ultB.distanceCM() > potFindDistance);

  //計算盆栽中心點
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Find Pot Mid");
  potDistance = ultB.distanceCM();
  while (true) {
    distance = ultB.distanceCM();
    if (distance < potDistance) potDistance = distance;
    lcd.setCursor(0, 1);
    lcd.print("minDis=");
    lcd.print(potDistance);
    if (distance > potFindDistance) break;
  }

  //對準盆栽中心點
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check Pot Mid");
  while (true) {
    distance = ultB.distanceCM();
    if (distance > potDistance) {
      if (rotate == 'L') {
        motL.output(75);
        motR.output(-90);
        rotate = 'R';
        lcd.setCursor(0, 1);
        lcd.print("R-Rotate");
      }
      if (rotate == 'R') {
        motL.output(-75);
        motR.output(90);
        rotate = 'L';
        lcd.setCursor(0, 1);
        lcd.print("L-Rotate");
      }
    }
    if (distance <= potDistance) break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OK!");

  motL.stop();
  motR.stop();
  rotate = '\0';
  delay(1000);

  //調整至澆灌距離
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fix Distance");
  while (true) {
    distance = ultB.distanceCM();
    if (distance < potIrrigateDistance) {
      motL.output(75);
      motR.output(90);
    }
    if (distance > potIrrigateDistance) {
      motL.output(-75);
      motR.output(-90);
    }
    if (distance == potIrrigateDistance) {
      motL.stop();
      motR.stop();
      break;
    }
  }

  delay(1000);

  //澆水2秒
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Watering!");
  watering(2000);

  while(true);
}

///////////////////////////////////   其他   ///////////////////////////////////
void waitForStart() {
  while (true) {
    delay(200);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(stage);
    if (digitalRead(35)) {
      while (digitalRead(35));
      stage++;
      if (stage > 7) stage = 0;
    }
    if (digitalRead(36)) {
      while (digitalRead(36));
      break;
    }
  }
}

void avoidViolentConflict() {
  for (int i = 0; i < 10; i++) {
    motL.output(motorLeftDefaut * i / 10);
    motR.output(motorRightDefaut * i / 10);
    delay(200);
  }
}

void watering(int time) {
  digitalWrite(34, HIGH);
  delay(time);
  digitalWrite(34, LOW);
}
