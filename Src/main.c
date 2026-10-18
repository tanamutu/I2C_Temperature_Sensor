//REGISTER DEFINITION
#define GPIOA_MODER     (*(volatile unsigned int *) 0x50000000)
#define GPIOA_OTYPER    (*(volatile unsigned int *) 0x50000004)
#define GPIOA_OSPEEDR   (*(volatile unsigned int *) 0x50000008)
#define GPIOA_PUPDR     (*(volatile unsigned int *) 0x5000000C)
#define GPIOA_IDR       (*(volatile unsigned int *) 0x50000010)
#define GPIOA_ODR       (*(volatile unsigned int *) 0x50000014)
#define GPIOA_BSRR      (*(volatile unsigned int *) 0x50000018)
#define GPIOA_LCKR      (*(volatile unsigned int *) 0x5000001C)
#define GPIOA_AFRL      (*(volatile unsigned int *) 0x50000020)
#define GPIOA_AFRH      (*(volatile unsigned int *) 0x50000024)
#define GPIOA_BRR       (*(volatile unsigned int *) 0x50000028)

#define RCC_IOPENR  (*(volatile unsigned int *)0x40021034)
#define RCC_APBENR1 (*(volatile unsigned int *)0x4002103C)
#define RCC_CCIPR   (*(volatile unsigned int *)0x40021054)

#define I2C_TIMINGR (*(volatile unsigned int *)0x40005410)
#define I2C1_CR     (*(volatile unsigned int *)0x40005400)

int main(void)
{
    /* Enable clock for GPIOA */
    RCC_IOPENR |= (1 << 0);
    
    //set to alt function
    GPIOA_MODER &= ~(0x3 << (9 * 2)); //CLEAR
    GPIOA_MODER |= (0x2 << (9 * 2)); //SET

    GPIOA_MODER &= ~(0x3 << (10 * 2)); //CLEAR
    GPIOA_MODER |= (0x2 << (10 * 2)); //SET

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
    I2C_TIMINGR |= 0x3042C3C7; //SET timing parameters
    
    //set the PE bit in the CR1 register
    I2C1_CR |= (1U << 0);

  

    
    
    

    


    /* Loop forever */
	for(;;);
}