# Scanning and decoding LCD matrix of dirt cheap CN IR thermometer.  
Device has proprietary analog sensor and MPU covered under epoxy. 
No I2C, no serial out, no nothing. Usual Arduino approach with MLX sensor is no option. 

### identifiy COMmon lines
4 lines, see images from startup phase

### identify SEGments  
apply synced pulses over SEQ and COM matrix with signal generator  

### results
LCD matrix f = 453Hz, T = 2,2ms  
4 COMmon lines, 11 SEGment lines  
Symbol segments are omittedt to get along with 8 ADC inputs  
Comparator used for triggering on every cycle (COM1 flange falling)  

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
