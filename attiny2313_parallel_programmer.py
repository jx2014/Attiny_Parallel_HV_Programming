import time
import serial
import argparse
import sys

PAGE_SIZE = 64
RECORD_TYPE_DATA = 0
RECORD_TYPE_EOF = 1
RECORD_TYPE_EXT_SEGMENT_ADDR = 2
RECORD_TYPE_START_SEGMENT_ADDR = 3
RECORD_TYPE_EXT_LINEAR_ADDR = 4
RECORD_TYPE_START_LINEAR_ADDR = 5




class Programmer:
    def __init__(self, port, baud):
        self.ser = serial.Serial(port=port, baudrate=int(baud), timeout=1)
        self.power_cycle()
        self.read_signature()

    def write(self, cmd):
        self.ser.write(cmd.encode())

    def write_with_new_line(self, cmd):
        cmd += '\r\n'
        self.ser.write(cmd.encode())

    def read(self, delay=0.01):
        time.sleep(delay)
        item_to_read = self.ser.in_waiting
        result = ''
        while item_to_read:
            time.sleep(delay)
            result += self.ser.read(item_to_read).decode()
            item_to_read = self.ser.in_waiting
        return result

    def chip_erase(self):
        self.main_menu()
        self.read()
        self.write('c')
        print(self.read())
        self.main_menu()
        self.write('r')
        self.write('1')


    def power_cycle(self):
        response = ''
        self.write('x')
        self.write('x')
        self.write('x')
        while 'Powering on' not in response.strip().split('\r\n')[-1]:
            self.write('x')
            response = self.read()

    def main_menu(self):
        response = self.read()
        while 'Main Menu' not in response.strip().split('\r\n')[0]:
            self.write('q')
            response = self.read()

    def read_signature(self):
        self.main_menu()
        self.write('r')
        self.read()
        self.write('1')
        print(self.read())

    def read_fuse(self):
        self.main_menu()
        self.write('r')
        self.read()
        self.write('2')
        print(self.read())

    def read_flash(self, start=0, end=1024, delay=0.01):
        print("Reading flash!")
        self.main_menu()
        self.write('r')
        self.read()
        self.write('5')
        result = self.read()
        all_data = result.split("Done!")[0].strip().split(' ')
        print("Read flash complete!")
        return [y for x in all_data for y in [x[:2], x[2:]]]

    def to_hex(self, data_bytes):
        """
        convert bytes to intel hex format
        :param data_bytes:
        :return:
        """
        hex_file = []
        eof_i = len(data_bytes) + 1
        eof = False
        for byte in data_bytes[::-1]:
            eof_i -= 1
            if byte != 'FF':
                # found end of file
                eof = True
                break

        # remove unwanted 'FF'
        if eof:
            data_bytes = data_bytes[:eof_i]
        else:
            data_bytes = data_bytes[:eof_i - 1]

        page_start = True
        byte_count = None
        page_data = None
        check_sum = None
        page_address = None
        for i, byte in enumerate(data_bytes):
            if page_start:
                byte_count = 0
                page_data = ''
                page_address = i
                check_sum = (page_address >> 8 & 0xff)+ (page_address & 0xff)
                page_start = False
            byte_count += 1
            check_sum += int(byte, 16)
            page_data += byte
            if byte_count == 32 or i == len(data_bytes) - 1:
                check_sum += byte_count
                check_sum_byte = format(((~check_sum & 0xff) + 1), '02X')[-2::]
                hex_file.append(f':{byte_count:02X}{page_address:04X}{RECORD_TYPE_DATA:02X}{page_data}{check_sum_byte}')
                page_start = True
        hex_file.append(":00000001FF")
        return hex_file

    def write_flash(self, hex_file):
        self.chip_erase()
        print(self.read())
        self.main_menu()
        self.write('w')
        self.read()
        self.write('3')
        address = 0
        while address < PAGE_SIZE * 16:
            line = hex_file.readline()
            if line == '':
                break
            byte_count, start_address, record_type, data, _, _ = self.decipher(line)
            if int(record_type, 16) == RECORD_TYPE_EOF:
                break
            if (byte_count / 2) != len(data):
                break
            for i, word in enumerate(data):
                addr_lowbyte = format((address + i) & 0xff, '02X')
                if word == 'FFFF':
                    continue
                self.write(addr_lowbyte)
                self.write('\n')
                print(self.read())
                self.write(word)
                self.write('\n')
                print(self.read())
            # write high byte
            address += (i + 1)
            self.write('w\n')
            addr_highbyte = format((address >> 8) & 0xff, '02X')
            self.write(addr_highbyte)
            self.write('\n')
        print(self.read())
        self.write('q\n')
        print(self.read())
        self.main_menu()
        self.power_cycle()
        print("Flash written.")


    def decipher(self, i):
        i = i.strip(':').strip('\n')
        byte_count_hex = i[:2]
        byte_count_int = int(byte_count_hex, 16)
        check_sum_cal = byte_count_int
        address_hex = i[2:6]
        check_sum_cal += int(address_hex[:2], 16)
        check_sum_cal += int(address_hex[2:], 16)
        record_type = i[6:8]
        check_sum_cal += int(record_type, 16)
        data_bytes = []
        n = len(i) - 10
        x = 0
        while (x < n):
            i[8 + x: 8 + x + 4]
            data_bytes.append(i[8 + x: 8 + x + 4])
            check_sum_cal += int(i[8 + x: 8 + x + 2], 16) + int(i[8 + x + 2: 8 + x + 4], 16)
            x += 4
        check_sum_cal = format(((~check_sum_cal & 0xff) + 1), 'X')[-2::]
        check_sum = i[-2:]
        return byte_count_int, address_hex, record_type, data_bytes, check_sum, check_sum_cal


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-P', type=str, help='Arduino COM port', required=True)
    parser.add_argument('-b', type=str, help='Arduino COM port baud rate', required=True)
    parser.add_argument('--read-flash', type=str, help='read flash', required=False)
    parser.add_argument('--read-eeprom', action='store_true', help='read eeprom', required=False)
    parser.add_argument('--read-fuse-lockbits', action='store_true', help='read fuse and lock bits', required=False)
    parser.add_argument('--write-flash', type=str, help='write flash', required=False)
    parser.add_argument('--write-eeprom', type=str, help='write eeprom', required=False)
    parser.add_argument('--write-fuse-high-byte', type=str, help='write high fuse byte', required=False)
    parser.add_argument('--write-fuse-low-byte', type=str, help='write low fuse byte', required=False)
    parser.add_argument('--write-fuse-extend-byte', type=str, help='write extended fuse byte', required=False)
    parser.add_argument('--write-lock-bits', type=str, help='write lock bits', required=False)
    parser.add_argument('--chip-erase', action='store_true', help='chip erase', required=False)

    args = parser.parse_args()
    programmer = Programmer(args.P, args.b)
    programmer.read_fuse()

    if args.chip_erase:
        programmer.chip_erase()

    if args.read_flash:
        output_file = args.read_flash
        a = programmer.read_flash()
        hex_file = programmer.to_hex(a)
        with open(output_file, 'w') as file:
            file.write('\n'.join(hex_file))

    if args.write_flash:
        input_file = args.write_flash
        with open(input_file, 'r') as file:
            programmer.write_flash(file)


