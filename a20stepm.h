#ifndef A20STEPM_H
#define A20STEPM_H

#include <linux/ioctl.h>


/*
 *   cat /dev/stepm =  0:0:7459,1:0:7967,2:0:000,3:0:000,4:0:000,5:0:000,
 *                    ch:H:steps, 
 *                       o:remaining
 * 						 m							
 * 		`				 e
 * 
 *  echo 0:100:1000, > /dev/stepm
 * 		ch:PPS:steps
 *  
 *  PPS = pulses per second [speed]
 * 
 * 
 */





/* 
 * The major device number. We can't rely on dynamic 
 * registration any more, because ioctls need to know 
 * it. 
 */
#define MAJOR_NUM 210


//Pulse length one step = 20uS [20000nS], 100 steps per seconds
#define PULSE_STEP 20000
#define SPEED_DEFAULT_PPS 50
#define MAX_SPEED_PPS 250
#define	ACSELERATION	10


#define	CHx		0
#define PCHx	14
#define	DCHx	13
#define PHIx	16

#define	CHy1	1
#define	PCHy1	12
#define	DCHy1	11
#define	PHIy1	15

#define	CHz		2
#define	PCHz	10
#define	DCHz	7
#define	PHIz	14

#define	CHa		3
#define	PCHa	6
#define	DCHa	5
#define	PHIa	12

#define	CHy2	4
#define	PCHy2	4
#define	DCHy2	3
#define	PHIy2	10

#define	CHt		5
#define	PCHt	17
#define	DCHt	16
#define	PHIte	9
#define	PHItf	7

#define	POwr	13	
#define AirV	15


#define SW_PORTB_IO_BASE 0x01C20824
#define PB_CONFIG0 0x00
#define PB_CONFIG1 0x04
#define PB_CONFIG2 0x08
#define PB_DATA    0x10

#define SW_PORTH_IO_BASE 0x01C208FC
#define PH_CONFIG0 0x00
#define PH_CONFIG1 0x04
#define PH_CONFIG2 0x08
#define PH_DATA    0x10


/* 
 * Set the message of the device driver 
 */
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)
/*
 * _IOR means that we're creating an ioctl command 
 * number for passing information from a user process
 * to the kernel module. 
 *
 * The first arguments, MAJOR_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */

/* 
 * Get the message of the device driver 
 */
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)
/* 
 * This IOCTL is used for output, to get the message 
 * of the device driver. However, we still need the 
 * buffer to place the message in to be input, 
 * as it is allocated by the process.
 */

/* 
 * Get the n'th byte of the message 
 */
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)
/* 
 * The IOCTL is used for both input and output. It 
 * receives from the user a number, n, and returns 
 * Message[n]. 
 */

/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "stepm"

#endif
