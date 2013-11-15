#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       /* for sleep() */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <math.h>


/* The Neutrino IO port used here corresponds to a single register, which is
 * one byte long */
#define PORT_LENGTH 1

/*Digital input output port configuration,*/
#define DIO_BASE_ADDR 0x280                //Base Address for Ports
#define DIO_PORTB_ADDR 0x09                                   //Base Address for Port_B
#define DIO_CTL_ADDR 0x0B
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

 /* bit 2 = printer initialisation (high to initialise)
  * bit 4 = hardware IRQ (high to enable) */
#define ENABLE_IN 0xFF
#define ENABLE_OUT 0x00

#define LOW 0x00                                          //Values to give pulse.
#define HIGH 0x01


int user_input = 0;
int start_time = 0;
int end_time = 0;
int diff =0;
int first_reading = 0;

/* Handlers for port-A and port-B */
uintptr_t ctrl_handle_portB;

/*Structures for timers used.*/
struct timespec my_timer_value1;

/*Threads functions.*/
void *generate_pulse( void *ptr );
void *out_pulse( void *ptr );
int timer = 20;

typedef union {
        struct _pulse   pulse;
} my_message_t;  //This union is for timer module.

int main( )
{
    printf("This is main");
	int privity_err, servothread ;

    uintptr_t ctrl_handle_portCTL;

    pthread_t thread1;

    //my_timer_value1.tv_nsec = 1000;
    my_timer_value1.tv_nsec = 1500000;
    my_timer_value1.tv_sec = 0;

    /* Give this thread root permissions to access the hardware */
    privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
    if ( privity_err == -1 )
    {
        fprintf( stderr, "can't get root permissions\n" );
        return -1;
    }

    /* Get a handle to the DIO port's Control register */
    ctrl_handle_portB = mmap_device_io( PORT_LENGTH, DIO_BASE_ADDR + DIO_PORTB_ADDR );
    ctrl_handle_portCTL = mmap_device_io( PORT_LENGTH, DIO_BASE_ADDR + DIO_CTL_ADDR);

    /* Initialize the DIO port */
    out8( ctrl_handle_portCTL, 0x10 );
    out8( ctrl_handle_portB, LOW );
    servothread = pthread_create( &thread1, NULL, generate_pulse, NULL);
    //int timer = 0;
    while(1)
    {
    	scanf("%d", &timer);
        if(timer == 0)
        {
        	my_timer_value1.tv_nsec = 100000;
        }
        else if(timer == 1)
        {
        	my_timer_value1.tv_nsec = 400000;
        }
        else if (timer == 2){
        	printf("Set it 2\n");
        	my_timer_value1.tv_nsec = 800000;
        }
        else if (timer == 3){
        	my_timer_value1.tv_nsec = 1100000;
        }
        else if (timer == 4){
        	my_timer_value1.tv_nsec = 1500000;
        }
        else if (timer == 5){
        	printf("Set it 5\n");
        	my_timer_value1.tv_nsec = 2000000;
        	my_timer_value1.tv_sec = 0;
        }
        else
        	printf("This is else\n");
        	my_timer_value1.tv_nsec = 100000;
       fflush(stdout);
    }
    printf("Out of loop.\n");
    pthread_join(thread1, NULL);
    return 0;
}

void *generate_pulse( void *ptr )
{
     printf("Timer Thread.\n");
	int privity_err;
	int runthread;
    privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
    if ( privity_err == -1 )
    {
        fprintf( stderr, "can't get root permissions\n" );
        pthread_exit(NULL);
     }
    struct sigevent         event;
             struct itimerspec       itime;
             timer_t                 timer_id;
             int                     chid, rcvid;
             my_message_t            msg;

             chid = ChannelCreate(0);

             event.sigev_notify = SIGEV_PULSE;
             event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
                                                chid,
                                                _NTO_SIDE_CHANNEL, 0);
             event.sigev_priority = getprio(0);
             event.sigev_code = MY_PULSE_CODE;
             timer_create(CLOCK_REALTIME, &event, &timer_id);

             itime.it_value.tv_sec = 0;
             /* 100 ms = .1 secs */
             itime.it_value.tv_nsec = 20000000;
             itime.it_interval.tv_sec = 0;
             /* 100 ms = .1 secs */
             itime.it_interval.tv_nsec = 20000000;
             timer_settime(timer_id, 0, &itime, NULL);
             // This for loop will update the global_time for every 100 ms which is 1 minute in simulation time.
             for (;;) {
                 rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
                 if (rcvid == 0) { /* we got a pulse */
                      if (msg.pulse.code == MY_PULSE_CODE) {
                    	  my_timer_value1.tv_nsec = timer * 100000;
                    	  out8( ctrl_handle_portB, HIGH );
                    	  nanospin(&my_timer_value1);
                    	  out8( ctrl_handle_portB, LOW );
                      } /* else other pulses ... */
                 } /* else other messages ... */
        }
   printf("\n Pulse Generator : Exiting\n");    //Debug log
   pthread_exit(NULL);
}

#if 0
void *generate_pulse( void *ptr )
{
        int privity_err;
        printf("Pulse_Thread\n");
        privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
        if ( privity_err == -1 )
        {
            fprintf( stderr, "can't get root permissions\n" );
            pthread_exit(NULL);
        }
        while(1){
                out8( ctrl_handle_portB, HIGH );
                usleep(  );
                out8( ctrl_handle_portB, LOW );
                usleep(  );
                out8( ctrl_handle_portB, HIGH );
                usleep( 100 );
                out8( ctrl_handle_portB, LOW );
                usleep( 1000000 );
                out8( ctrl_handle_portB, HIGH );
                usleep( 200 );
                out8( ctrl_handle_portB, LOW );
                usleep( 100000 );
        }
        printf("Pwm thread: Exiting\n");    //Debug Log
        pthread_exit(NULL);
}
#endif
