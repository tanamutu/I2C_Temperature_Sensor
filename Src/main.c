#include <stdbool.h>

//REGISTER DEFINITION
#define GPIOA_MODER     (*(volatile unsigned int *) 0x50000000)
#define GPIOA_OTYPER    (*(volatile unsigned int *) 0x50000004)
#define GPIOA_AFRL      (*(volatile unsigned int *) 0x50000020)
#define GPIOA_AFRH      (*(volatile unsigned int *) 0x50000024)

#define RCC_IOPENR  (*(volatile unsigned int *)0x40021034)
#define RCC_APBENR1 (*(volatile unsigned int *)0x4002103C)
#define RCC_CCIPR   (*(volatile unsigned int *)0x40021054)

#define I2C_TIMINGR (*(volatile unsigned int *)0x40005410)
#define I2C1_CR     (*(volatile unsigned int *)0x40005400)
#define I2C1_CR2    (*(volatile unsigned int *)0x40005404)
#define I2C1_ISR    (*(volatile unsigned int *)0x40005418)
#define I2C1_ICR    (*(volatile unsigned int *)0x4000541C)
#define I2C1_TXDR   (*(volatile unsigned int *)0x40005428)
#define I2C1_RXDR   (*(volatile unsigned int *)0x40005424)

#define BUTTON_PIN (1U << 3) // GPA3 on the I2C expander.

static unsigned char read_expander(unsigned char reg);
static void write_expander(unsigned char reg, unsigned char segments);

static void button_init(void){
    // BANK=0 register map. GPA3 is input; remaining GPA pins are outputs.
    write_expander(0x00, BUTTON_PIN); // IODIRA = 0x08.
    write_expander(0x0C, BUTTON_PIN); // GPPUA: enable GPA3 pull-up.
}

static void debounce_delay(void){
    // Approximate settling delay; duration depends on CPU speed and compiler.
    for (volatile unsigned int i = 0; i < 100000U; ++i) {
    }
}

static bool button_was_pressed(void){
    // static remembers whether the button was down on the previous call.
    static bool was_pressed = false;
    bool pressed = (read_expander(0x12) & BUTTON_PIN) == 0;

    if (pressed != was_pressed) {
        debounce_delay();

        // Read again to check that the change was not just contact bounce.
        if (pressed == ((read_expander(0x12) & BUTTON_PIN) == 0)) {
            was_pressed = pressed;
            return pressed; // New press: true. Release: false.
        }
    }

    return false;
}

int display (int data){
    // Bits 7..0: F G A B DP C D E. Active-low segments.
    // Bit 3 stays high; GPA3 is the button input, GPB3 is an unused DP.
    switch (data) {
        case 0: return 0b01001000;
        case 1: return 0b11101011;
        case 2: return 0b10001100;
        case 3: return 0b10001001;
        case 4: return 0b00101011;
        case 5: return 0b00011001;
        case 6: return 0b00011000;
        case 7: return 0b11001011;
        case 8: return 0b00001000;
        case 9: return 0b00001001;
        default: return 0b11111111;
    }
}

static inline int toFareinheit(int celsius) {
    return (celsius * 9 / 5) + 32;
}


static inline void wait_tc(void){
    // Wait for the complete NBYTES transfer.
    while ((I2C1_ISR & (1U << 6)) == 0) {
    }
}

void start (unsigned int address, unsigned int rw,unsigned int size){
    
    //set SADD
    I2C1_CR2 &= ~(0x3FF); //CLEAR
    I2C1_CR2 |= ((address & 0x7F) << 1); //SET

    //set RD_WRN bit
    I2C1_CR2 &= ~(0x1 << 10); //CLEAR
    I2C1_CR2 |= (rw << 10); //SET

    //set byte count
    I2C1_CR2 &= ~(0xFFU << 16); // CLEAR  
    I2C1_CR2 |=  (size << 16); // SET

    //set START BIT 
    I2C1_CR2 |= (0x1 << 13);

    do{
        //wait for the START bit to be cleared
    }while((I2C1_CR2 & (1U << 13)) != 0); 

    if (I2C1_ISR & (1U << 4)) {
        //clear NACKF bit
        I2C1_ICR = (1U << 4);
    }

}

void write (unsigned char data){

    do{
    //wait for TXIS = 1
    }while((I2C1_ISR & (0x1 << 1)) == 0);

    //write data to TXDR
    I2C1_TXDR = data ;
}

unsigned char read (){

    do{
        //wait for RXNE = 1
    }while((I2C1_ISR & (0x1 << 2)) == 0);

    //read data from RXDR
    unsigned char data = I2C1_RXDR & 0xFF;

    return data;
}

void stop (void){
    wait_tc();

    //set STOP bit
    I2C1_CR2 |= (0x1 << 14);

    do{
        //wait for STOPF = 1
    }while((I2C1_ISR & (0x1 << 5)) == 0);

    //clear STOPF bit
    I2C1_ICR = (0x1 << 5);
}

unsigned char read_sensor(void){
    start(0b1001101, 0, 1);
    write(0x00);
    wait_tc();

    start(0b1001101, 1, 1);
    unsigned char data = read();
    stop();

    return data;
}

void write_expander(unsigned char reg, unsigned char segments){
    start(0b0100000, 0, 2);

    write(reg);
    write(segments);

    stop();
}

static unsigned char read_expander(unsigned char reg){
    start(0b0100000, 0, 1);
    write(reg);
    wait_tc();

    start(0b0100000, 1, 1);
    unsigned char data = read();
    stop();
    return data;
}

int main(void)
{
    /* Enable clock for GPIOA */
    RCC_IOPENR |= (1 << 0);

    //I2C SETUP
    
    //set to alt function
    GPIOA_MODER &= ~(0xF << (9 * 2)); //CLEAR
    GPIOA_MODER |= (0xA << (9 * 2)); //SET

    GPIOA_AFRH  &= ~(0xF << 4); //CLEAR
    GPIOA_AFRH  |= (0x6 << 4); //SET

    GPIOA_AFRH &= ~(0xF << 8); //CLEAR
    GPIOA_AFRH |= (0x6 << 8); //SET

    GPIOA_OTYPER &= ~(0x3 << 9); //CLEAR
    GPIOA_OTYPER |= (0x3 << 9); //SET

    //enable i2c1 clock
    RCC_APBENR1 |= (1 << 21);

    //clear the PE bit
    I2C1_CR &= ~(1 << 0);

    //SET i2c clock source to HSI16
    RCC_CCIPR &= ~(0x3 << 12); //CLEAR
    RCC_CCIPR |= (0x2 << 12); //SET

    // set timing parameters for I2C
    I2C_TIMINGR &= ~(0xFFFFFFFF); //CLEAR
    I2C_TIMINGR |= 0x3042C3C7; //timing parameters outlined in manual 
    
    //set the PE bit in the CR1 register
    I2C1_CR |= (1U << 0);

    button_init();
    write_expander(0x01, 0x00); // IODIRB: all GPB pins are outputs.

    unsigned char data;
    bool use_fahrenheit = false;

	while (1){

        if (button_was_pressed()) {
            use_fahrenheit = !use_fahrenheit;
        }

        //get temperature
        data = read_sensor();

        // Decode signed 8-bit temperature.
        int temperature = (data & 0x80U) ? (int)data - 256 : (int)data;


        unsigned char tens_segments = 0xFF;
        unsigned char ones_segments = 0xFF;

        if (use_fahrenheit) {
            temperature = toFareinheit(temperature);
        }

        // Split values that fit on the two digits; otherwise leave them blank.
        if (temperature >= 0 && temperature <= 99) {
            tens_segments = display(temperature / 10);
            ones_segments = display(temperature % 10);
        }

        // GPIOA: tens digit. GPIOB: ones digit.

        write_expander(0x12, tens_segments);
        write_expander(0x13, ones_segments);
}
}
