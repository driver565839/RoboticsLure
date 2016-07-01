#include <stdio.h>
#include <time.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "minnowB.h"
#define MAX_BUF 512
#define irPin 480

//Please remmeber to run this script as a super user. Also if using gcc, you may need to add "-lm"
//to the end of your compile command to include math.h

//A standard signal will start like: --------___ and then have 32 bits of data

//The time from the first rise edge to the data rise is ~13.5us, including the software and hardware delays
//dT, the timing variable will display this delay as 13

//Start reading 32 bits at the 13.

int irDecode(int bin){
	//Converts the binary code to a value.
	//NOTE: these codes for each case may change from remote to remote. If you notice no buttons are working:
	//1. Make sure theres a good battery in the remote and that you removed the pull tab over it.
	//2. Uncomment the line that says "USE THIS LINE TO GET CODES" and run the script.
	//Press each button a few times and see what value it returns. THen change the number here to match.
	//Recompile and enjoy.
	
	int val = -1;//Val is a 2digit number XY, x = row, y = col. Start count at 1.
	//Defaults to -1 if no val is read.
	
	//printf("%d ",bin);//USE THIS LINE TO GET CODES
	
	switch(bin){
		case 2139119488:
			//printf("VOL - ");
			val = 11;
			break;
		case 2130763648:
			//printf("PAUSEUNPAUSE ");
			val = 12;
			break;
		case 2122407808:
			//printf("VOL + ");
			val = 13;
			break;
		case 2105696128:
			//printf("SETUP ");
			val = 21;
			break;
		case 2097340288:
			//printf("UP ");
			val = 22;
			break;
		case 2088984448:
			//printf("STOP/MODE ");
			val = 23;
			break;
		case 2072272768:
			//printf("LEFT ");
			val = 31;
			break;
		case 2063916928:
			//printf("ENTER ");
			val = 32;
			break;
		case 2055561088:
			//printf("RIGHT ");
			val = 33;
			break;
		case 2038849408:
			//printf("0/10+ ");
			val = 41;
			break;
		case 2030493568:
			//printf("DOWN ");
			val = 42;
			break;
		case 2022137728:
			//printf("BACK ");
			val = 43;
			break;
		case 2005426048:
			//printf("1 ");
			val = 51;
			break;
		case 1997070208:
			//printf("2 ");
			val = 52;
			break;
		case 1988714368:
			//printf("3 ");
			val = 53;
			break;
		case 1972002688:
			//printf("4 ");
			val = 61;
			break;
		case 1963646848:
			//printf("5 ");
			val = 62;
			break;
		case 1955291008:
			//printf("6 ");
			val = 63;
			break;
		case 1938579328:
			//printf("7 ");
			val = 71;
			break;
		case 1930223488:
			//printf("8 ");
			val = 72;
			break;
		case 1921867648:
			//printf("9 ");
			val = 73;
			break;
		default:
			val = -1;
	}
	return val;
}

int irRead(){
	//reads the irPin until a button comes through.
	//Returns either a two digit int XY where x is the button row and Y is the button column.
	//Or can return 0 signifying a repeated button.
	//Or can return -1, signifying a bad read/interference.
	gpioOpen(irPin);
	int i = 0;
	int len = 32;
	int adr[len];
	int old = 0;
	int new = 0;
	int exit = 0;
	struct timespec timeNS = timer_start();
	int dT = timer_end(timeNS);
	while(exit ==0){
		i = 0;
		old = new;
		new = !gpioQuickRead(irPin);
		if(new == 1 && old == 0){//FOUND A RISING EDGE
			dT = timer_end(timeNS)/1000000;//Makes all the numbers nice
			if(dT ==13){//Found the start bit. 
				while(i+1<len && exit == 0){//Fill up the array.
					old = new;
					new = !gpioQuickRead(irPin);
					if(new == 1 && old == 0){//FOUND A RISING EDGE
						dT = (timer_end(timeNS)/1000000);//Makes all the numbers nice
						if(dT == 1){//If a 0 is found
							adr[i] = 0;
							i++;}
						else if(dT == 2){//If a 1 is found
							adr[i] = 1;
							i++;}
						else if(dT>15 || dT<=0){//If this happens, there was likely an error reading.
							exit = 1;
						}
						timeNS = timer_start();
					}
				}
				exit = 1;
			}
			else if(dT == 11){//The repeat code
				exit = 2;
			}
			timeNS = timer_start();
		}	
		
		if(exit == 1){
			uint32_t bin = 0;
			for(int i = 0;i<len;i++){//Convert array to a 32bit binary.
				bin += pow(2,i) * adr[i];
				adr[i] = 0;//Need to reset otherwise the output changes everytime the code is compiled and run.
			}
			gpioClose(irPin);
			return irDecode(bin);
		}
		else if(exit == 2){
			gpioClose(irPin);
			return 0;
		}
		else{
			for(int i = 0;i<len;i++){
				adr[i] = 0;
			}
		}
	}
	
}
