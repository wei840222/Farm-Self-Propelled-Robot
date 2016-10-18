///////////////////////////////////   顯示資訊   ///////////////////////////////////
void showData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stage:");
  lcd.print(stage);
  lcd.print(" A:");
  lcd.print(mpuGetAngle());
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(ultL.distanceCM());
  lcd.print(" R:");
  lcd.print(ultR.distanceCM());
  Serial.print("A:");
  Serial.print(mpuGetAngle());
  Serial.print(" L:");
  Serial.print(ultL.distanceCM());
  Serial.print(" R:");
  Serial.println(ultR.distanceCM());
}
///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  static int count = 0;                  //計數盆栽

  switch (stage) {
    case 0:
      setupMotorDefaut(90, 167 , 113 , 210);
      goForward();
      break;

    case 1:
      setupMotorDefaut(90, 167 , 113 , 210);
      //偵測右方盆栽
      lcd.setCursor(8, 1);
      lcd.print("Pot:");
      lcd.print(count);

      if (ultR.distanceCM() < 50 && count < 3) {
        delay(600);
        irrigateRightPot();
        count++;
      }
      detectChangeStage();
      break;

    case 2:
      setupMotorDefaut(90, 167 , 113 , 210);
      detectChangeStage();
      break;

    case 3:
      setupMotorDefaut(90, 167 , 113 , 210);
      break;

    case 4:
      setupMotorDefaut(90, 167 , 113 , 210);
      detectChangeStage();
      break;

    case 5:
      setupMotorDefaut(90, 167 , 113 , 210);
      detectChangeStage();
      break;

    case 6:
      setupMotorDefaut(90, 167 , 113 , 210);
      detectChangeStage();
      break;

    case 7:
      setupMotorDefaut(90, 167 , 113 , 210);
      if (ultR.distanceCM() < 50 && count < 3) {
        delay(600);
        catchRightPot();
        count++;
      }

      detectChangeStage();
      break;

    case 8:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clear!");
      lcd.setCursor(1, 0);
      lcd.print("Fuck all!");
      goStop();
      while (true);
  }
}

///////////////////////////////////   偵測切換關卡   ///////////////////////////////////
void detectChangeStage() {
  const int changeFrontDis = 35;         //切換關卡前方距離
  const int changeSideDis = 50;          //切換關卡側方距離

  if (ultF.distanceCM() < changeFrontDis && ultL.distanceCM() < changeSideDis) {
    rotateToAngle(85);
    mpuInit();
    delay(1000);
    stage++;
  }
  else if (ultF.distanceCM() < changeFrontDis && ultR.distanceCM() < changeSideDis) {
    rotateToAngle(-85);
    mpuInit();
    delay(1000);
    stage++;
  }
}

///////////////////////////////////   走直線   ///////////////////////////////////
void fixStraight() {
  const int fixInterval = 1;                  //角度修正區間
  const int moreFixInterval = 40;             //加強角度修正區間
  const int fixMaxAngle = 90;                 //最大修正角度
  int angle = mpuGetAngle();

  if (abs(angle) > fixMaxAngle) angle = fixMaxAngle * angle / abs(angle);
  if (abs(angle) <= moreFixInterval) angle = moreFixInterval * angle / abs(angle);
  if (abs(angle) <= fixInterval) angle = 0;

  if (angle >= 0) {
    motLF.output(motLF.defaut - angle * motLF.defaut / 90);
    motLB.output(motLB.defaut - angle * motLB.defaut / 90);
    motRF.output(motRF.defaut + angle * (255 - motRF.defaut) / 90);
    motRB.output(motRB.defaut + angle * (255 - motRB.defaut) / 90);
  }
  else {
    motLF.output(motLF.defaut + abs(angle) * (255 - motLF.defaut) / 90);
    motLB.output(motLB.defaut + abs(angle) * (255 - motLB.defaut) / 90);
    motRF.output(motRF.defaut - abs(angle) * motRF.defaut / 90);
    motRB.output(motRB.defaut - abs(angle) * motRB.defaut / 90);
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
    motLF.fwd();
    motLB.fwd();
    motRF.output(motRF.defaut - motRF.defaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    motRB.output(motRB.defaut - motRB.defaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    return true;
  }
  else if (distanceR < fixDistance) {
    motLF.output(motLF.defaut - motLF.defaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motLB.output(motLB.defaut - motLB.defaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motRF.fwd();
    motRB.fwd();
    return true;
  }
  else return false;
}

///////////////////////////////////   澆灌右邊盆栽   ///////////////////////////////////
void irrigateRightPot() {
  const int potFindDistance = 50;                //盆栽尋找距離
  const int potIrrigateDistance = 20;            //澆灌距離
  float distance;

  //逆時針旋轉尋找盆栽
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L-Rotate");
  lcd.setCursor(0, 1);
  lcd.print("Find Pot");

  motLF.back();
  motLB.back();
  motRF.fwd();
  motRB.fwd();
  while (ultB.distanceCM() > potFindDistance);
  delay(300);

  //停止
  goStop();
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
  goStop();
  delay(1000);

  //澆水
  watering(7000);
  delay(1000);

  //前進
  goForward();
  delay(500);

  //旋轉車體回正
  rotateToAngle(1);

  //前進
  goForward();
  delay(1000);
}

///////////////////////////////////   抓取右邊盆栽   ///////////////////////////////////
void catchRightPot() {
  const int potFindDistance = 50;                //盆栽尋找距離
  const int potCatchDistance = 12;            //澆灌距離
  float distance;

  //逆時針旋轉尋找盆栽
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L-Rotate");
  lcd.setCursor(0, 1);
  lcd.print("Find Pot");

  motLF.back();
  motLB.back();
  motRF.fwd();
  motRB.fwd();
  while (ultB.distanceCM() > potFindDistance);
  delay(300);

  //停止
  goStop();
  delay(1000);

  //調整距離
  do {
    distance = ultB.distanceCM();
    if (distance < potCatchDistance)
      goForward();
    if (distance > potCatchDistance)
      goBack();
  } while (distance != potCatchDistance);

  //停車
  goStop();
  delay(1000);

  //抓取
  serB.write(80);
  delay(1000);
  serL.write(120);
  serR.write(60);
  delay(1000);
  serB.write(140);
  delay(1000);

  //前進
  goForward();
  delay(500);

  //旋轉車體回正
  rotateToAngle(1);

  //前進
  goForward();
  delay(1000);
}

///////////////////////////////////   其他   ///////////////////////////////////
void waitForStart() {
  while (!digitalRead(36)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Device ready!");
    lcd.setCursor(0, 1);
    lcd.print("Stage:");
    lcd.print(stage);

    if (digitalRead(35)) {
      stage++;
      if (stage > 7) stage = 0;
      while (digitalRead(35));
    }

    delay(200);
  }
  while (digitalRead(36));
}

void waitForPause() {
  if (digitalRead(36)) {
    while (digitalRead(36));

    goStop();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pause");

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

void setupMotorDefaut(int LF, int LB, int RF, int RB) {
  motLF.defaut = LF;
  motLB.defaut = LB;
  motRF.defaut = RF;
  motRB.defaut = RB;
}

void goForward() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Forward");

  motLF.fwd();
  motRF.fwd();
  motLB.fwd();
  motRB.fwd();
}

void goBack() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Back");

  motLF.back();
  motRF.back();
  motLB.back();
  motRB.back();
}

void goStop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop");

  motLF.stop();
  motRF.stop();
  motLB.stop();
  motRB.stop();
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
      motLF.back();
      motLB.back();
      motRF.fwd();
      motRB.fwd();
    }

    if (angle < rotationAngle) {
      motLF.fwd();
      motLB.fwd();
      motRF.back();
      motRB.back();
    }

  } while (angle != rotationAngle);

  goStop();
}
