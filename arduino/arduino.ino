#define DLX3416_DELAY_US 10
const int blankPin = 5;

// Pin connections for the MC14094B shift registers
const int dataPin = 4;   // Connect to DS (Serial Data Input)
const int clockPin = 3;  // Connect to SH_CP (Clock Input)
const int latchPin = 2;  // Connect to ST_CP (Latch/Storage Register Clock Input)

typedef struct {
  bool d0_data;
  bool d1_data;
  bool d2_data;
  bool d3_data;
  bool d4_data;
  bool d5_data;
  bool d6_data;
  bool n_clr;
  bool n_wr;
  bool a1_dig;
  bool a0_dig;
  bool n_d2_enable;
  bool n_d3_enable;
  bool n_d1_enable;
} DisplayPins ;

void setup() {
  // Set the pins as outputs
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(blankPin, OUTPUT);

  digitalWrite(blankPin, LOW);

  while (!Serial) {}

  Serial.begin(9600, SERIAL_8N1);

}

void loop() {
char *curString = (char *)malloc(sizeof(char) * 13);
curString[12] = '\0';

clearDisplay();

while(1){
    if (Serial.available() > 0) {
        char receivedMessage[13]; // 12 characters + null terminator
        int bytesRead = Serial.readBytesUntil('\n', receivedMessage, sizeof(receivedMessage));
        receivedMessage[13] = '\0';
        if (bytesRead == 13) {
        //Serial.println("Writing to display..");
        //Serial.println(receivedMessage);  // Print the received message for debugging
        //Serial.println(curString);  // Print the received message for debugging
        writeStrToDisplay(receivedMessage, curString);     
        } else {
          //Serial.print("Message len:");
          //Serial.println(bytesRead);
        }
    }
}
}

void clearDisplay() {
  digitalWrite(blankPin, LOW);
    DisplayPins dp;
    dp.n_clr = false;
    dp.n_wr = false;

    writeToDisplay(&dp);
    delayMicroseconds(DLX3416_DELAY_US);

  digitalWrite(blankPin, HIGH);
}

void writeStrToDisplay(String str, char *curString) {
  digitalWrite(blankPin, LOW);

  for (int i = 0; i < 12; ++i) {

    if (curString[i] == str[i] && i != 7) {
      continue;
    }
    curString[i] = str[i];
    DisplayPins dp;
    charToData(str[i], &dp);

    dp.n_d1_enable = true;
    dp.n_d2_enable = true;
    dp.n_d3_enable = true;

    if (i >= 4 && i <= 7) {
      dp.n_d2_enable = false;
    } else if (i >= 8 && i <= 11) {
      dp.n_d3_enable = false;
    } else {
      dp.n_d1_enable = false;
    }

    switch (i % 4) {
      case 0:
        dp.a1_dig = true;
        dp.a0_dig = true;
        break;
      case 1:
        dp.a1_dig = true;
        dp.a0_dig = false;
        break;
      case 2:
        dp.a1_dig = false;
        dp.a0_dig = true;
        break;
      case 3:
        dp.a1_dig = false;
        dp.a0_dig = false;
        break;
    }

    dp.n_clr = true;
    dp.n_wr = false;

    writeToDisplay(&dp);
    delayMicroseconds(DLX3416_DELAY_US);

    dp.n_wr = true;
    writeToDisplay(&dp);
    delayMicroseconds(DLX3416_DELAY_US);
  }
  
  digitalWrite(blankPin, HIGH);
  Serial.println();  // Print a newline after displaying the string
}

void charToData(char c, DisplayPins *dp) {
  char array[8][17] = {{"ì⬆➡⬇⬅¿àØøòùñçêÉé"},   {"èÆæÅåÄäÖöÜü℃℉ß£¥"},
                       {" !\"#$%&\'()*+,-./"}, {"0123456789:;<=>?"},
                       {"@ABCDEFGHIJKLMNO"},   {"PQRSTUVWXYZ[\]^_"},
                       {"'abcdefghijklmno"},   {"pqrstuvwxyz{|}~"}};
  dp->d0_data = false;
  dp->d1_data = false;
  dp->d2_data = false;
  dp->d3_data = false;
  dp->d4_data = false;
  dp->d5_data = false;
  dp->d6_data = false;

  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 16; ++j) {
      if (array[i][j] == c) {
        if (i - 4 >= 0) {
          i -= 4;
          dp->d6_data = true;
        }
        if (i - 2 >= 0) {
          i -= 2;
          dp->d5_data = true;
        }
        if (i - 1 >= 0) {
          i -= 1;
          dp->d4_data = true;
        }
        if (j - 8 >= 0) {
          j -= 8;
          dp->d3_data = true;
        }
        if (j - 4 >= 0) {
          j -= 4;
          dp->d2_data = true;
        }
        if (j - 2 >= 0) {
          j -= 2;
          dp->d1_data = true;
        }
        if (j - 1 >= 0) {
          j -= 1;
          dp->d0_data = true;
        }
        return;
      }
    }
  }
}

void writeToDisplay(DisplayPins *displayPins) {

  // Combine data1 and data2 into a single byte
  uint16_t data = convertDisplayPinsToBytes(displayPins);

  // Send data to the shift registers
  //shiftOut(dataPin, clockPin, LSBFIRST, data);

  shiftOut(dataPin, clockPin, LSBFIRST, highByte(data));
  shiftOut(dataPin, clockPin, LSBFIRST, lowByte(data));
  
  // Latch the data to update the output
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(DLX3416_DELAY_US);
  digitalWrite(latchPin, LOW);
}

//void writeToDisplay(DisplayPins *dp) {
//  int16_t data = (dp->d0_data ? 32768 : 0) + (dp->d1_data ? 16384 : 0) +
//                  (dp->d2_data ? 8192 : 0) + (dp->d3_data ? 4096 : 0) +
//                  (dp->d4_data ? 2048 : 0) + (dp->d5_data ? 1024 : 0) +
//                  (dp->d6_data ? 512 : 0) + (!dp->clr ? 256 : 0) +
//                  (!dp->wr ? 128 : 0) + (dp->a1_dig ? 64 : 0) +
//                  (dp->a0_dig ? 32 : 0) + (!dp->d2_enable ? 16 : 0) +
//                  (!dp->d3_enable ? 8 : 0) + (!dp->d1_enable ? 4 : 0);

uint16_t convertDisplayPinsToBytes(const DisplayPins *displayPins) {
      byte data1 = (displayPins->d0_data ?      0B10000000 : 0) |
                        (displayPins->d1_data ? 0B01000000 : 0) |
                        (displayPins->d2_data ? 0B00100000 : 0) |
                        (displayPins->d3_data ? 0B00010000 : 0) |
                        (displayPins->d4_data ? 0B00001000 : 0) |
                        (displayPins->d5_data ? 0B00000100 : 0) |
                        (displayPins->d6_data ? 0B00000010 : 0) |
                        (displayPins->n_clr ?   0B00000001 : 0);

      byte data2 =  (displayPins->n_wr ?        0B10000000 : 0) |
                    (displayPins->a1_dig ?      0B01000000 : 0) |
                    (displayPins->a0_dig ?      0B00100000 : 0) |
                    (displayPins->n_d2_enable ? 0B00010000 : 0) |
                    (displayPins->n_d3_enable ? 0B00001000 : 0) |
                    (displayPins->n_d1_enable ? 0B00000100 : 0);
  
  uint16_t data = (data2 << 8) | data1;

  return data;
}
