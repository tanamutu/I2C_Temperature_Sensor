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
W#define I2C1_TXDR  (*(volatile unsigned int *)0x40005428)

void start (unsigned int rw, unsigned int address, unsigned int size){
    
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
    //wait for TXIS bit to be set
    }while((I2C1_ISR & (1U << 1)) == 0);

    //write data to TXDR
    I2C1_TXDR = I2C1_TXDR & ~0xFF | data;
}


int main(void)
{
    /* Enable clock for GPIOA */
    RCC_IOPENR |= (1 << 0);
    
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

    /* Loop forever */
	for(;;);
}