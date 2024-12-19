# Raspberry Pi Pico + ESP8285 (19 December 2024)

This document details the process of understanding and working with this unique chip.

## Table of Contents
- [Introduction](#introduction)
- [Hardware](#hardware)
- [Software](#software)
- [Setup](#software)
- [Contributing](#contributing)
- [License](#license)


## Introduction
I purchased two of these chips from eBay, initially believing they were clones of the Raspberry Pi Pico W. At that time, I was not familiar with the specific wireless chip included. Thus, they appeared quite standard.

![image](https://github.com/user-attachments/assets/44f75e18-558e-405f-bfda-1350edb80838)

Although the board features an RP2040, keen observers will notice it uses an ESP8285 as its network module instead of the CYW43439. This makes it more of a Raspberry Pi Pico clone with an added ESP8285 rather than a direct Pico W clone. This distinction is crucial because, despite the similar functionality on paper, you cannot use the WiFi commands from Pico W example programs with this board.

However, there is still hope. Network-enabled chips typically offer mechanisms for communication and control by other chips.

After some research, I discovered a link (https://mega.nz/folder/1TQ1mCgB#08XOqEZgDEoZkZh1CIOP-A) from a Chinese vendor selling similar boards. This link contains sample code, which I have uploaded to the specified folder.

I have chosen to use [MicroPython](https://micropython.org/) to program this microcontroller. Although other options like [CircuitPython](https://circuitpython.org/) and the Arduino IDE are available, I opted for MicroPython because it is the preferred method by the board's manufacturers. This choice should help avoid potential issues. Additionally, it is convenient for me as I am currently teaching an Introduction to Programming class (Semester 1, 2024) and have recently refreshed my knowledge of the syntax. I am using Visual Studio Code as my text editor with the Pico extension installed.

## Hardware
- Microcontroller: [RP2040](https://www.raspberrypi.org/products/rp2040/)
- Network Module: [ESP8285](https://www.espressif.com/en/products/socs/esp8285)
- Other components: LEDs, sensors, etc.

## Software
- Programming Language: [MicroPython](https://micropython.org/)
- IDE: [Visual Studio Code](https://code.visualstudio.com/)

## Setup
1. Clone the repository:
    ```sh
    git clone https://github.com/a1inani/MicrocontrollerShenanigans.git
    ```
2. Install the necessary software and libraries.
3. Flash the firmware to the microcontroller.
4. Connect the hardware components.

## Contributing
Here is how you can contribute to the project.

1. Fork the repository.
2. Create a new branch:
`git checkout -b feature-branch`
3. Make your changes and commit them:
`git commit -m "Add new feature"`
4. Push to the branch:
`git push origin feature-branch`
5. Create a pull request.

## License
Specify the license under which the project is distributed.

This project is licensed under the MIT License - see the LICENSE file for details.