/* commands */
#define CHIP_ERASE            0x80
#define WRITE_FUSE            0x40
#define WRITE_LOCK_BITS       0x20
#define WRITE_FLASH           0x10
#define WRITE_EEPROM          0x11
#define READ_SIGN             0x08  
#define READ_FUSE_LOCK_BITS   0x04
#define READ_FLASH            0x02
#define READ_EEPROM           0x03
#define NO_OPER               0x00

/* input/output pins */
#define RDY_BSY               A0    /* analog input pin A0, high means ready, low means busy */
#define OE_N                  13
#define WR_N                  12
#define BS1                   11
#define XA0                   10
#define XA1                   9
#define BS2                   9
#define XTAL1                 8

#define PB7 7
#define PB6 6
#define PB5 5
#define PB4 4
#define PB3 3
#define PB2 2
#define PB1 A2
#define PB0 A1

#define PWR_ON A3

const uint8_t BUFFER_SIZE = 8;
bool power_on = 1;
bool enable_echo = 1;
byte address;
byte data;
char inChars[BUFFER_SIZE];

void setGpioAsInput() {
    for (int i = 2; i <= 7; i++) {
      pinMode(i, INPUT);
  }
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

void setGpioAsOutput() {
    for (int i = 2; i <= 7; i++) {
      pinMode(i, OUTPUT);
  }
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(RDY_BSY, INPUT);
  pinMode(OE_N,   OUTPUT);
  pinMode(WR_N,   OUTPUT);
  pinMode(BS1,    OUTPUT);
  pinMode(XA0,    OUTPUT);
  pinMode(XA1,    OUTPUT); /* BS2 is the same as XA1 */  
  pinMode(XTAL1,  OUTPUT);
  pinMode(PWR_ON,  OUTPUT);

  setGpioAsInput();

  digitalWrite(OE_N, HIGH);
  digitalWrite(WR_N, LOW);
  digitalWrite(BS1, LOW);
  digitalWrite(XA0, LOW);
  digitalWrite(XA1, LOW);
  digitalWrite(XTAL1, LOW);
  digitalWrite(PWR_ON,  LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Main Menu:");
  Serial.println("r - Read Options");
  Serial.println("c - Chip Erase");
  Serial.println("w - Write Options");
  Serial.println("x - Power On/Off device");
  Serial.println("e - Turn On/Off echo");
  while (true) {
    while (Serial.available() == 0); // wait for user input
    char command = Serial.read();
    switch (command) {
      case 'r':
        showReadMenu();
        return;
      case 'c':
        chipErase();
        break;
      case 'w':
        showWriteMenu();
        return;
      case 'x':
        powerUpDownDevice();
        break;
      case 'e':
        setEcho();
        break;
      default:
        return;
    }
  }
}

void showReadMenu() {
  while (true) {
    helpReadMenu();
    while (true) {
      while (Serial.available() == 0); // wait for user input
      char subCommand = Serial.read();
      switch (subCommand) {
        case '1':
          readSignatureBits();
          break;        
        case '2':
          readFuseLockBits();
          break;
        case '3':
          readFlash();      
          helpReadMenu();
          break;
        case '4':
          readEEPROM();
          helpReadMenu();
          break;
        case 'q':
          return;
        default:
          helpReadMenu();
          break;
      }
    }
  }
}

void helpReadMenu() {
    Serial.println("Read Options:");
    Serial.println("1 - Read Signature Bits");
    Serial.println("2 - Read Fuse/Lock Bits");
    Serial.println("3 - Read Flash");
    Serial.println("4 - Read EEPROM");
    Serial.println("q - Quit to Main Menu");
}

void showWriteMenu() {
  while (true) {
    helpWriteMenu();
    while (true) {
      while (Serial.available() == 0); // wait for user input
      char subCommand = Serial.read();
      switch (subCommand) {
        case '1':
          writeFuseMenu();
          helpWriteMenu();
          break;
        case '2':
          Serial.println(" Write Lock bits is not yet implemented");
          break;
        case '3':
          writeFlash();
          helpWriteMenu();
          break;
        case '4':
          writeEEPROM();
          helpWriteMenu();
          break;
        case 'q':
          return;
        default:
          helpWriteMenu();
      }
    }
  }
}

void helpWriteMenu() {
  Serial.println("Write Options:");
  Serial.println("1 - Write Fuse Bits");
  Serial.println("2 - Write Lock Bits");
  Serial.println("3 - Write Flash");
  Serial.println("4 - Write EEPROM");
  Serial.println("q - Quit to Main Menu");
}

void readSignatureBits() {
  Serial.println("Reading Signature Bits...");

  setControlLines(READ_SIGN, 0x00, 0);
  digitalWrite(BS1, LOW);
  Serial.print("Signature Byte 0: ");
  Serial.println(readData(), HEX);

  setControlLines(READ_SIGN, 0x01, 0);
  digitalWrite(BS1, LOW);
  Serial.print("Signature Byte 1: ");
  Serial.println(readData(), HEX);

  setControlLines(READ_SIGN, 0x02, 0);
  digitalWrite(BS1, LOW);
  Serial.print("Signature Byte 2: ");
  Serial.println(readData(), HEX);
}

void readFuseLockBits() {
  Serial.println("Reading Fuse/Lock Bits...");
  
  loadCommand(READ_FUSE_LOCK_BITS);
  digitalWrite(BS2, LOW);
  digitalWrite(BS1, LOW);
  Serial.print("Fuse Low Byte: ");
  Serial.println(readData(), HEX);

  digitalWrite(BS2, HIGH);
  digitalWrite(BS1, HIGH);
  Serial.print("Fuse High Byte: ");
  Serial.println(readData(), HEX);

  digitalWrite(BS2, HIGH);
  digitalWrite(BS1, LOW);
  Serial.print("Extended Fuse Byte: ");
  Serial.println(readData(), HEX);

  digitalWrite(BS2, LOW);
  digitalWrite(BS1, HIGH);
  Serial.print("Lock Bits: ");
  Serial.println(readData(), HEX);

  digitalWrite(BS2, LOW);
  digitalWrite(BS1, LOW);
}


void readFlash() {
  loadCommand(READ_FLASH);
  uint8_t dataByte;
  while (true) {
    Serial.println("Enter address (four bytes) to read Flash, enter 'q' to go back to read menu");  
    readLine();
    Serial.println();
    if (inChars[0] == 'q') {
      break;
    }
    address = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
    loadAddress(address, 1); // load high byte address
    address = (hexCharToByte(inChars[2]) << 4) | hexCharToByte(inChars[3]);
    loadAddress(address, 0); // load low byte address

    digitalWrite(OE_N, LOW);
    
    // read low byte data
    digitalWrite(BS1, LOW);
    dataByte = readData();

    if (dataByte < 0x10) {
      Serial.print('0');
    }
    Serial.print(dataByte, HEX);

    // read high byte
    digitalWrite(BS1, HIGH);
        dataByte = readData();

    if (dataByte < 0x10) {
      Serial.print('0');
    }
    Serial.println(dataByte, HEX);    
  }
  digitalWrite(OE_N, HIGH);
}

void readEEPROM() {
  loadCommand(READ_EEPROM);
  uint8_t dataByte;
  while (true) {
    Serial.println("Enter address (four bytes) to read EEPROM, enter 'q' to go back to read menu");  
    readLine();
    Serial.println();
    if (inChars[0] == 'q') {
      break;
    }
    address = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
    loadAddress(address, 1); // load high byte address
    address = (hexCharToByte(inChars[2]) << 4) | hexCharToByte(inChars[3]);
    loadAddress(address, 0); // load low byte address

    digitalWrite(OE_N, LOW);
    
    // read low byte data
    digitalWrite(BS1, LOW);
    dataByte = readData();

    if (dataByte < 0x10) {
      Serial.print('0');
    }
    Serial.println(dataByte, HEX);  
  }
  digitalWrite(OE_N, HIGH);
}


void chipErase() {
  Serial.println("Erasing Chip...");
  digitalWrite(XA1, HIGH);
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);  
  setData(CHIP_ERASE);
  pulseXTAL1();
  delayMicroseconds(1);
  pulseWR();
  Serial.println("Chip Erased.");
}

void writeFuseMenu() {
  while (true) {
    Serial.println("Write Fuse Options:");
    Serial.println("1 - Write Fuse Low Byte");
    Serial.println("2 - Write Fuse High Byte");
    Serial.println("3 - Write Extended Fuse Byte");
    Serial.println("q - Quit to Write Menu");

    while (Serial.available() == 0);
    char subCommand = Serial.read();
    switch (subCommand) {
      case '1':
        writeFuseLowByte();
        break;
      case '2':
        writeFuseHighByte();
        break;
      case '3':
        writeFuseExtendedByte();
        break;
      case 'q':
        return;
      default:
        continue;
    }
  }
 }

void writeFuseLowByte() {
  Serial.println("Enter fuse low byte (in hex):");
  while (Serial.available() == 0);
  //String input = Serial.readStringUntil('\n');
  readLine();  
  byte fuseByte = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
  
  Serial.print("Writing fuse low byte: ");
  Serial.println(fuseByte, HEX);
  
  loadCommand(WRITE_FUSE);
  loadData(fuseByte, 0);  
  digitalWrite(BS1, LOW);
  digitalWrite(BS2, LOW);  // Select low byte
  //setData(fuseByte);
  pulseWR();
  while (!isReady()) {
    delayMicroseconds(1);
  }
  resetWriteBytes();
  Serial.println("Fuse low byte written.");
}

void writeFuseHighByte() {
  Serial.println("Enter fuse high byte (in hex):");
  while (Serial.available() == 0);
  //String input = Serial.readStringUntil('\n');
  readLine();
  byte fuseByte = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
  
  Serial.print("Writing fuse high byte: ");
  Serial.println(fuseByte, HEX);
  
  loadCommand(WRITE_FUSE);
  loadData(fuseByte, 0);
  digitalWrite(BS1, HIGH);
  digitalWrite(BS2, LOW);  // Select high byte
  //setData(fuseByte);
  pulseWR();
  while (!isReady()) {
    delayMicroseconds(1);
  }
  resetWriteBytes();
  Serial.println("Fuse high byte written.");
}

void writeFuseExtendedByte() {
  Serial.println("Enter extended fuse byte (in hex):");
  while (Serial.available() == 0);
  //String input = Serial.readStringUntil('\n');
  readLine();
  byte fuseByte = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
  
  Serial.print("Writing extended fuse byte: ");
  Serial.println(fuseByte, HEX);
  
  loadCommand(WRITE_FUSE);
  loadData(fuseByte, 0);
  digitalWrite(BS1, LOW);
  digitalWrite(BS2, HIGH);  // Select extended byte
  //setData(fuseByte);
  pulseWR();
  while (!isReady()) {
    delayMicroseconds(1);
  }
  resetWriteBytes();
  Serial.println("Extended fuse byte written.");
}

void writeFlash() {
  loadCommand(WRITE_FLASH);
  uint8_t dataByte;
  while (true) {
    while (true) {
      Serial.println("Enter address low bytes to write Flash, enter 'q' to go back to write menu, enter 'w' to write page");  
      readLine();
      Serial.println();
      if (inChars[0] == 'q' | inChars[0] == 'w') {
        break;
      }
      address = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
      loadAddress(address, 0); // load low byte address

      Serial.println("Enter data (four bytes) to write Flash");
      readLine();
      Serial.println();

      data = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
      loadData(data, 0); // load data low byte
      data = (hexCharToByte(inChars[2]) << 4) | hexCharToByte(inChars[3]);
      loadData(data, 1); // load data high byte
    }

    if (inChars[0] == 'w') {
      Serial.println("Enter address high bytes to write page to Flash");  
      readLine();
      Serial.println();
      
      address = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
      loadAddress(address, 1); // load high byte address
      pulseWR();
      while (!isReady()) {
        delayMicroseconds(1);
      }
      Serial.println("Page written successfully");  
    }

    if (inChars[0] == 'q') {
      loadCommand(NO_OPER);
      return;
    }
  }
}

void writeEEPROM() {
  loadCommand(WRITE_EEPROM);
  uint8_t dataByte;
  while (true) {
    while (true) {
      Serial.println("Enter address high and low bytels to write EEPROM, enter 'q' to go back to write menu, enter 'w' to write page");  
      readLine();
      Serial.println();
      if (inChars[0] == 'q' | inChars[0] == 'w') {
        break;
      }
      address = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
      loadAddress(address, 1); // load high byte address
      address = (hexCharToByte(inChars[2]) << 4) | hexCharToByte(inChars[3]);
      loadAddress(address, 0); // load low byte address

      Serial.println("Enter data (two bytes) to write EEPROM");
      readLine();
      Serial.println();

      data = (hexCharToByte(inChars[0]) << 4) | hexCharToByte(inChars[1]);
      loadData(data, 0);
    }

    if (inChars[0] == 'w') {
      digitalWrite(BS1, LOW);  
      pulseWR();
      while (!isReady()) {
        delayMicroseconds(1);
      }
      Serial.println("EEPROM written successfully");  
    }

    if (inChars[0] == 'q') {
      loadCommand(NO_OPER);
      return;
    }
  }
}


void pulseXTAL1() {
   digitalWrite(XTAL1, HIGH);
   delayMicroseconds(1);
   digitalWrite(XTAL1, LOW);
}

void pulseWR() {
   digitalWrite(WR_N, LOW);
   delayMicroseconds(1);
   digitalWrite(WR_N, HIGH);
   while (!isReady()) {
      delayMicroseconds(1);
   }
  }

void loadCommand(uint8_t command) {  
  digitalWrite(OE_N, HIGH);
  digitalWrite(WR_N, HIGH);  
  digitalWrite(XA1, HIGH); // same as BS2
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);  
  setData(command);
  delayMicroseconds(1);
  pulseXTAL1();
}

void loadData(uint8_t data, bool high_byte) { 
  while (!isReady()) {
    delayMicroseconds(1);
  }
  digitalWrite(OE_N, HIGH);
  digitalWrite(WR_N, HIGH);  
  digitalWrite(XA1, LOW); // same as BS2
  digitalWrite(XA0, HIGH);
  digitalWrite(BS1, LOW);
  if (high_byte) {
    digitalWrite(BS1, HIGH);
  };
  setData(data);
  delayMicroseconds(1);
  pulseXTAL1();
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
}

void loadAddress(uint8_t addr, bool high_byte) {
  while (!isReady()) {
    delayMicroseconds(1);
  }
  digitalWrite(OE_N, HIGH);
  digitalWrite(WR_N, HIGH);  
  digitalWrite(XA1, LOW); // same as BS2
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
  if (high_byte) {
    digitalWrite(BS1, HIGH);
  };
  setData(addr);
  delayMicroseconds(1);
  pulseXTAL1();
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
}

void setControlLines(uint8_t command, uint8_t address, bool high_byte) {
  loadCommand(command);  
  loadAddress(address, high_byte);
  delayMicroseconds(1);

}

uint8_t readData() {
  setGpioAsInput();
  digitalWrite(OE_N, LOW);
  delayMicroseconds(1);
  uint8_t data = 0;
  data |= (digitalRead(A1) << 0);
  data |= (digitalRead(A2) << 1);
  for (int i = 2; i <= 7; i++) {
    data |= (digitalRead(i) << i);
  }
  digitalWrite(OE_N, HIGH);
  return data;
}

void setData(uint8_t data) {
  setGpioAsOutput();
  for (int i = 2; i <= 7; i++) {
    digitalWrite(i, (data >> i) & 0x01);
  }
  digitalWrite(A1, data & 0x01);
  digitalWrite(A2, (data >> 1) & 0x01);
}


void powerUpDownDevice() {
  if (power_on == true) {
      power_on = powerDownDevice();
  } else {
      power_on = powerUpDevice();
  }
}

void setEcho() {
  if (enable_echo) {
    enable_echo = false;
    Serial.println("Turn off echo.");
  } else {
    enable_echo = true;
    Serial.println("Turn on echo.");
  }
}

bool powerUpDevice() {
  setGpioAsInput();
  digitalWrite(OE_N, LOW);
  digitalWrite(WR_N, LOW);
  digitalWrite(PWR_ON, HIGH);
  Serial.println("Powering on ...");
  return 1;
}

bool powerDownDevice() {
  digitalWrite(PWR_ON, LOW);
  setGpioAsInput();
  digitalWrite(OE_N, LOW);
  digitalWrite(WR_N, LOW);  
  Serial.println("Powering off ...");
  return 0;
}

bool isReady() {
  return digitalRead(RDY_BSY) == HIGH;
}

void resetWriteBytes(){
  setGpioAsInput();
  digitalWrite(XA1, LOW); // same as BS2
  digitalWrite(XA0, LOW);
  digitalWrite(BS1, LOW);
}

bool isHexadecimalDigit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

byte hexCharToByte(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0; // This should not happen if isHexadecimalDigit is correctly used
}

int readLine() {
  char incomingChar;
  int i = 0;
  String line_string;
  
  while (i < BUFFER_SIZE) {
    if (Serial.available() > 0) {
      incomingChar= Serial.read();
      if (incomingChar == '\n' || incomingChar == '\r') {
        break;
      }
      if (i == (BUFFER_SIZE - 1)) {        
        break;
      }
      if (incomingChar == 8 || incomingChar == 127) {
        if (i > 0) {
          i--;
          incomingChar = '\0';
          Serial.print("\b \b");
          continue;
        }
      }
      if (enable_echo) Serial.print(incomingChar);
      inChars[i] = incomingChar;
      i += 1;
    }
  }
  return i;
}