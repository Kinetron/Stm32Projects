# Display board for the charger STM32F103C6T6
The board displays the current, voltage and controls the fan rotation. When the battery voltage reaches 14.7 volts, the indicator starts flashing.
The voltage and current on the indicator switch automatically every 5 seconds.

It was necessary to charge the LiFePol battery 100Ah. I bought a power supply for a LED strip(12v 29A) and redid it.
![LDE DC](./photo/ledDC.jpg)
![battery](./photo/battery.jpg)

I added a 10A current limiter based on the LM358.
The first part of the chip amplifies the voltage for TL494,
the second part - for STM32 board.
![circuit diagram](./photo/LM358circuit.jpg)
![circuit diagram](./photo/lm358Board.jpg)

Bottom board
![circuit diagram](./photo/buttonBordLedDC.jpg)

The next step is to make an indication board and write the firmware for it.
I used sprint layout 6.

![circuit diagram](./photo/rawBoard1.jpg)
![circuit diagram](./photo/rawBoard2.jpg)

![board full](./photo/boardFull.jpg)
Load firmware use flash loader demo and CH340G .
![load firmware](./photo/flashing.jpg)

Board with firmware
![top board](./photo/boardTop.jpg)

Result - charger
![hand made charger](./photo/fullDevice.jpg)

Last binary file you found in
./binary

Board design in ./board 

Battery charge current 10А.