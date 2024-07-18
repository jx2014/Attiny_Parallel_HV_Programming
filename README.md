# Attiny_Parallel_HV_Programming

# Arduino-based ATtiny 2313A High Voltage Programmer

## Purpose

The purpose of this project is to reprogram the fuse and lock bits of an ATtiny 2313A microcontroller in case its serial interface (SPI) is disabled.

## Parts List

The parts list is included in the `at2313a_parallel_programmer.csv` file relative to this `README.md` file.

## Schematic

The schematic is provided in the `at2313a_parallel_programmer.pdf` file.

### Additional Components Not Included in the Netlist

- Arduino Perf board: [Amazon Link](https://www.amazon.com/dp/B01J1KM3RM?psc=1&ref=ppx_yo2ov_dt_b_product_details)
- 5V to 12V boost converter: [Amazon Link](https://www.amazon.com/dp/B089JYBF25?psc=1&ref=ppx_yo2ov_dt_b_product_details)

## Usage Instructions

1. **Program the Arduino**: Load the provided code onto the Arduino.
2. **Wire Up the Boards**: Connect the components according to the schematic provided.

### Quick Reference Wiring List

| ATtiny Pin | Description  | Arduino Pin |
|------------|--------------|-------------|
| 1          | ^RESET, 12.5V|             |
| 2          | NC           |             |
| 3          | RDY_BSY      | A0          |
| 4          | NC           |             |
| 5          | XTAL1        | D8          |
| 6          | ^OE          | D13         |
| 7          | ^WR          | D12         |
| 8          | BS1          | D11         |
| 9          | XA0          | D10         |
| 10         | GND          |             |
| 11         | BS2/XA1      | D9          |
| 12         | PB0          | A1          |
| 13         | PB1          | A2          |
| 14         | PB2          | D2          |
| 15         | PB3          | D3          |
| 16         | PB4          | D4          |
| 17         | PB5          | D5          |
| 18         | PB6          | D6          |
| 19         | PB7          | D7          |
| 20         | VCC, 5V+     |             |

Arduino A3 is used as `PWR_ON` to toggle the transistors. The transistors are only needed for automatically entering programming mode. Without the transistor, you can manually enter programming mode by following this sequence:
1. Keep all inputs to ATtiny2313A low (press x to power off).
2. Apply 12.5V to pin 1 (Reset).
3. Apply 5V to pin 20 (VCC). 
4. If successful, pin 3 of the ATtiny should go high.

## Programming Manual

### Main Menu

- `r` - Read Options
- `c` - Chip Erase
- `w` - Write Options
- `x` - Power On/Off device
- `q` - Quit

**Note**: Press `x` to power off and on the device. The LED or pin 3 should go high to indicate the IC is in parallel programming mode.

### Read Options

- `1` - Read Signature Bits
- `2` - Read Fuse/Lock Bits
- `3` - Read Flash
- `4` - Read EEPROM
- `q` - Quit to Main Menu

#### Example: Reading Signature Bits

Here is a sample README.md file for your Arduino-based ATtiny 2313A High Voltage Programmer project:

markdown
Copy code
# Arduino-based ATtiny 2313A High Voltage Programmer

## Purpose

The purpose of this project is to reprogram the fuse and lock bits of an ATtiny 2313A microcontroller in case its serial interface (SPI) is disabled.

## Parts List

The parts list is included in the `at2313a_parallel_programmer.csv` file relative to this `README.md` file.

## Schematic

The schematic is provided in the `at2313a_parallel_programmer.pdf` file.

### Additional Components Not Included in the Netlist

- Arduino Perf board: [Amazon Link](https://www.amazon.com/dp/B01J1KM3RM?psc=1&ref=ppx_yo2ov_dt_b_product_details)
- 5V to 12V boost converter: [Amazon Link](https://www.amazon.com/dp/B089JYBF25?psc=1&ref=ppx_yo2ov_dt_b_product_details)

## Usage Instructions

1. **Program the Arduino**: Load the provided code onto the Arduino.
2. **Wire Up the Boards**: Connect the components according to the schematic provided.

### Quick Reference Wiring List

| ATtiny Pin | Description  | Arduino Pin |
|------------|--------------|-------------|
| 1          | ^RESET, 12.5V|             |
| 2          | NC           |             |
| 3          | RDY_BSY      | A0          |
| 4          | NC           |             |
| 5          | XTAL1        | D8          |
| 6          | ^OE          | D13         |
| 7          | ^WR          | D12         |
| 8          | BS1          | D11         |
| 9          | XA0          | D10         |
| 10         | GND          |             |
| 11         | BS2/XA1      | D9          |
| 12         | PB0          | A1          |
| 13         | PB1          | A2          |
| 14         | PB2          | D2          |
| 15         | PB3          | D3          |
| 16         | PB4          | D4          |
| 17         | PB5          | D5          |
| 18         | PB6          | D6          |
| 19         | PB7          | D7          |
| 20         | VCC, 5V+     |             |

Arduino A3 is used as `PWR_ON` to toggle the transistors. The transistors are only needed for automatically entering programming mode. Without the transistor, you can manually enter programming mode by following this sequence:
1. Keep all inputs to ATtiny2313A low (press x to power off).
2. Apply 12.5V to pin 1 (Reset).
3. Apply 5V to pin 20 (VCC). 
4. If successful, pin 3 of the ATtiny should go high.

## Programming Manual

### Main Menu

- `r` - Read Options
- `c` - Chip Erase
- `w` - Write Options
- `x` - Power On/Off device
- `q` - Quit

**Note**: Press `x` to power off and on the device. The LED or pin 3 should go high to indicate the IC is in parallel programming mode.

### Read Options

- `1` - Read Signature Bits
- `2` - Read Fuse/Lock Bits
- `3` - Read Flash
- `4` - Read EEPROM
- `q` - Quit to Main Menu

#### Example: Reading Signature Bits

Reading Signature Bits...
Signature Byte 0: 1E
Signature Byte 1: 91
Signature Byte 2: 0A


### Read Fuse/Lock Bits

Press `2` to read Fuse/Lock bits:

Reading Fuse/Lock Bits...
Fuse Low Byte: 62
Fuse High Byte: DF
Extended Fuse Byte: FF
Lock Bits: FF

### Not Yet Implemented

- Read Flash
- Read EEPROM
- Programming Flash
- Programming EEPROM

## Acknowledgements

Special thanks to all contributors and sources that have made this project possible.


