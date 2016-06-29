#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
#include "minnowB.h"

#define SRCLK 509
//please note this is pin 509 on a turbot model. On an MB max, it's 464.
#define DATA 338
#define OE 339
#define RCLK 340
//DOUBLE CHECK ALL THE MOTORS ARE WHAT YOU THINK THEY ARE.


int motorWrite[] = {0,0,0,0,1,1,1,1};//Array of motor states and directions.
//{mActive, m2Active, m3Active, m4Active, mDir,m2Dir,m3Dir,m4Dir} Defaults to all motors off and forward.

//NOTE: IF YOU COMPILE AND FMOD HAS AN UNDEFINED REFERENCE, ADD "-lm" TO THE END OF YOUR COMPILE COMMAND
//EX. gcc motorDriver.c -o mDrive.o -m"

void writeOut(int *ar){
	//Writes an 8 bit array out to the shift register and thus to the motors.
	gpioWrite(SRCLK,0);
	gpioWrite(RCLK,0);
	for(int i = 0 ; i<8;i++){
		gpioWrite(DATA, motorWrite[i]);//Sends the 8 bits into the shift register
		gpioWrite(SRCLK,1);
		gpioWrite(SRCLK,0);
	}
	gpioWrite(RCLK,1);//Once the 8 bits are in place, this writes them out.
	gpioWrite(RCLK,0);
	
}

void setDir(int d, int m){
	//Sets direction 1 (forward) or 0 (backwards) on motor m
	switch(m){
		case 1:
			motorWrite[4]=d;
			break;
		case 2:
			motorWrite[5]=d;
			break;
		case 3:
			motorWrite[6]=d;
			break;
		case 4:
			motorWrite[7]=d;
			break;
		default:
			printf("ERROR IN setDir: INVALID MOTOR\n");
	}
	writeOut(motorWrite);
}

int getDir(int m){
	switch(m){
		case 1:
			return motorWrite[4];
			break;
		case 2:
			return motorWrite[5];
			break;
		case 3:
			return motorWrite[6];
			break;
		case 4:
			return motorWrite[7];
			break;
		default:
			printf("ERROR IN getDir: INVALID MOTOR\n");
			return -1;
			
	}
}

void activateM(int m){
	//Turns on the given motor.
	if(m>=0 && m<8)
		motorWrite[m] = 1;
	else
		printf("ERROR IN activateM: INVALID MOTOR\n");
	writeOut(motorWrite);
}

void deactivateM(int m){
	//Turns off the given motor.
	//This isn't a brake function but rather cuts power to the motor, letting it coast.
	if(m>=0 && m<8)
		motorWrite[m] =0;
	else
		printf("ERROR: INVALID MOTOR");
	writeOut(motorWrite);
}

void drive2(int m, int m2, int time, int power){
	//Drives m and m2 for time milliseconds  at power (-100->100) where negative is backwards.
	if(power < 0){
		setDir(1,m);
		setDir(1,m2);
	}
	else{
		setDir(0,m);
		setDir(0,m2);
		power *=-1;
	}
	int i =0;
	struct timeb t_start;
	struct timeb t_current;
	ftime(&t_start);
	ftime(&t_current);
	int t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));  
	while(t_diff<time){
		if((int)(fmod((double)i,(100.0/((double)power)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m);
			activateM(m2);
		}
		else{
			deactivateM(m);
			deactivateM(m2);
		}
		i++;
		ftime(&t_current);
		t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm)); 
	}
	deactivateM(m);
	deactivateM(m2);
	
}

void drive4(int m, int m2, int m3, int m4, int time, int power){
	//Drives m-m4 for time milliseconds in dir direction(1 =forward, 0 = back) at power (-100->100)
	if(power < 0){
		setDir(1,m);
		setDir(1,m2);
		setDir(1,m3);
		setDir(1,m4);
	}
	else{
		setDir(0,m);
		setDir(0,m2);
		setDir(0,m3);
		setDir(0,m4);
	}
	
	int i =0;
	struct timeb t_start;
	struct timeb t_current;
	ftime(&t_start);
	ftime(&t_current);
	int t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm)); 
	
	while(t_diff<time){
		if((int)(fmod((double)i,(100.0/((double)power)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m);
			activateM(m2);
			activateM(m3);
			activateM(m4);
		}
		else{
			deactivateM(m);
			deactivateM(m2);
			deactivateM(m3);
			deactivateM(m4);
		}
		i++;
		ftime(&t_current);
		t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));
	}
	deactivateM(m);
	deactivateM(m2);
	deactivateM(m3);
	deactivateM(m4);
}

void turn2(int m, int m2, int time, int power, int power2){
//Motor m is left. Motor m2 is right. Time is the duration of the move. 
//Use power and power2 to adjust the turn. 100,100 is straight, 100,-100 would be a right turn in place.
//Power scales the turning accordingly to make a turn faster or slower.
	if(power < 0){
		setDir(1,m);
	}
	else{
		setDir(0,m);
		power *=-1;//Flip the power back to positive so it works in the modulus. 
	}
	if(power2 < 0){
		setDir(1,m2);
	}
	else{
		setDir(0,m2);
		power2 *=-1;
	}
	//start timer
	struct timeb t_start;
	struct timeb t_current;
	ftime(&t_start);
	ftime(&t_current);
	int t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));
	int i = 0;
	while(t_diff<time){
		if((int)(fmod((double)i,(100.0/((double)power)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m);//Runs m at power forward. 
		}
		else{
			deactivateM(m);
		}
		if((int)(fmod((double)i,(100.0/((double)power2)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m2);//Runs m at power forward. 
		}
		else{
			deactivateM(m2);
		}
		i++;
		ftime(&t_current);
		t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));
	}
	deactivateM(m);
	deactivateM(m2);
}

void turn4(int m, int m2, int m3, int m4, int time, int power, int power2){
//Motor m and m3 are left. Motor m2 and m4 are right. Time is the duration of the move. 
//Use power and power2 to adjust the turn. 100,100 is straight, 100,-100 would be a right turn in place.
//Power scales the turning accordingly to make a turn faster or slower.
	if(power < 0){
		setDir(1,m);
		setDir(1,m3);
	}
	else{
		setDir(0,m);
		setDir(0,m3);
		power *=-1;
	}
	if(power2 < 0){
		setDir(1,m2);
		setDir(1,m4);
	}
	else{
		setDir(0,m2);
		setDir(0,m4);
		power2 *=-1;
	}
	//start timer
	struct timeb t_start;
	struct timeb t_current;
	ftime(&t_start);
	ftime(&t_current);
	int t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));
	int i = 0;
	while(t_diff<time){
		if((int)(fmod((double)i,(100.0/((double)power)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m);//Runs m at power forward. 
			activateM(m3);
		}
		else{
			deactivateM(m);
			deactivateM(m3);
		}
		if((int)(fmod((double)i,(100.0/((double)power2)))) == 0){
			//Runs the motors only for the power percent of the loops.
			//Ex. If power = 25, 100/25 = 4. When i%4 is 0 (so i is 0,4,8,12...) the motors fire.
			//This effectively runs the motors at 25% power.
			activateM(m2);//Runs m at power forward. 
			activateM(m4);
		}
		else{
			deactivateM(m2);
			deactivateM(m4);
		}
		i++;
		ftime(&t_current);
		t_diff = (int) (1000.0 * (t_current.time - t_start.time)+ (t_current.millitm - t_start.millitm));
	}
	deactivateM(m);
	deactivateM(m2);
	deactivateM(m3);
	deactivateM(m4);
}

int main(void){
	gpioWrite(OE, 0);//ENABLE OE BY SENDING IT LOW (IT IS REVERSED)
	writeOut(motorWrite);
	printf("Motor 1 dir: %d \n",getDir(1));
	setDir(0,1);
	printf("Motor 1 dir after: %d \n",getDir(1));
	activateM(3);
	deactivateM(3);
	drive2(1,2,1000,100);
	drive4(1,2,3,4,750,50);
	turn2(1,2,10,10,-10);
	turn4(1,2,3,4,10,100,-100);
	return 1;
}





