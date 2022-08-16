RC522-RFID-HAT from seengreat:www.seengreat.com
 ===================================
# Instructions
## 1.1、Overview
The RC522 RFID HAT is a module designed for Raspberry Pi application in RFID scenarios, which is used for contact-less communication at 13.56MHz. It also supports I2C, SPI and UART serial port communication, and the communication interface can be set by the on-board DIP switch.<br>
## 1.2、Specifications
Power Supply 3.3V / 5V<br>
Supports ISO/IEC14443A/MIFARE<br>
Supports MIFARE® Classic encryption in reader/writer model<br>
Supports interfaces of SPI/I2C/UART<br>
Running at 13.56MHz<br>
# Usage
# 2.1、Instructions of Hard Interface Configuration( For Three Types of Interface Conguration)
RC522 RFID HAT supports three communication modes of I2C, SPI and UART  at the same time. The communication interface can be switched through the two DIP switches, SW1 and SW2. The specific methods of configuration will be mentioned in the following diagrams 2.2.1, 2.2.2, 2.2.3.<br>
# 2.2、Demo Codes
  First, install the WiringPI library. <br>
  sudo apt-get install wiringpi<br>
  wget https://project-downloads.drogon.net/wiringpi-latest.deb  # Raspberry Pi 4B version upgrade <br>
  sudo dpkg -i wiringpi-latest.deb<br>
  gpio -v # If version 2.52 appears, it means that the installation has been successful<br>
### 2.2.1、uart Serial Port
Configure the module hardware DIP switch as the UART interface according to the requirements of the module UART interface configuration. This is shown below:<br>
 *  SW1:    RX   -> ON          SW2:    A1   -> -<br>
 *          TX   -> ON                  A0   -> -<br>
 *          SDA  -> OFF                 ADR5 -> 0<br>
 *          SCL  -> OFF                 ADR4 -> 0<br>
 *          NSS  -> OFF                 ADR3 -> 0<br>
 *          MOSI -> OFF                 ADR2 -> 0<br>
 *          MISO -> OFF                 ADR1 -> 0<br>
 *          SCK  -> OFF                 ADR0 -> 0<br>

_1、Serial port configuration of Raspberry Pi_
Because the serial port of "ttyS0" is used in the demo codes, it is necessary to confirm whether the "ttyS0" serial port has been configured before running the demo codes, and the confirmation process is as follows:<br>
The Raspberry Pi 3/4B has two serial ports, namely "hardware serial port" "/dev/ttyAMA0" and "mini serial port" "/dev/ttyS0". Due to the on-board Bluetooth module, this "hardware serial port" is assigned to connect with the Bluetooth module by default, and the "mini serial port" is assigned to the GPIO Tx Rx led by the pin by default. First, check the serial port mapping through the terminal command ls -l /dev, such as only "serial1 -> ttyAMA0", no "ttyS0" related mapping.<br>
Access the terminal of Raspberry Pi:<br>
sudo raspi-config<br>
Interface Options->Serial Port->No->Yes->OK<br>
Then reboot<br>
At this time, you can see the mapping relationship between serial0 -> ttyS0 and serial1 ->ttyAMA0, and "ttyS0" can be used normally.
In addition, we also need to disable the serial console, access the Raspberry Pi Configuration in the main menu, and select the Serial Console as Disable in the Interfaces options.<br>
Install python serial library：<br>
sudo apt-get install python3-serial<br>
_2、Execute the demo codes of serial port _
Access the terminal of Raspberry Pi and get the program directory:<br>
cd /home/pi/RC522_RFID_HAT/<br>
Python:<br>
Access python directory: cd Python<br>
Then execute: sudo python3 rc522-python-uart.py<br>
C:<br>
Access c directory: cd C/UART<br>
Execute:<br>
sudo make<br>
sudo ./main<br>
### 2.2.2、SPI Interface
Configure the module hardware DIP switch as the SPI interface according to the requirements of the module SPI interface configuration. This is shown below:<br>
 *  SW1:    RX   -> OFF         SW2:    A1   -> -<br>
 *          TX   -> OFF                 A0   -> +<br>
 *          SDA  -> OFF                 ADR5 -> 0<br>
 *          SCL  -> OFF                 ADR4 -> 0<br>
 *          NSS  -> ON                  ADR3 -> 0<br>
 *          MOSI -> ON                  ADR2 -> 0<br>
 *          MISO -> ON                  ADR1 -> 0<br>
 *          SCK  -> ON                  ADR0 -> 0<br>

_1、SPI Configuration of Raspberry Pi_
Start the system configuration of Raspberry Pi:<br>
sudo raspi-config<br>
Enable the SPI interface:<br>
Interfacing Options -> SPI -> Yes<br>
Check the enabled SPI devices:<br>
ls /dev/spi* # will print out:“/dev/spidev0.0”and“/dev/spidev0.1”<br>
Install the spidev library for Python3:<br>
sudo pip3 install spidev<br>
_2、Execute the demo codes of SPI interface_ 
Access the terminal of Raspberry Pi and get the program directory:<br>
cd /home/pi/RC522_RFID_HAT/<br>
Python:<br>
Access python directory: cd Python<br>
Then execute: sudo python3 rc522-python-spi.py<br>
C:<br>
Access c directory: cd C/SPI<br>
Execute:<br>
sudo make<br>
sudo ./main<br>

### 2.2.3、I2C Interface 
RC522 RFID HAT has been led the I2C address lines out. It can be freely selected by the users. The way to configure can be queried in RC522 official manual. Here we will also offer the screenshot of the mentioned configuration methods in the manual.<br>

The demo codes we provide is to set the EA to 1 via the DIP switch SW2, ADR_0-ADR_5 also all set to 1, and ADR6 is always 0, so the address is 0X3F (00111111).<br>
Configure the module hardware DIP switch as the I2C interface according to the requirements of the module I2C interface configuration. This is shown below:<br>
 *  SW1:    RX   -> OFF         SW2:    A1   -> +<br>
 *          TX   -> OFF                 A0   -> +<br>
 *          SDA  -> ON                  ADR5 -> +<br>
 *          SCL  -> ON                  ADR4 -> +<br>
 *          NSS  -> OFF                 ADR3 -> +<br>
 *          MOSI -> OFF                 ADR2 -> +<br>
 *          MISO -> OFF                 ADR1 -> +<br>
 *          SCK  -> OFF                 ADR0 -> +<br>

_1、I2C Configuration of Raspberry Pi_
Start the system configuration of Raspberry Pi:<br>
sudo raspi-config<br>
Enable the I2C interface:<br>
Interfacing Options -> I2C -> Yes<br>
sudo reboot<br>
Check the enabled I2C devices:<br>
ls /dev/i2c*   # will print out:“/dev/i2c-1”<br>
Install I2C library:<br>
sudo apt install i2c-tools <br>
Install smbus of python:<br>
sudo apt install python-smbus <br>
Test the address of the device mounted on the I2C bus:<br>
sudo i2cdetect -y -a 1<br>
_2、Execute the demo codes of the I2C interface:_
cd /home/pi/RC522_RFID_HAT/<br>
Python:<br>
Access python directory: cd Python<br>
Then execute: sudo python3 rc522-python-i2c.py<br>
C:<br>
Access c directory； cd C/IIC<br>
Execute:<br>
sudo make<br>
sudo ./main<br>
__Thank you for choosing the products of Shengui Technology Co.,Ltd. For more details about this product, please visit:
www.seengreat.com__
