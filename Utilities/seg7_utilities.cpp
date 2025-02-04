#undef __ARM_FP

#include "mbed.h"
#include "seg7.h"

char convert(char digit)
{
    char leddata;

	if(digit==0)leddata=0b00111111;
	if(digit==1)leddata=0b00000110;
	if(digit==2)leddata=0b01011011;
	if(digit==3)leddata=0b01001111;
	if(digit==4)leddata=0b01100110;
	if(digit==5)leddata=0b01101101;
	if(digit==6)leddata=0b01111101;
	if(digit==7)leddata=0b00000111;
	if(digit==8)leddata=0b01111111;
	if(digit==9)leddata=0b01101111;
    return(leddata);
}

void update(unsigned char val[], int size)			//Function to adjust DIG values
{
    if (size != 4)
        return;

	if(val[0]>=10)
	{
		val[1]=val[1]+1;
		val[0]=0;
	}

	if(val[1]>=10)
	{
		val[2]=val[2]+1;
		val[1]=0;
	}
	if(val[2]>=10)
	{
		val[3]=val[3]+1;
		val[2]=0;
	}
	if(val[3]>=10)
	{
		val[3]=0;
	}

    return;
}
