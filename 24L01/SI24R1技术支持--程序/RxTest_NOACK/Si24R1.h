//======================================================================
//========  		Define interface to Si24R1               ========   
//======================================================================

//======================================================================
//                      Define SPI pins(Si24R1)
//======================================================================
sbit CE   = P1^0;  // Chip Enable pin signal (output to Si24R1)
sbit CSN  = P1^1;  // Slave Select pin, (output to  Si24R1)
sbit SCK  = P1^2;  // Master Out, Slave In pin (output to Si24R1)
sbit MOSI = P1^3;  // Serial Clock pin, (output to Si24R1)
sbit MISO = P1^4;  // Master In, Slave Out pin (input from Si24R1)
sbit IRQ  = P1^5;  // Interrupt signal,(input  from Si24R1)

//======================================================================
//                        SPI commands (Si24R1)
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
//                    SPI registers address(Si24R1)
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
#define RPD         0x09  // 'Received Power Detector' register address  equal to Si24R1 is "CD"(Carrier Detecotr)
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


