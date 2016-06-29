#include <stdio.h>
#include <time.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>
#define MAX_BUF 512

//Note: This some of the headers this function references make cauase warnings in your compiler. 
//As long as the header files are present, these are harmless.

struct timespec timer_start(){
	//Starts a nano second timer. Stop it with timer_end()
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

long timer_end(struct timespec start_time){
	//Ends the nano second timer started with timer_start() and then returns the number of elaplsed nanoseconds
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
    return diffInNanos;
}

void waitN(int n){
	//Waits a given number of nanoseconds.
	//Not tremendously precise at that level so it's recommended to only use for millis.
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
}

int gpioRead(int pin){
	//Reads and returns the value on gpio pin "pin"
	//Make sure you use the linux pin number, not the pin number on the header. Ex. Header pin 25 is linux pin 340.
	
	int fd;
	char buf[MAX_BUF]; 
	int gpio = pin;
	//Mimics the "echo <pin> > /sys/class/gpio/export" command
	fd = open("/sys/class/gpio/export",O_WRONLY);
	sprintf(buf, "%d",gpio);
	write(fd, buf , strlen(buf));
	close(fd);
	//Mimics the "echo in > /sys/class/gpio/gpio<pin>/direction" command
	sprintf(buf, "/sys/class/gpio/gpio%d/direction",gpio);
	fd = open(buf,O_WRONLY);
	write(fd, "in",2);
	close(fd);
	//Automatically sets the active low aspect.
	sprintf(buf, "/sys/class/gpio/gpio%d/active_low",gpio);
	fd = open(buf,O_WRONLY);
	write(fd, "0",2);
	close(fd);
	//Reads the value on the pin
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
	//Unexports the pin when done.
	fd = open("/sys/class/gpio/unexport",O_WRONLY);
	sprintf(buf,"%d",gpio);
	write(fd,buf,strlen(buf));
	close(fd);
	return val;
}

int gpioReadDir(int pin, int pull){
	//Reads and returns the value on gpio pin "pin"
	//Make sure you use the linux pin number, not the pin number on the header. Ex. Header pin 25 is linux pin 340.
	int fd;
	char buf[MAX_BUF]; 
	int gpio = pin;
	//Mimics the "echo <pin> > /sys/class/gpio/export" command
	fd = open("/sys/class/gpio/export",O_WRONLY);
	sprintf(buf, "%d",gpio);
	write(fd, buf , strlen(buf));
	close(fd);
	//Mimics the "echo in > /sys/class/gpio/gpio<pin>/direction" command
	sprintf(buf, "/sys/class/gpio/gpio%d/direction",gpio);
	fd = open(buf,O_WRONLY);
	write(fd, "in",2);
	close(fd);
	//Sets the active_low trait according to the user input.
	sprintf(buf, "/sys/class/gpio/gpio%d/active_low",gpio);
	fd = open(buf,O_WRONLY);
	if(pull == 1){
		write(fd, "1",2);}
	else if(pull == 0){
		write(fd, "0",2);}
	close(fd);
	//Reads in the gpio value.
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
	//Unexports the pin
	fd = open("/sys/class/gpio/unexport",O_WRONLY);
	sprintf(buf,"%d",gpio);
	write(fd,buf,strlen(buf));
	close(fd);
	return val;
}

void gpioWrite(int pin, int val){
	//Writes the value "val" to pin "pin"
	//Make sure you use the linux pin number, not the pin number on the header. Ex. Header pin 25 is linux pin 340.
	int fd;
	char buf[MAX_BUF]; 
	int gpio = pin;
	//Initializes the pin
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", gpio); 
	write(fd, buf, strlen(buf));
	close(fd);	
	//Sets the direction to out for writing.
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
	fd = open(buf, O_WRONLY);
	// Set out direction
	write(fd, "out", 3); 
	close(fd);
	//Writes the user value val to the gpio
	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
	fd = open(buf, O_WRONLY);
	if(val == 1)
		write(fd, "1", 1); 
	else
		write(fd,"0",1);
	close(fd);
	//Unexports the pin
	fd = open("/sys/class/gpio/unexport",O_WRONLY);
	sprintf(buf,"%d",gpio);
	write(fd,buf,strlen(buf));
	close(fd);
}

//Experimental Serial functions. Use at your own risk! (You can see working implementations of these in the Minnowboard GPS library)
/*
int initport(int fd){
	//Initialize Serial Port with baudrate of 9600. Feel free to change the two lines if needed.
    int portstatus = 0;

    struct termios options;
    // Get the current options for the port
    tcgetattr(fd, &options);
    // Set the baud rates to 9600
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    // Enable the receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~PARENB;//Enable parity generation on output and parity checking for input.
    options.c_cflag &= ~CSTOPB;//Use 2 stop bits
    options.c_cflag &= ~CSIZE;//Set character size to flag CS8
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;                            // disable hardware flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)

    options.c_cc[VMIN] = 0;     //min carachters to be read
    options.c_cc[VTIME] = 0;    //Time to wait for data (tenths of seconds)

    //Set the new options for the port...
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options)==-1)
    {
        perror("On tcsetattr:");
        portstatus = -1;
    }
    else
        portstatus = 1;


    return portstatus;
}

int open_port(char* port){
	//Opens Serial port for reading.
	//Input port to the desired port
    int fd; 
    fd = open("/dev/%s",port, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1)
    {
        perror("open_port: Unable to open /dev/%s --- \n",port);
    }
    else
        fcntl(fd, F_SETFL, 0);
    return (fd);
}

void reader(char *value, char *port){
	//Reads from the serial port.
	//Pass in a string and this function will overwrite it with the input sentence.
	//Pass in a string port to specity which port to read. ex: "ttyS4"
    int serial_fd = open_port(port);

    if(serial_fd == -1)
        printf("Error opening serial port /dev/%s \n",port);
    else
    {
        if(initport(serial_fd) == -1)
        {
            printf("Error Initializing port");
            close(serial_fd);
        }

        sleep(.5);

		
		char buffer[32];
		int n = read(serial_fd, buffer, sizeof(buffer));
		if (n < 0)
			fputs("read failed!\n", stderr);
		else
		{
			strcpy(value,buffer);
		}
		sleep(.5);
		}
        close(serial_fd);
}
*/