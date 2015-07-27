int display[13][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
/*
// Say Hi!
int display[13][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 1, 0, 1, 1, 1},
  {1, 0, 0, 1, 0, 0, 1, 0},
  {1, 0, 0, 1, 0, 0, 1, 0},
  {1, 1, 1, 1, 0, 0, 1, 0},
  {1, 1, 1, 1, 0, 0, 1, 0},
  {1, 0, 0, 1, 0, 0, 1, 0},
  {1, 0, 0, 1, 0, 0, 1, 0},
  {1, 0, 0, 1, 0, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
*/
int pinY[5] = {
  B00001000, B00010000, B00100000, B01000000, B10000000
};

struct block {
  int n1x; // this node is the center of rotation
  int n1y;
  int n2x;
  int n2y;
  int n3x;
  int n3y;
  int n4x;
  int n4y;
};

// declare IO pins
int buttonRotate = A5;
int buttonLeft = A3;
int buttonRight = A1;
int enable = 9;
int dataX = 8;
int dataY1 = 6;
int dataY2 = 7;
int mr = 3;
int cp = 4;

// misc variables
int temp, blockDown, blockActive, shiftVal, temp2;
block curr;

unsigned long time1, time2;

// delay timers
#define DISPLAY_TIME 2
#define BUTTON_DELAY 100

void setup() {
  Serial.begin(9600);
  pinMode(buttonRotate, INPUT);
  pinMode(buttonLeft, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(enable, OUTPUT);
  pinMode(dataX, OUTPUT);
  pinMode(dataY1, OUTPUT);
  pinMode(dataY2, OUTPUT);
  pinMode(mr, OUTPUT);
  pinMode(cp, OUTPUT);

  digitalWrite(enable, 0);
  digitalWrite(cp, 0);

  clearDisplay();

  blockDown = 200;
  blockActive = 0;

  randomSeed(analogRead(0));

  time1 = millis();
  time2 = millis();
}

void loop() {
  // check for active block, if no active block create random block
  if (blockActive == 0) {
    generateBlock(random(6));
  }

  // check for inputs
  if (digitalRead(buttonRotate) == 0 && millis() - time2 > BUTTON_DELAY) {
    rotateBlock();
    time2 = millis();
  }
  else if (digitalRead(buttonLeft) == 0 && millis() - time2 > BUTTON_DELAY) {
    moveBlockLeft();
    time2 = millis();
  }
  else if (digitalRead(buttonRight) == 0 && millis() - time2 > BUTTON_DELAY) {
    moveBlockRight();
    time2 = millis();
  }

  // shift blocks down use millis() here instead...
  if ((millis() - time1) > blockDown) {
    if (moveBlockDown() == 1) {
      blockActive = 0;
    }
    time1 = millis();
  }

  // refresh display
  refresh();
}

void enableDisplay() {
  digitalWrite(enable, 1);
  return;
}

void disableDisplay() {
  digitalWrite(enable, 0);
  return;
}

void dataOut(int dataPinX, int dataPinY, int dataShiftX, int dataShiftY) {
  digitalWrite(cp, 0);
  for (int k = 0; k < 8; k++) {
    shiftVal = dataShiftX & B00000001;
    digitalWrite(dataPinX, shiftVal);
    shiftVal = dataShiftY & B00000001;
    digitalWrite(dataPinY, shiftVal);
    digitalWrite(cp, 1);
    digitalWrite(cp, 0);
    dataShiftX = dataShiftX >> 1;
    dataShiftY = dataShiftY >> 1;
  }
  return;
}

void refresh() {
  for (int i = 0; i < 5; i++) {
    temp = 0;
    for (int j = 7; j >= 0; j--) {
      temp = temp << 1;
      temp = display[i][j] | temp;
    }
    disableDisplay();
    dataOut(dataX, dataY1, temp, pinY[i]);
    enableDisplay();
    delay(DISPLAY_TIME);
  }
  digitalWrite(dataY1, 0);
  for (int i = 5; i < 10; i++) {
    temp = 0;
    for (int j = 7; j >= 0; j--) {
      temp = temp << 1;
      temp = display[i][j] | temp;
    }
    disableDisplay();
    dataOut(dataX, dataY2, temp, pinY[i - 5]);
    enableDisplay();
    delay(DISPLAY_TIME);
  }
  digitalWrite(dataY2, 0);
  return;
}

void clearDisplay() {
  digitalWrite(mr, 0);
  digitalWrite(mr, 1);
  return;
}

/* Rotation Matrix
	| x'| = | 0 -1 | | x |
	| y'| = | 1  0 | | y |
*/

// need to protect against going outside of boundaries

int rotateBlock() {
  // check if rotate is possible
  // check node2
  if (display[curr.n1y + (curr.n2x - curr.n1x)][curr.n1x - (curr.n2y - curr.n1y)]) {
    return 1;
  }
  // check node3
  if (display[curr.n1y + (curr.n3x - curr.n1x)][curr.n1x - (curr.n3y - curr.n1y)]) {
    if (curr.n1x - (curr.n3y - curr.n1y) == curr.n2x && curr.n1y + (curr.n3x - curr.n1x) == curr.n2y) {
    }
    else {
      return 1;
    }
  }
  // check node4
  if (display[curr.n1y + (curr.n4x - curr.n1x)][curr.n1x - (curr.n4y - curr.n1y)]) {
    if (curr.n1x - (curr.n4y - curr.n1y) == curr.n2x && curr.n1y + (curr.n4x - curr.n1x) == curr.n2y) {
    }
    else if (curr.n1x - (curr.n4y - curr.n1y) == curr.n3x && curr.n1y + (curr.n4x - curr.n1x) == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // move points and update struct
  // change node2
  display[curr.n2y][curr.n2x] = 0;
  temp2 = curr.n2x;
  curr.n2x = curr.n1x - (curr.n2y - curr.n1y);
  curr.n2y = curr.n1y + (temp2 - curr.n1x);
  // change node3
  display[curr.n3y][curr.n3x] = 0;
  temp2 = curr.n3x;
  curr.n3x = curr.n1x - (curr.n3y - curr.n1y);
  curr.n3y = curr.n1y + (temp2 - curr.n1x);
  // change node4
  display[curr.n4y][curr.n4x] = 0;
  temp2 = curr.n3x;
  curr.n4x = curr.n1x - (curr.n4y - curr.n1y);
  curr.n4y = curr.n1y + (temp2 - curr.n1x);
  display[curr.n2y][curr.n2x] = 1;
  display[curr.n3y][curr.n3x] = 1;
  display[curr.n4y][curr.n4x] = 1;

  return 0;

}

int moveBlockRight() {
  // check if move is possible
  // check node1
  if (display[curr.n1y][curr.n1x + 1]) {
    if (curr.n1x + 1 == curr.n2x && curr.n1y == curr.n2y) {
    }
    else if (curr.n1x + 1 == curr.n3x && curr.n1y == curr.n3y) {
    }
    else if (curr.n1x + 1 == curr.n4x && curr.n1y == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node2
  if (display[curr.n2y][curr.n2x + 1]) {
    if (curr.n2x + 1 == curr.n1x && curr.n2y == curr.n1y) {
    }
    else if (curr.n2x + 1 == curr.n3x && curr.n2y == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // check node3
  if (display[curr.n3y][curr.n3x + 1]) {
    if (curr.n3x + 1 == curr.n1x && curr.n3y == curr.n1y) {
    }
    else if (curr.n3x + 1 == curr.n2x && curr.n3y == curr.n2y) {
    }
    else if (curr.n3x + 1 == curr.n4x && curr.n3y == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node4
  if (display[curr.n4y][curr.n4x + 1]) {
    if (curr.n4x + 1 == curr.n1x && curr.n4y == curr.n1y) {
    }
    else if (curr.n4x + 1 == curr.n3x && curr.n4y == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // move points and update struct
  // change node1
  display[curr.n1y][curr.n1x] = 0;
  curr.n1x = curr.n1x + 1;
  // change node2
  display[curr.n2y][curr.n2x] = 0;
  curr.n2x = curr.n2x + 1;
  // change node3
  display[curr.n3y][curr.n3x] = 0;
  curr.n3x = curr.n3x + 1;
  // change node4
  display[curr.n4y][curr.n4x] = 0;
  curr.n4x = curr.n4x + 1;
  display[curr.n1y][curr.n1x] = 1;
  display[curr.n2y][curr.n2x] = 1;
  display[curr.n3y][curr.n3x] = 1;
  display[curr.n4y][curr.n4x] = 1;
  return 0;
}

int moveBlockLeft() {
  // check if move is possible
  // check node1
  if (display[curr.n1y][curr.n1x - 1]) {
    if (curr.n1x - 1 == curr.n2x && curr.n1y == curr.n2y) {
    }
    else if (curr.n1x - 1 == curr.n3x && curr.n1y == curr.n3y) {
    }
    else if (curr.n1x - 1 == curr.n4x && curr.n1y == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node2
  if (display[curr.n2y][curr.n2x - 1]) {
    if (curr.n2x - 1 == curr.n1x && curr.n2y == curr.n1y) {
    }
    else if (curr.n2x - 1 == curr.n3x && curr.n2y == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // check node3
  if (display[curr.n3y][curr.n3x - 1]) {
    if (curr.n3x - 1 == curr.n1x && curr.n3y == curr.n1y) {
    }
    else if (curr.n3x - 1 == curr.n2x && curr.n3y == curr.n2y) {
    }
    else if (curr.n3x - 1 == curr.n4x && curr.n3y == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node4
  if (display[curr.n4y][curr.n4x + 1]) {
    if (curr.n4x + 1 == curr.n1x && curr.n4y == curr.n1y) {
    }
    else if (curr.n4x + 1 == curr.n2x && curr.n4y == curr.n2y) {
    }
    else if (curr.n4x + 1 == curr.n3x && curr.n4y == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // move points and update struct
  // change node1
  display[curr.n1y][curr.n1x] = 0;
  curr.n1x = curr.n1x - 1;
  // change node2
  display[curr.n2y][curr.n2x] = 0;
  curr.n2x = curr.n2x - 1;
  // change node3
  display[curr.n3y][curr.n3x] = 0;
  curr.n3x = curr.n3x - 1;
  // change node4
  display[curr.n4y][curr.n4x] = 0;
  curr.n4x = curr.n4x - 1;
  display[curr.n1y][curr.n1x] = 1;
  display[curr.n2y][curr.n2x] = 1;
  display[curr.n3y][curr.n3x] = 1;
  display[curr.n4y][curr.n4x] = 1;
  return 0;
}

int moveBlockDown() {
  // check if move is possible
  // check if at bottom
  if (curr.n1y < 1 || curr.n2y < 1 || curr.n3y < 1 || curr.n4y < 1) {
    return 1;
  }
  // check node1
  if (display[curr.n1y - 1][curr.n1x]) {
    if (curr.n1x == curr.n2x && curr.n1y - 1 == curr.n2y) {
    }
    else if (curr.n1x == curr.n3x && curr.n1y - 1 == curr.n3y) {
    }
    else if (curr.n1x == curr.n4x && curr.n1y - 1 == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node2
  if (display[curr.n2y - 1][curr.n2x]) {
    if (curr.n2x == curr.n1x && curr.n2y - 1 == curr.n1y) {
    }
    else if (curr.n2x == curr.n3x && curr.n2y - 1 == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // check node3
  if (display[curr.n3y - 1][curr.n3x]) {
    if (curr.n3x == curr.n1x && curr.n3y - 1 == curr.n1y) {
    }
    else if (curr.n3x == curr.n2x && curr.n3y - 1 == curr.n2y) {
    }
    else if (curr.n3x == curr.n4x && curr.n3y - 1 == curr.n4y) {
    }
    else {
      return 1;
    }
  }
  // check node4
  if (display[curr.n4y - 1][curr.n4x]) {
    if (curr.n4x == curr.n1x && curr.n4y - 1 == curr.n1y) {
    }
    else if (curr.n4x == curr.n3x && curr.n4y - 1 == curr.n3y) {
    }
    else {
      return 1;
    }
  }
  // move points and update struct
  // change node1
  display[curr.n1y][curr.n1x] = 0;
  curr.n1y = curr.n1y - 1;
  // change node2
  display[curr.n2y][curr.n2x] = 0;
  curr.n2y = curr.n2y - 1;
  // change node3
  display[curr.n3y][curr.n3x] = 0;
  curr.n3y = curr.n3y - 1;
  // change node4
  display[curr.n4y][curr.n4x] = 0;
  curr.n4y = curr.n4y - 1;
  display[curr.n1y][curr.n1x] = 1;
  display[curr.n2y][curr.n2x] = 1;
  display[curr.n3y][curr.n3x] = 1;
  display[curr.n4y][curr.n4x] = 1;
  return 0;
}

void generateBlock(long blockType) {
  // square
  if (blockType == 0) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 4;
    curr.n2y = 11;
    curr.n3x = 3;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 12;
  }
  // I
  else if (blockType == 1) {
    curr.n1x = 4;
    curr.n1y = 11;
    curr.n2x = 5;
    curr.n2y = 11;
    curr.n3x = 3;
    curr.n3y = 11;
    curr.n4x = 2;
    curr.n4y = 11;
  }
  // S
  else if (blockType == 2) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 5;
    curr.n2y = 12;
    curr.n3x = 4;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 11;
  }
  // Z
  else if (blockType == 3) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 5;
    curr.n2y = 11;
    curr.n3x = 4;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 12;
  }
  // L
  else if (blockType == 4) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 5;
    curr.n2y = 12;
    curr.n3x = 3;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 12;
  }
  // J
  else if (blockType == 5) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 5;
    curr.n2y = 12;
    curr.n3x = 5;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 12;
  }
  // T
  else if (blockType == 6) {
    curr.n1x = 4;
    curr.n1y = 12;
    curr.n2x = 5;
    curr.n2y = 12;
    curr.n3x = 4;
    curr.n3y = 11;
    curr.n4x = 3;
    curr.n4y = 12;
  }
  blockActive = 1;
  return;
}
