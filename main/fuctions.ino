///////////////////////////////////   判斷關卡特殊事件   ///////////////////////////////////
void stageEvent() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stage:");
  lcd.print(stage);

  switch (stage) {
    case 0:
      break;

    case 1:
      //偵測右方盆栽
      lcd.print(" Pot:");
      lcd.print(potCount);

      if (ultR.distanceCM() < 50 && potCount < 3) {
        if (skipPot == 0) {
          delay(600);
          goStop();
          findBackPot(50);
          delay(1000);
          fixBackDis(20);
          delay(1000);
          watering(7000);
          delay(1000);
          goForward();
          delay(500);
          rotateToAngle(1);
          goForward();
          delay(1000);
          potCount++;
        }
        else
          skipPot--;
      }

      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        rotateToAngle(82);
        mpuInit();
        delay(1000);
        stage++;
      }
      break;

    case 2:
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        rotateToAngle(82);
        mpuInit();
        delay(1000);
        stage++;
      }
      break;

    case 3:
      break;

    case 4:
      if (ultF.distanceCM() < 35 && ultR.distanceCM() < 50) {
        rotateToAngle(-82);
        mpuInit();
        delay(1000);
        stage++;
      }
      break;

    case 5:
      break;

    case 6:
      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        rotateToAngle(82);
        mpuInit();
        delay(1000);
        stage++;
      }
      break;

    case 7:
      if (ultR.distanceCM() < 50 && potCount < 3) {
        delay(600);
        goStop();
        findBackPot(50);
        delay(1000);
        fixBackDis(20);
        delay(1000);
        watering(7000);
        delay(1000);
        goForward();
        delay(500);
        rotateToAngle(1);
        goForward();
        delay(1000);
        potCount++;
      }

      if (ultF.distanceCM() < 35 && ultL.distanceCM() < 50) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Clear!");
        lcd.setCursor(1, 0);
        lcd.print("Fuck all!");
        goStop();
        while (true);
      }
      break;
  }
}

///////////////////////////////////   走直線   ///////////////////////////////////
void fixStraight() {
  const int fixInterval = 1;                  //角度修正區間
  const int moreFixInterval = 40;             //加強角度修正區間
  const int fixMaxAngle = 90;                 //最大修正角度
  int angle = mpuGetAngle();

  lcd.setCursor(0, 1);
  lcd.print("Angle:");
  lcd.print(angle);

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
    lcd.setCursor(0, 1);
    lcd.print("DisL:");
    lcd.print(distanceL);
    motLF.fwd();
    motLB.fwd();
    motRF.output(motRF.defaut - motRF.defaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    motRB.output(motRB.defaut - motRB.defaut / (fixDistance - minDistance) * (fixDistance - distanceL));
    return true;
  }
  else if (distanceR < fixDistance) {
    lcd.setCursor(0, 1);
    lcd.print("DisR:");
    lcd.print(distanceR);
    motLF.output(motLF.defaut - motLF.defaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motLB.output(motLB.defaut - motLB.defaut / (fixDistance - minDistance) * (fixDistance - distanceR));
    motRF.fwd();
    motRB.fwd();
    return true;
  }
  else return false;
}

///////////////////////////////////   尋找後方盆栽   ///////////////////////////////////
void findBackPot(int dis) {
  lcd.setCursor(0, 0);
  lcd.print("Find Pot");
  lcd.setCursor(0, 1);
  lcd.print(dis);
  lcd.print("/");
  lcd.print(ultB.distanceCM());

  motLF.back();
  motLB.back();
  motRF.fwd();
  motRB.fwd();
  while (ultB.distanceCM() > dis);
  delay(300);
  goStop();
}

///////////////////////////////////   調整後方距離   ///////////////////////////////////
void fixBackDis(int dis) {
  lcd.setCursor(0, 0);
  lcd.print("Fix Dis");
  lcd.setCursor(0, 1);
  lcd.print(dis);
  lcd.print("/");
  lcd.print(ultB.distanceCM());

  do {
    if (ultB.distanceCM() < dis)
      goForward();
    if (ultB.distanceCM() > dis)
      goBack();
  } while (ultB.distanceCM() != dis);
  goStop();
}

///////////////////////////////////   澆水   ///////////////////////////////////
void watering(int time) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water");
  lcd.setCursor(0, 1);
  lcd.print(time / 1000);
  lcd.print("s");

  digitalWrite(34, HIGH);
  delay(time);
  digitalWrite(34, LOW);
}

///////////////////////////////////   夾盆栽   ///////////////////////////////////
void catchPot() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Catch Pot");

  serL.write(120);
  serR.write(60);
  delay(1000);
  serB.write(80);
  delay(1000);
  serL.write(85);
  serR.write(90);
  delay(1000);
  serB.write(140);
  delay(5000);
}

///////////////////////////////////   開始暫停鈕   ///////////////////////////////////
void waitForStart() {
  while (!digitalRead(4)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stage:");
    lcd.print(stage);

    if (digitalRead(35)) {
      stage++;
      if (stage > 7)
        stage = 0;
      while (digitalRead(35));
    }
    delay(100);
  }
  while (digitalRead(4));

  if (stage == 1) {
    while (!digitalRead(4)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stage:1");
      lcd.setCursor(0, 1);
      lcd.print("Skip Pot:");
      lcd.print(skipPot);
      if (digitalRead(35)) {
        skipPot++;
        if (skipPot > 3)
          skipPot = 0;
        while (digitalRead(35));
      }
      delay(100);
    }
    while (digitalRead(4));
  }
}

void pause() {

    goStop();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pause");

    while (!digitalRead(4));
    while (digitalRead(4));
}

///////////////////////////////////   車體動作   ///////////////////////////////////

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
  const int error = 20;
  int angle;
  do {
    angle = mpuGetAngle();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rotate");
    lcd.setCursor(0, 1);
    lcd.print(rotationAngle);
    lcd.print("/");
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
  delay(1500);
}
