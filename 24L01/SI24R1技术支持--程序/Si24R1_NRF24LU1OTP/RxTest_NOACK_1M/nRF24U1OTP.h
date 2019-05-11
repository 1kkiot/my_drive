//======================================================================
//========  		Define interface to nRF24LU1OTP               ========   
//======================================================================
  
sfr RFCON = 0x90 ; 
sfr RFSBUF = 0xE5 ; 
sfr RFCTL = 0xE6 ;
sfr IRCON = 0xC0 ;
sbit RFRDY = IRCON^0 ;
sbit RFIRQ =IRCON^1;

//                        SPI commands (nRF24LU1OTP+)
//======================================================================
#define R_REGISTER    		0x00  	// Define Read command to register
#define W_REGISTER   		0x20  	// Define Write command to register
#define R_RX_PAYLOAD 		0x61  	// Define Read RX payload register address
#define W_TX_PAYLOAD 		0xA0  	// Define Write TX payload register address
#define FLUSH_TX    		0xE1  	// Define Flush TX register command
#define FLUSH_RX    		0xE2  	// Define Flush RX register command
#define REUSE_TX_PL 		0xE3  	// Define Reuse TX payload register command
#define R_RX_PL_WID     	0x60  	// Define Read RX payload width for the R_RX_PAYLOAD in the RX FIFO
#define W_ACK_PAYLOAD   	0xA8	// Define Write Payload to be transmitted together with ACK packet on PIPExxx xxx valid in the range from 000 to 101)
#define W_TX_PAYLOAD_NOACK	0xB0	// Define Write TX Disables AUTOACK on this specific packet
#define NOP         		0xFF  	// Define No Operation, might be used to read status register

//======================================================================
//                    SPI registers address(nRF24LU1OTP+)
//======================================================================
#define CONFIG      0x00  // 'Config' register address
#define EN_AA       0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  // 'Enabled RX addresses' register address
#define SETUP_AW    0x03  // 'Setup address width' register address
#define SETUP_RETR  0x04  // 'Setup Auto. Retrans' register address
#define RF_CH       0x05  // 'RF channel' register address
#define RF_SETUP    0x06  // 'RF setup' register address
#define STATUS      0x07  // 'Status' register address
#define OBSERVE_TX  0x08  // 'Observe TX' register address
#define RPD         0x09  // 'Received Power Detector' register address  equal to nRF24LU1OTP is "CD"(Carrier Detecotr)
#define RX_ADDR_P0  0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F  // 'RX address pipe5' register address
#define TX_ADDR     0x10  // 'TX address' register address
#define RX_PW_P0    0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17  // 'FIFO Status Register' register address
#define DYNPD       0x1C  // 'Enable dynamic paload length' register address
#define FEATURE     0x1D  //  Feature Register address


////////////////////// MCU 
sfr P0DIR = 0x94 ;
sfr P0EXP = 0xC9 ;
sfr P0ALT = 0x95 ; 
sfr P1DIR = 0x96 ;
sfr P1ALT = 0x97 ;

sfr WDCON = 0xD8 ;
sfr S0RELL = 0xAA ; 
sfr S0RELH = 0xBA ; 

                                     
sfr ACC  = 0xE0; //Accumulator                                                              0000,0000
sfr B    = 0xF0; //B Register                                                               0000,0000
sfr PSW  = 0xD0; //Program Status Word      CY    AC    F0    RS1   RS0    OV    F1    P    0000,0000
//-----------------------------------
sbit CY  = PSW^7;
sbit AC  = PSW^6;
sbit F0  = PSW^5;
sbit RS1 = PSW^4;
sbit RS0 = PSW^3;
sbit OV  = PSW^2;
sbit P   = PSW^0;
//-----------------------------------
sfr SP   = 0x81; //Stack Pointer                                                            0000,0111
sfr DPL0  = 0x82; //Data Pointer Low Byte                                                    0000,0000
sfr DPH0  = 0x83; //Data Pointer High Byte  
sfr DPS = 0x92 ;  
sfr P0   = 0x80; //8 bitPort0  
sfr PCON = 0x87; //Power Control
sfr IE  = 0xA8;  //
sbit ES = IE^4; // ENABLE UARTINT
sbit EA = IE^7;
sfr SCON = 0x98; //Serial Control 
sbit SM0 = SCON^7;  //SM0/FE
sbit SM1 = SCON^6;
sbit SM2 = SCON^5;
sbit REN = SCON^4;
sbit TB8 = SCON^3;
sbit RB8 = SCON^2;
sbit TI  = SCON^1;
sbit RI  = SCON^0;
//-----------------------------------
sfr SBUF = 0x99; //Serial Data Buffer   
//-----------------------------------
sfr CLKCTL = 0xa3 ;// clk config     
