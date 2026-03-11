# Scanning and decoding LCD matrix of dirt cheap CN IR thermometer.  
Device has proprietary analog sensor and MPU covered under epoxy. 
No I2C, no serial out, no nothing. Usual Arduino approach with MLX sensor is no option. 

### identifiy COMmon lines
4 lines, see images from startup phase

### identify SEGments  
apply synced pulses over SEQ and COM matrix with signal generator  

### results
LCD matrix f = 450Hz, T = 2,1ms  
4 COMmon lines, 11 SEGment lines  
3 Symbol segment lines are omitted to get along with 8 ADC inputs

### operation
AIN0 and AIN1 comparator is used for triggering on every display refresh cycle (COM1 flange falling). Threshold voltage is 2.6 V for operating device on 3 V supply.  
Comparator interrupt triggers scanning of all 8 digits on line common #1.  
Thereafter timer0 is started with around 2.1 ms interval.  
On these interrupts the respective 8 digits on common lines #2 to #4 are scanned.  
After three timer overflows the scannning is complete and program waits for next run.
Due to such LCDs operating with multiple voltage levels, analog inputs A0 ... A7 are required. A level of > 1.1 V is considered an active segment digit.

### sample screenshots
some power up and startup sequences  

line A0 showing 1101.png:  

|channel|signal|
|-----|-----|
|CH1|COM1|
|CH2|timer1 overflow interrupt|
|CH3|comparator interrupt|
|CH4|SEG line A0 (1101) when "22.0" displayed|
|MATH| resulting signal common line 1 - segment line 0| 

line A6 showing 0000.png:
same as above but line A6 (0000), display showing "21.4"
    
### segment numbering scheme:
```
    5         
  -----      -----      -----        
6|     |4   |     |    |     |
 |  7  |    |     |    |     |
  -----      -----      -----       __ 
1|     |3   |     |    |     |     |__|
 |     |    |     |    |     |  0  |__|
  -----      -----      -----
    2
    A          B          C       D
```
### common/segment matrix:

|SEG #|COM1|COM2|COM3|COM4|bit #|
|---|---|---|---|---|---|
|1|na|BAT|BAT|BAT|na|
|2|BAT|BAT|°F|°C|na|
|3|D5|D4|D7|D3|0|
|4|na|D6|D1|D2|1|
|5|backlight|na|na|dot|na|
|6|C5|C4|C7|C3|2|
|7|laser|C6|C1|C2|3|
|8|B5|B4|B7|B3|4|
|9|SCAN|B6|B1|B2|5|
|10|A5|A4|A7|A3|6|
|11|HOLD|A6|A1|A2|7|

### bitmasks for each figure (read inverted)
```
 _
| |
|_|    01111110    0x7E

  |
  |    00011000    0x18    

 _
 _|
|_     10110110    0xB6    

_
_|
_|     10111100    0xBC

|_|
  |    11011000    0xD8

 _
|_
 _|    11101100    0xEC

 _
|_
|_|    11101110    0xEE

_
 |
 |     00111000    0x38

 _
|_|
|_|    11111110    0xFE

 _
|_|
 _|    11111100    0xFC


 _     10000000    0x80
```
