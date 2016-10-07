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
    motL.output(motorLeftDefaut + angle * (255 - motorLeftDefaut) / 90);
    motR.output(motorRightDefaut - angle * motorRightDefaut / 90);
  }
}

///////////////////////////////////   左右避障   ///////////////////////////////////
void avoidance() {
  const int fixDistance = 10;                   //開始閃牆距離
  const int minDistance = 5;                    //最小閃牆距離

  float distanceL = ultL.distanceCM();
  float distanceR = ultR.distanceCM();

  if (distanceL < minDistance) distanceL = minDistance;
  if (distanceR < minDistance) distanceR = minDistance;

  if (distanceL < fixDistance) {
    motL.output(motorLeftDefaut);
    motR.output(motorRightDefaut - motorRightDefaut / (fixDistance - minDistance) * (fixDistance - distanceL));
  }
  if (distanceR < fixDistance) {
    motL.output(motorLeftDefaut - motorLeftDefaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motR.output(motorRightDefaut);
  }
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

void lcdShowData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mpuGetAngle());
}

void serialShowData() {
}
