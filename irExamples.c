#include <stdio.h>
#include "minnowIR.h"


int main(void){
	while(1==1){
		int v = irRead();
		while(v == -1){
			v = irRead();}
	if(v == 0)
		printf("ANOTHER ONE");
	switch(v){
	case 11:
		printf("VOL - ");
		break;
	case 12:
		printf("PAUSEUNPAUSE ");
		break;
	case 13:
		printf("VOL + ");
		break;
	case 21:
		printf("SETUP ");
		break;
	case 22:
		printf("UP ");
		break;
	case 23:
		printf("STOP/MODE ");
		break;
	case 31:
		printf("LEFT ");
		break;
	case 32:
		printf("ENTER ");
		break;
	case 33:
		printf("RIGHT ");
		break;
	case 41:
		printf("0/10+ ");
		break;
	case 42:
		printf("DOWN ");
		break;
	case 43:
		printf("BACK ");
		break;
	case 51:
		printf("1 ");
		break;
	case 52:
		printf("2 ");
		break;
	case 53:
		printf("3 ");
		break;
	case 61:
		printf("4 ");
		break;
	case 62:
		printf("5 ");
		break;
	case 63:
		printf("6 ");
		break;
	case 71:
		printf("7 ");
		break;
	case 72:
		printf("8 ");
		break;
	case 73:
		printf("9 ");
		break;
}
		printf("\n");
		sleep(1);
	}
	
	printf("\n");
}

