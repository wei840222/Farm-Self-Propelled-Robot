///////////////////////////////////   GLOBAL VALUES   ///////////////////////////////////
uint16_t packetSize;                    // expected DMP packet size (default is 42 bytes)
uint8_t fifoBuffer[64];                 // FIFO storage buffer

Quaternion q;                           // [w, x, y, z]         quaternion container
VectorFloat gravity;                    // [x, y, z]            gravity vector
float ypr[3];                           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

///////////////////////////////////   ACCELGTRO OFFSET CONFIGURATION   ///////////////////////////////////
//Change this 3 variables if you want to fine tune the skecth to your needs.
int buffersize = 1000;   //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone = 8;   //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone = 1;   //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int16_t ax, ay, az, gx, gy, gz;
int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;

int ax_offset = -391, ay_offset = 1507, az_offset = 1340, gx_offset = 64, gy_offset = -26, gz_offset = -70;
float angle_fix = -0.61;

///////////////////////////////////   FUNCTIONS   ///////////////////////////////////
void mpuInit() {
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // verify connection
  if (!mpu.testConnection())
    Serial.println("MPU6050 connection failed");

  mpu.initialize();

  // make sure it worked (returns 0 if so)
  if (mpu.dmpInitialize() != 0) {
    Serial.print("DMP Initialization failed (code ");
    Serial.print(mpu.dmpInitialize());
    Serial.println(")");
  }

  // turn on the DMP, now that it's ready
  mpu.setDMPEnabled(true);
  // get expected DMP packet size for later comparison
  packetSize = mpu.dmpGetFIFOPacketSize();

  // use the code below to change accel/gyro offset values
  mpu.setXAccelOffset(ax_offset);
  mpu.setYAccelOffset(ay_offset);
  mpu.setZAccelOffset(az_offset);
  mpu.setXGyroOffset(gx_offset);
  mpu.setYGyroOffset(gy_offset);
  mpu.setZGyroOffset(gz_offset);
}

float mpuGetAngle() {
  // holds actual interrupt status byte from MPU
  uint8_t mpuIntStatus = mpu.getIntStatus();

  // get count of all bytes currently in FIFO
  uint16_t fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    //Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize)
      fifoCount = mpu.getFIFOCount();
    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    return ypr[0] * 180 / M_PI - angle_fix;
  }
}

float calculateAngle() {
  const int num = 10;
  float angles[num * 4];
  float angle = 0;

  for (int i = 0; i < num * 4; i++)
    angles[i] = mpuGetAngle();

  stats.bubbleSort(angles, num * 4);

  for (int i = num; i < (num * 3 - 1); i++)
    angle += angles[i];

  angle /= (num * 2);

  if (angle < -180)
    angle += 360;
  return angle;
}

void mpuOffset() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  // COMMENT NEXT LINE IF YOU ARE USING ARDUINO DUE
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Leonardo measured 250kHz.

  // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer

  // start message
  Serial.println("Your MPU6050 should be placed in horizontal position, with package letters facing up.");
  Serial.println("Don't touch it until you see a finish message.");

  // reset offsets
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  angle_fix = 0;

  // calculate offset
  if (state == 0) {
    Serial.println("Reading sensors for first time...");
    meansensors();
    state++;
    delay(1000);
  }

  if (state == 1) {
    Serial.print("Calculating offsets...");
    calibration();
    state++;
    delay(1000);
  }

  if (state == 2) {
    meansensors();
    angleFix();
    Serial.println();
    Serial.println("Finished!");
    Serial.println("Sensor readings with offsets:");
    Serial.print("AX:");
    Serial.print(mean_ax);
    Serial.print("\tAY:");
    Serial.print(mean_ay);
    Serial.print("\tAZ:");
    Serial.println(mean_az);
    Serial.print("GX:");
    Serial.print(mean_gx);
    Serial.print("\tGY:");
    Serial.print(mean_gy);
    Serial.print("\tGZ:");
    Serial.println(mean_gz);
    Serial.println("Your offsets:");
    Serial.print("AX:");
    Serial.print(ax_offset);
    Serial.print("\tAY:");
    Serial.print(ay_offset);
    Serial.print("\tAZ:");
    Serial.println(az_offset);
    Serial.print("GX:");
    Serial.print(gx_offset);
    Serial.print("\tGY:");
    Serial.print(gy_offset);
    Serial.print("\tGZ:");
    Serial.println(gz_offset);
    Serial.print("Angle Fix:");
    Serial.println(angle_fix);
  }
}

void meansensors() {
  long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

  while (i < (buffersize + 101)) {
    // read raw accel/gyro measurements from device
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (i > 100 && i <= (buffersize + 100)) { //First 100 measures are discarded
      buff_ax = buff_ax + ax;
      buff_ay = buff_ay + ay;
      buff_az = buff_az + az;
      buff_gx = buff_gx + gx;
      buff_gy = buff_gy + gy;
      buff_gz = buff_gz + gz;
    }
    if (i == (buffersize + 100)) {
      mean_ax = buff_ax / buffersize;
      mean_ay = buff_ay / buffersize;
      mean_az = buff_az / buffersize;
      mean_gx = buff_gx / buffersize;
      mean_gy = buff_gy / buffersize;
      mean_gz = buff_gz / buffersize;
    }
    i++;
    delay(2); //Needed so we don't get repeated measures
  }
}

void calibration() {
  ax_offset = -mean_ax / 8;
  ay_offset = -mean_ay / 8;
  az_offset = (16384 - mean_az) / 8;

  gx_offset = -mean_gx / 4;
  gy_offset = -mean_gy / 4;
  gz_offset = -mean_gz / 4;
  while (true) {
    int ready = 0;
    mpu.setXAccelOffset(ax_offset);
    mpu.setYAccelOffset(ay_offset);
    mpu.setZAccelOffset(az_offset);

    mpu.setXGyroOffset(gx_offset);
    mpu.setYGyroOffset(gy_offset);
    mpu.setZGyroOffset(gz_offset);

    meansensors();
    Serial.print("...");

    if (abs(mean_ax) <= acel_deadzone) ready++;
    else ax_offset = ax_offset - mean_ax / acel_deadzone;

    if (abs(mean_ay) <= acel_deadzone) ready++;
    else ay_offset = ay_offset - mean_ay / acel_deadzone;

    if (abs(16384 - mean_az) <= acel_deadzone) ready++;
    else az_offset = az_offset + (16384 - mean_az) / acel_deadzone;

    if (abs(mean_gx) <= giro_deadzone) ready++;
    else gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

    if (abs(mean_gy) <= giro_deadzone) ready++;
    else gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

    if (abs(mean_gz) <= giro_deadzone) ready++;
    else gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

    if (ready == 6) break;
  }
}

void angleFix() {
  float angles[10];
  mpuInit();
  for (int i = 0; i < 5; i++) {
    Serial.print("...");
    delay(1200);
  }
  for (int i = 0; i < 10; i++)
    angles[i] = calculateAngle();
  angle_fix = stats.average(angles,10);
}

