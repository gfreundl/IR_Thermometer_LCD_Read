
// reads LCD matrix line for China IR Thermometer
// LCD has 4 COMmon and 11 SEGment lines, so we collect data in 4 passes
// frequency 453Hz, voltage levels 0V, 1V, 2V, 3V 
// only digits are read, additional symbols ignored

// functional description:
// comparator on COM 1 line detects first COM pulse and starts scanning run
// only relevant SEG lines are connected to A/D converter, decimal point and symbols are omitted
// matrix pulses are converted to digits binary representation
// digits converted to numbers
// changes to original sketch:
//    timing 489 Hz to 455 Hz
//    threshold <0,75 V to >1.1 V 
//    test pins D3 and D4 for debugging IRQ pulses on scope

// defines for setting and clearing register bits
// sbi() and cbi() originally are Arduibno inline assembler
#ifndef cbi                                   //set bit
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))   //_BV is just an alternative for (1<<bit)
#endif
#ifndef sbi                                   //clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int timer1_counter;
volatile byte _comState;                      //counter for current COM line (1...4)
volatile byte _displayScan[4];                //contains scanned pulse results for each pass
volatile byte _scanComplete = 0;
volatile byte _triggeredCounter = 0;
int LCD_threshold = 230;                      // (LCD_threshold/1023)*5 = 1,1 V
byte digitA, digitB, digitC, digitD;          //intermediate result bytes

void setup() {
  noInterrupts();  
  Serial.begin(9600);
  // set ADC prescale to 16 to speed up the read speed.
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  // Comparator Pins
  pinMode(7,INPUT);
  pinMode(6, INPUT);
  Serial.println("IR Thermometer");
  Serial.println("140326");
  
  //emulate required scan button sequence for continous operation
  //CAUTION: make sure to NEVER set this pin HIGH, 5V could fry the device !!!
  //start with open drain
  pinMode(2, INPUT);
  digitalWrite(2, LOW);
  //waiting 5 sec for thermometer to come up
  delay(5000);
  //switching thermometer scan on
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(1000);
  //switching scan off
  pinMode(2, INPUT);
  digitalWrite(2, LOW);
  delay(1000);
  //switching thermometer scan finallly on 
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(500);

  //test pins for interrupts, decrease scan interval to detect these on scope!
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

  // comparator interrupt enabled and tripped on falling edge, 
  // which is the rising edge of the input signal.
  ACSR = B00011010; 

  // Timer setup
  TCCR1A = 0;
  TCCR1B = 0;
  //timer1_counter = 61453;   // original 489Hz @16MHz => 200us ahead of expected next start
  timer1_counter = 61140;     // so changed to 455Hz
  TCCR1B |= (1 << CS11);    // 8 prescaler 
  
  interrupts(); 
}

// convert observed bit sequence to number
int convertDigit(byte digit)
{
if(digit == 0x80) return -1;        //minus sign
  if(digit == 0x7E) return 0;
  if(digit == 0x18) return 1;
  if(digit == 0xB6) return 2;
  if(digit == 0xBC) return 3;
  if(digit == 0xD8) return 4;
  if(digit == 0xEC) return 5;
  if(digit == 0xEE) return 6;
  if(digit == 0x38) return 7;
  if(digit == 0xFE) return 8;
  if(digit == 0xFC) return 9;
  if(digit == 0x00) return -1000;
  return -2000;
}

int generateOutput()
{
  int converted;

  converted = convertDigit(digitD);
  if(converted < 0) return -2001; // Error
  int output = converted;
  
  converted = convertDigit(digitC);
  if(converted < 0) return -2002; // Maybe an error???
  output += converted * 10;
  
  converted = convertDigit(digitB);
  if(converted == -1){                    //minus sign
    return output * -1;
  }else if(converted == -1000){
    return output; 
  } else if(converted == -2000){
    return -2003;
  } else {
    output += converted * 100;
  }

  converted = convertDigit(digitA);
  if(converted == -1){                    //minus sign
    return output * -1;
  }else if(converted == -1000){
    return output; 
  } else if(converted == -2000){
    return -2004;
  } else {
    output += converted * 1000;
  }

  return output;
}

// see COM/SEG matrix with A0...A7 lines for reference
void sortDigits()
{
  digitA = digitB = digitC = digitD = 0;
  // in COM1 only 4 relevant SEGs
  if(_displayScan[0] & (1<<0)) digitD |= (1<<5);       
  if(_displayScan[0] & (1<<2)) digitC |= (1<<5);
  if(_displayScan[0] & (1<<4)) digitB |= (1<<5);
  if(_displayScan[0] & (1<<6)) digitA |= (1<<5);

  if(_displayScan[1] & (1<<0)) digitD |= (1<<4);
  if(_displayScan[1] & (1<<1)) digitD |= (1<<6);
  if(_displayScan[1] & (1<<2)) digitC |= (1<<4);
  if(_displayScan[1] & (1<<3)) digitC |= (1<<6);
  if(_displayScan[1] & (1<<4)) digitB |= (1<<4);
  if(_displayScan[1] & (1<<5)) digitB |= (1<<6);
  if(_displayScan[1] & (1<<6)) digitA |= (1<<4);
  if(_displayScan[1] & (1<<7)) digitA |= (1<<6);

  if(_displayScan[2] & (1<<0)) digitD |= (1<<7);
  if(_displayScan[2] & (1<<1)) digitD |= (1<<1);
  if(_displayScan[2] & (1<<2)) digitC |= (1<<7);
  if(_displayScan[2] & (1<<3)) digitC |= (1<<1);
  if(_displayScan[2] & (1<<4)) digitB |= (1<<7);
  if(_displayScan[2] & (1<<5)) digitB |= (1<<1);
  if(_displayScan[2] & (1<<6)) digitA |= (1<<7);
  if(_displayScan[2] & (1<<7)) digitA |= (1<<1);

  if(_displayScan[3] & (1<<0)) digitD |= (1<<3);
  if(_displayScan[3] & (1<<1)) digitD |= (1<<2);
  if(_displayScan[3] & (1<<2)) digitC |= (1<<3);
  if(_displayScan[3] & (1<<3)) digitC |= (1<<2);
  if(_displayScan[3] & (1<<4)) digitB |= (1<<3);
  if(_displayScan[3] & (1<<5)) digitB |= (1<<2);
  if(_displayScan[3] & (1<<6)) digitA |= (1<<3);
  if(_displayScan[3] & (1<<7)) digitA |= (1<<2);
}

//scan matrix "vertically"
//voltage levels above LCD_threshold are considered significant
void scanInputs()
{
    byte segments = 0;
    if(analogRead(A0) > LCD_threshold) segments |= (1 << 0);      
    if(analogRead(A1) > LCD_threshold) segments |= (1 << 1);
    if(analogRead(A2) > LCD_threshold) segments |= (1 << 2);
    if(analogRead(A3) > LCD_threshold) segments |= (1 << 3);
    if(analogRead(A4) > LCD_threshold) segments |= (1 << 4);
    if(analogRead(A5) > LCD_threshold) segments |= (1 << 5);
    if(analogRead(A6) > LCD_threshold) segments |= (1 << 6);
    if(analogRead(A7) > LCD_threshold) segments |= (1 << 7);

    _displayScan[_comState] = segments;
    _comState++;
}

ISR(TIMER1_OVF_vect)
{
  TCNT1 = timer1_counter;   // preload timer
  digitalWrite(4,HIGH);
  delayMicroseconds(100);          //wait for rise time of flange
  digitalWrite(4,LOW);  
  scanInputs();
  
  if(_comState > 3)
  {
    TIMSK1 &= ~(1 << TOIE1);   // disable timer overflow interrupt
    _scanComplete = 1;
  }
}


ISR(ANALOG_COMP_vect)
{
  // Discard the first 5 triggers, as on power up the display has some odd behaviour.
  if(_triggeredCounter++ < 3) return;
    _triggeredCounter = 0;
    digitalWrite(3,HIGH);    

    // Setup timer for the next set
    TCNT1 = timer1_counter;
    TIFR1 |= (1 << TOV1); // Clear any pending interrupt
    TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
    _comState = 0;

    delayMicroseconds(100);
    digitalWrite(3,LOW);    
    
    scanInputs();

    // Disable the analog comparator until this result has been processed
    ACSR &= ~(1<<ACIE);               
    // previous line unclear as interrupt automatically disabled at start and re-enabled at end of ISR
}

void initiateRead()
{
  delay(500);
  delay(500);
  
  //Clear any pending comparator interrupt
  ACSR |= (1<<ACI);
  // Enable the interrupt
  ACSR |= (1<<ACIE);
}

void loop() {
  
  if(1)//Serial.available() ) //&& Serial.read() == 's')
  { 
    initiateRead();
    while(1){
      if(_scanComplete){
        _scanComplete = 0;
    
        sortDigits();
        
        int output = generateOutput();
        //this original clause stops any automatic operation after one error -200.-1 has been displayed
        //if (output != 8888) // 8888 is the initial state when then the device boots
        if(1)
        {
          /*Serial.print(digitA, HEX);
            Serial.print(" ");
            Serial.print(digitB, HEX);
            Serial.print(" ");
            Serial.print(digitC), HEX;
            Serial.print(" ");
            Serial.println(digitD, HEX);
            Serial.println(" ");

            Serial.print(_displayScan[0], BIN);
            Serial.print(" ");
            Serial.print(_displayScan[1], BIN);
            Serial.print(" ");
            Serial.print(_displayScan[2], BIN);
            Serial.print(" ");
            Serial.println(_displayScan[3], BIN);          
            */ 
          Serial.print(output/10);
          Serial.print('.');
          Serial.println(output%10);
          break;
        }          
      } 
    }
  }
  digitalWrite(3,LOW);
}



