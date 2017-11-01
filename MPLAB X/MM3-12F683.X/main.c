// 12F683 Multi-Mode 3 Open Source Modchip
// 2017 Gadorach
// Based on MM3, OldCrow, garyOPA
// Built with MPLAB X through the XC8 Compiler
//
// Build: 10/31/2017
// 
//                      ________  ________
//                      |       \/       |
//           VDD (5V) --+ 1 >>      >> 8 +-- VSS (Ground)
//                      |                |
//           /        --+ 2         << 7 +-- signal from door (GPIO0)
//                      |                |
//           /        --+ 3         >> 6 +-- data stream (GPIO1)
//                      |                |
//         MCLR Reset --+ 4 >>      >> 5 +-- gate output (GPIO2)
//                      |                |
//                      +----------------+
//
// Only connect pins 1, 4, 5, 6, 7, 8
// Pinout compatible with MM3 installation diagrams
// OneChip PAL BIOS patches not implemented at this time
// NTSC BIOS patches not implemented at this time
//
// For an Arduino (Atmel) version, refer to PsNee or PsNee V6 (OneChip)
// PsNee:    https://pastebin.com/82h52q37
// PsNee V6: https://pastebin.com/cWCsYugc
//
// Note, the version of the Flash library linked in those is outdated and will not compile in recent Arduino builds.
// For both, you will need this library: https://github.com/schinken/Flash/releases/tag/v1.0.1

#pragma config FOSC = INTOSCCLK // Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR, MCLR externally set)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)


#define _XTAL_FREQ 8000000

//INCLUDES
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pic12f683.h>

// Region string character variables
unsigned char SCExS = 0b01010011;
unsigned char SCExC = 0b01000011;
unsigned char SCExE = 0b01000101;
unsigned char SCExA = 0b01000001;
unsigned char SCExI = 0b01001001;
unsigned char SCExW = 0b01010111;

#define REGIONSEL 0 // 0 = ALL, 1 = NA, 2 = EUR, 3 = JPN, 4 = Yaroze
                    // All, or Multiregion, may cause boot time delays for some consoles. This can be fixed by using the correct region for your console exclusively.

//TRIS Pins for handling the high-impedance input state for high outputs and ignoring regular operation
#define MICRO_PIN7 TRISIObits.TRISIO0 //detect CD door pin
#define MICRO_PIN6 TRISIObits.TRISIO1 //data pin
#define MICRO_PIN5 TRISIObits.TRISIO2 //transistor gate pin

volatile unsigned char sendSecurityEnable = 1;

void sendSecurityString(unsigned char secChar){ //send the selected security string
    unsigned char bitPosition = 0;
    unsigned char sendMask = 0b00000001; //send bytes one bit at a time on pin 6, LSB first
    unsigned char sendFlag;
        
        MICRO_PIN6 = 1; //data start bit
        __delay_ms(4);
        
        while(bitPosition < 8){
            sendFlag = (secChar & sendMask); //bitwise AND to setup the current bit in the character to transmit
            if(!sendFlag){                   //Invert the data to conform to the PS1 laser's data stream
                MICRO_PIN6 = 1; //set to input mode to trigger a HIGH
            } else {
                MICRO_PIN6 = 0; //set to output mode to trigger a LOW
            }
            bitPosition++;
            sendMask = sendMask << 1; //shift our mask over one bit for the next position
            __delay_ms(4);            //wait 4ms between bits for the CD controller to read them
        }
        
        MICRO_PIN6 = 0;  //data end bit
        __delay_ms(8);   //two end bits (4ms*2)
        
        bitPosition = 0; //reset bit position to 0 for next character
}

static void interrupt DOOR_DETECT(void){
    GPIO;                           //clear any mismatch conditions
    if( GPIObits.GP0 == 0){         //If GPIO0 (PIN7) detected as triggering the interrupt
        INTCONbits.GIE = 0;         //Disable interrupts to bypass switch bounce
        __delay_ms(50);             //wait for the door bounce to finish
        if(GPIObits.GP0 == 0){      //if door still closed
            sendSecurityEnable = 1; //allow security bits to be sent again
        }
        INTCONbits.GIE = 1;         //Re-enable interrupts when done
    }
    
    INTCONbits.GPIF = 0;            //clear the interrupt flags
}

void main(void) {
  OSCCON = 0x70; // switch to 8MHz system clock
  GPIO   = 0xFF; // start with all GPIO low
  ANSEL  = 0x00; // disable analog inputs
  GPIObits.GP1 = 0; //set GPIOs LOW for mode switching usage (output mode becomes low, input becomes high thanks to PS1 on-board pullup)
  GPIObits.GP2 = 0;
  TRISIO = 0x00; // set data flow mode to output
  ADCON0 = 0x00; // disable analog-to-digital conversion block
  CMCON0 = 0x07; // disable comparators
  INTCONbits.GIE = 1;  //enable interrupts
  INTCONbits.GPIE = 1;
  IOC = 0x00;          //clear IOC bits (just incase)
  IOCbits.IOC0 = 1;    //enable pin 7 IOC
  
  MICRO_PIN7 = 1; //set pin 7 as an input for reading the CD door status (stealth)
  MICRO_PIN6 = 1; //set pin 6 as an input to start in high impedance state
  MICRO_PIN5 = 1; //set pin 5 as an input to start in high impedance state
  
  while(1){
    if (sendSecurityEnable == 1){  
      __delay_ms(50); //after wakeup, wait 50ms and set Pin 6 LOW
      MICRO_PIN6 = 0;
      __delay_ms(850); //next, wait 850ms and set Pin 5 LOW as well
      MICRO_PIN5 = 0;
      __delay_ms(314); //wait 314ms before next instruction
      
      for (unsigned char i = 30; i > 0; i--){ // Set to 2 for debugging
        for(unsigned char j = 4; j > 0; j--){
            sendSecurityString(SCExS); //inject the security string for your console region to the CD controller
            sendSecurityString(SCExC);
            sendSecurityString(SCExE);
          #if (REGIONSEL == 0) //ALL
            switch(j){
                case 4:
                {
                    sendSecurityString(SCExA);
                    break;
                }
                case 3:
                {
                    sendSecurityString(SCExE);
                    break;
                }
                case 2:
                {
                    sendSecurityString(SCExI);
                    break;
                }
                case 1:
                {
                    sendSecurityString(SCExW);
                    break;
                }
            }
          #elif (REGIONSEL == 1) //NA
            sendSecurityString(SCExA);
          #elif (REGIONSEL == 2) //EUR
            sendSecurityString(SCExE);
          #elif (REGIONSEL == 3) //JPN
            sendSecurityString(SCExI);
          #elif (REGIONSEL == 4) //Yaroze
            sendSecurityString(SCExW);
          #endif
            __delay_ms(72); //72ms delay footer for the CD drive to read each string
        }
          
        
      }

      MICRO_PIN6 = 1; //re-enable the regular communication between the laser and the CD controller
      MICRO_PIN5 = 1;
      
      sendSecurityEnable = 0;
    }
    SLEEP(); //disable microcontroller for stealth until CD door open/close detected again
  }
  return; //end program (never reached)
}