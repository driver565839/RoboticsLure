#include <stdio.h>
#include <time.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#define MAX_BUF 512

int trigPin = 476;//476
int echoPin = 481;//481

struct timespec timer_start(){
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

long timer_end(struct timespec start_time){
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
    return diffInNanos;
}

long microsecondsToInches(long microseconds){
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds){
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void waitN(int n){
	//Slightly more accurate than waitNanos()
	struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
	long dT;
	while(dT<n){
		struct timespec end_time;
   		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    	long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
		dT = diffInNanos;
	}
	dT/=1000;
	//printf("%ld  \n",dT);
}

void waitNanos(int n){
	struct timespec timeNS = timer_start();
	long dT;
	while(dT<n){
		dT = timer_end(timeNS);	
	}
	dT/=1000;
	printf("%ld  \n",dT);
}

int gpioRead(int pin){
	int fd;
	char buf[MAX_BUF]; 
	int gpio = pin;
	fd = open("/sys/class/gpio/export",O_WRONLY);
	sprintf(buf, "%d",gpio);
	write(fd, buf , strlen(buf));
	close(fd);
	sprintf(buf, "/sys/class/gpio/gpio%d/direction",gpio);
	fd = open(buf,O_WRONLY);
	write(fd, "in",2);
	close(fd);
	sprintf(buf, "/sys/class/gpio/gpio%d/active_low",gpio);
	fd = open(buf,O_WRONLY);
	write(fd, "0",2);
	close(fd);
	char value;
	sprintf(buf, "/sys/class/gpio/gpio%d/value",gpio);
	fd = open(buf,O_RDONLY);
	lseek(fd, 0, SEEK_SET);
	read(fd, &value,1);
	close(fd);
	int val = -1;
	if(value == '0')
		val = 0;
	if(value == '1')
		val = 1;
	//printf("val: %d and as char: %c  returned val: ",val,value);
	
	fd = open("/sys/class/gpio/unexport",O_WRONLY);
	sprintf(buf,"%d",gpio);
	write(fd,buf,strlen(buf));
	close(fd);
	return val;
}

void gpioWrite(int pin, int val){
	int fd;
	char buf[MAX_BUF]; 
	int gpio = pin;
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", gpio); 
	write(fd, buf, strlen(buf));
	close(fd);	
	
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
	fd = open(buf, O_WRONLY);
	// Set out direction
	write(fd, "out", 3); 
	close(fd);
	
	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
	fd = open(buf, O_WRONLY);
	//printf("  Writing Value: %d ",val);
	if(val == 1)
		write(fd, "1", 1); 
	else
		write(fd,"0",1);
	close(fd);
	fd = open("/sys/class/gpio/unexport",O_WRONLY);
	sprintf(buf,"%d",gpio);
	write(fd,buf,strlen(buf));
	close(fd);
}

int getDistance(int unit){
	int distance = -1;
	//Write Trig Low
	gpioWrite(trigPin,0);
	//Wait 2 us
	waitN(2000);
	//Write Trig High
	gpioWrite(trigPin,1);
	//Wait 10 us
	waitN(10000);
	///Write trig LOW
	gpioWrite(trigPin,0);
	//When Echo goes HIGH, start timer
	//printf("Waiting for a ping....\n");
	int timeOut = 0;
	while(gpioRead(echoPin)==0 & (timeOut < 100)){timeOut++;}
	if(timeOut>=999){printf("BAD READ \N");}
	else{
		struct timespec start_time;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
		long dT;
		//When echo goes low, stop timer
		timeOut=0;
		//printf("Waiting for Pong\n");
		while(gpioRead(echoPin)==1 & (timeOut<100)){timeOut++;}
		if(timeOut>=999){
			printf("BAD READ \N");
			return -1;
		}
		else{
			struct timespec end_time;
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
			long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
			dT = diffInNanos;
			//The length of the pulse is the distance in time. 
			//printf("Pulse Duration: %ld   \n",dT);
			//Convert to unit of choice
			if(unit == 1){
				distance = microsecondsToInches(dT/1000);
				if(distance > 40)distance = 40;
			}
			else if(unit == 0){
				distance = microsecondsToCentimeters(dT/1000);
				if(distance > 100)distance = 150;
			}
			else
				distance = -1;
		}
	}
	return distance;
	
}



void main(void){
	while(1==1){
		printf("DISTANCE IS: %d \n",getDistance(1));
		sleep(1);
	}
	
}