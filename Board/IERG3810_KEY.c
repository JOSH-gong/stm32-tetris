#include "IERG3810_KEY.h"
#include "stm32f10x.h"

void IERG3810_KEY_Init(void)
{
    RCC->APB2ENR |= (1 << 2) | (1 << 6); 
    GPIOA->CRL &= ~(0xF << 0);    
    GPIOA->CRL |= (0x8 << 0);     
    GPIOA->ODR &= ~(1 << 0);      
    GPIOE->CRL &= ~(0xF << 8);    
    GPIOE->CRL |= (0x8 << 8);     
    GPIOE->ODR |= (1 << 2);       
    GPIOE->CRL &= ~(0xF << 12);   
    GPIOE->CRL |= (0x8 << 12);    
    GPIOE->ODR |= (1 << 3);      
}
