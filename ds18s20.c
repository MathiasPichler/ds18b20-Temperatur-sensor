/******************************************************************************/
/* File Name:   ds18b20.c                                                     */
/* Autor: 			Matthias Inhauser                             */
/* Version: 		V1.00                                                 */
/* Date: 				25/07/2017                            */
/* Description: Sensordaden von ds18s20 lesen           		      */
/******************************************************************************/
/* History: 	V1.00  	creation										          									*/
/* 						V1.1		calculation of negative temperatures										*/
/******************************************************************************/
#define sensors

#include <ARMV10_STD.h>
#include <string.h>
#include "sensors.h"

/******************************************************************************/
/*            U N T E R P R O G R A M M:    wait_5us                          */
/*                                                                            */
/* Aufgabe:   waits 5µs                                                       */
/* Input:     factor for 5us   						      */
/* return:	 	                                                      */
/******************************************************************************/
void wait_5us(int mult)
{
	int i,j;
	
	for(i = 0; i < mult; i++)
	{
		for(j = 0; j < 6; j++);
	}
}

/******************************************************************************/
/*            U N T E R P R O G R A M M:    write                             */
/*                                                                            */
/* Aufgabe:   Schreibt command auf 1-wire                                     */
/* Input:     Command in hexadezimal                                          */
/* return:	 	                                                      */
/******************************************************************************/
void write(int command)
{
	int hilf,i=0;
	
	if(command==0xCC)	//initialization
	{
		temp_out=0;				//write reset pulse		
		wait_ms(1);
		temp_out=1;
		while(temp_in==0 || i<=150)
		{
			wait_5us(1);
			i++;
		}
	}				//initialization

	for(i=0;i<=7;i++)	//loop for hex command (8 bit)
	{
		hilf=command;
		hilf |= 0x01;	//check if lsb = 0/1
		if(hilf!=command)	//write "0" Slot
		{
			temp_out=0;
			wait_5us(20);
			temp_out=1;
		}
		if(hilf==command)	//write "1" Slot
		{
			temp_out=0;
			wait_5us(1);
			temp_out=1;
			wait_5us(20);
		}
		command=command>>1;	//next bit
	}
	wait_ms(1);					
}

/******************************************************************************/
/*            U N T E R P R O G R A M M:    read                              */
/*                                                                            */
/* Aufgabe:		Liest von der Datenleitung                            */
/* Input:                                                                     */
/* return:	 	                                                      */
/******************************************************************************/
int read(int mode)
{
	int temp_C,i;
	if(mode==0)			//read temperature from sensor's scrachpad
	{
		for(i=0;i<=15;i++)  		//16 read timeslots
		{
		 	temp_out=0;
			wait_5us(1);
			temp_out=1;
			wait_5us(1);
			temp_C |= (temp_in<<16);
			temp_C = temp_C >> 1;
			wait_5us(15);
		}
	}

	if(mode==1)	 		//wait while temperature conversion is in progress
	{
		do			//readtimeslots until din is not "0"
		{
		 	temp_out=0;
			wait_5us(1);
			temp_out=1;
			wait_5us(1);
			temp_C = temp_in;
			wait_5us(15);
		}while(temp_C==0);
	}
return temp_C;					
}

/******************************************************************************/
/*            U N T E R P R O G R A M M:    convert			      */
/*                                                                            */
/* Aufgabe:   Converts read bits into floating point variable		      */
/* Input:     bits read							      */
/* return:	 	calculated floating point variable		      */
/******************************************************************************/
float convert(int temp_C)
{
	float komma, temp_C_real;
	int hilf;
	
	if(temp_C > 255)	//negative Temperature?
	{
		temp_C &= 0x0FF;
		temp_C = !temp_C;
		temp_C ++;
	}
		
	hilf = temp_C;
	temp_C = (temp_C >> 1);		//LSB = fractional part
	hilf &= 0x01;		//clear all bits except LSB
	komma = hilf;
	komma = komma/2;	//calculate fractional
	temp_C_real = temp_C+komma;
	
	return temp_C_real;				
}

/******************************************************************************/
/* MAIN function 							      */					         													  */
/******************************************************************************/
float temp_get (void) 
{
	int command,conv=1,scrpad_r=0,temp_C=0;
	float temp_real;
	
	temp_out=1;

	command = 0xCC;		//Skip Rom Command (because only 1 ds18b20)
	write(command);
	command = 0x44;		//convert Temperature
	write(command);
	read(conv);				//conv=0 -> Read "0"s while converting
	command = 0xCC;		//Skip Rom
	write(command);
	command = 0xBE;		//read Scratchpad (Memory)
	write(command);
	temp_C=read(scrpad_r);	//scrpad_r = 1 -> Read data bits out of Scratchpad

	temp_real=convert(temp_C);		//Convert the bits into floating point variable

	return(temp_real);
}
