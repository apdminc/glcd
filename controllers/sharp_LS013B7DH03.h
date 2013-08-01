#ifndef SHARP_LS013B7DH03_H_
#define SHARP_LS013B7DH03_H_



// LCD commands - Note: the bits are reversed per the memory LCD data
// sheets because of the order the bits are shifted out in the SPI
// port.
#define MLCD_WR_LSB               0x80                  //MLCD write line command
#define MLCD_WR_MSB               0x01                  //MLCD write line command

#define MLCD_CM_LSB               0x20                  //MLCD clear memory command
#define MLCD_CM_MSB               0x04                  //MLCD clear memory command

#define MLCD_NO_LSB               0x00                  //MLCD NOP command (used to switch VCOM)
//LCD resolution
#define MLCD_XRES                128            //pixels per horizontal line
#define MLCD_YRES                128           //pixels per vertical line
#define MLCD_BYTES_LINE          (MLCD_XRES / 8) //number of bytes in a line
#define MLCD_BUF_SIZE            (MLCD_YRES * MLCD_BYTES_LINE)
//defines the VCOM bit in the command word that goes to the LCD
#define VCOM_HI                  0x40
#define VCOM_LO                  0x00


#endif /* SHARP_LS013B7DH03_H_ */
