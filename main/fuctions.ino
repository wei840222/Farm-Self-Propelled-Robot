///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  switch (stage) {
    case 0:
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
  motL.output(-motorLeftDefaut);
  motR.output(motorRightDefaut);
  rotate = 'L';

  //尋找盆栽
  while (ultB.distanceCM() > potFindDistance);

  //計算盆栽中心點
  potDistance = ultB.distanceCM();
  while (true) {
    distance = ultB.distanceCM();
    if (distance < potDistance) potDistance = distance;
    if (distance > potFindDistance) break;
  }

  //對準盆栽中心點
  while (true) {
    distance = ultB.distanceCM();
    if (distance > potDistance) {
      if (rotate == 'L') {
        motL.output(motorLeftDefaut);
        motR.output(-motorRightDefaut);
        rotate = 'R';
      }
      if (rotate == 'R') {
        motL.output(-motorLeftDefaut);
        motR.output(motorRightDefaut);
        rotate = 'L';
      }
    }
    if (distance <= potDistance) break;
  }

  motL.stop();
  motR.stop();
  rotate = null;
  delay(1000);

  //調整至澆灌距離
  while (true) {
    distance = ultB.distanceCM();
    if (distance < potIrrigateDistance) {
      motL.output(motorLeftDefaut);
      motR.output(motorRightDefaut);
    }
    if (distance > potIrrigateDistance) {
      motL.output(-motorLeftDefaut);
      motR.output(-motorRightDefaut);
    }
    if (distance == potIrrigateDistance) {
      motL.stop();
      motR.stop();
      break;
    }
  }

  delay(1000);

  //澆水2秒
  watering(2000);
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
