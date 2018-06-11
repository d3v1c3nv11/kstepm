

// uICRO A20 --- RM26 TO BE REMOVED !!!!!!!!!

#define DRIVER_AUTHOR "Olimex Ltd. - Chris Boudacoff <chris@protonic.co.uk>"
#define DRIVER_DESC "a20-olinuxino stepmotors driver"

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include "a20stepm.h"
#include <linux/cdev.h>




#define SUCCESS 0
#define DEVICE_NAME DEVICE_FILE_NAME
#define BUF_LEN 128


static void a20stepm_exit(void);
static int a20stepm_init(void);
static long device_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param);
static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *file, char __user * buffer,	size_t length, loff_t * offset);
static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset);

static void __iomem *pb_base;
static void __iomem *ph_base;


static uint32_t portb;
static uint32_t porth;
//static uint32_t portb_m;
static uint32_t econfig0;
static uint32_t econfig1;
static uint32_t econfig2;


static struct hrtimer ch_startX_timer;
static struct hrtimer ch_startY1_timer;
static struct hrtimer ch_startZ_timer;
static struct hrtimer ch_startA_timer;
static struct hrtimer ch_startY2_timer;
static struct hrtimer ch_startT_timer;

static struct timer_list air_timer;
static struct hrtimer x_timer;
static struct hrtimer y1_timer;
static struct hrtimer z_timer;
static struct hrtimer a_timer;
static struct hrtimer y2_timer;
static struct hrtimer t_timer;



//set to 0 = 1 000 000 nS
// min = 1000uS, max = 2000uS
static unsigned long ch_steps[6] = {0,0,0,0,0,0};
static unsigned long ch_steps_done[6] = {0,0,0,0,0,0};

static	int	ch_direction[6] = {0,0,0,0,0,0};  // 0 - off, -1 - back, 1 - forn	
static unsigned long ch_speed[6] = { 1000000, 200000, 300000, 250000, 200000, 500000};
//homestatic	int	home[6] = {0,0,0,0,0,0};
//static long ch_position[6] = {0,0,0,0,0,0};
/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
};


struct cdev stepm;
/* 
 * 
 * 
 * Is the device open right now? Used to prevent
 * concurent access into the same device 
 */
static int Device_Open = 0;
static int Major = MAJOR_NUM;	
/* 
 * The message the device will give when asked 
 */
static char Message[BUF_LEN];
static char Request[BUF_LEN];
/* 
 * How far did the process reading the message get?
 * Useful if the message is larger than the size of the
 * buffer we get to fill in device_read. 
 */
static char *Message_Ptr;
static char *Request_Ptr;

int get_ch_speed(int ch){

int speed;

	if (ch_steps[ch] < ch_steps_done[ch])	
				 speed = int_sqrt(2 * ch_steps[ch] * ACSELERATION);
			else speed = int_sqrt(2 * ch_steps_done[ch] * ACSELERATION);


		if (speed > ch_speed[ch]) speed =  10000000 / ch_speed[ch];
		else speed = 10000000 / speed;
		return speed;
	
}

void set_output(int pin){
	portb = (readl(pb_base + PB_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4) & ~(0x0f << (pin & 0x07) * 4)) | (1 << (pin & 0x07) * 4);
	writel(portb, pb_base + PB_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4);
}
void set_outputh(int pin){
	porth = (readl(ph_base + PH_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4) & ~(0x0f << (pin & 0x07) * 4)) | (1 << (pin & 0x07) * 4);
	writel(porth, ph_base + PH_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4);
}

void set_input(int pin){
	porth = (readl(ph_base + PH_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4) & ~(0x0f << (pin & 0x07) * 4));
	writel(porth, ph_base + PH_CONFIG0 + (pin > 7 ) * 4 + (pin > 15) * 4);
}
void set_low(int pin){
		portb=readl(pb_base+PB_DATA) & ~(1<<pin);
		writel(portb, pb_base + PB_DATA); 
}	

void set_highh(int pin){
		porth=readl(ph_base+PH_DATA) | (1<<pin);
		writel(porth, ph_base + PH_DATA); 
}
void set_lowh(int pin){
		porth=readl(ph_base+PH_DATA) & ~(1<<pin);
		writel(porth, ph_base + PH_DATA); 
}	

unsigned int read_buttons(void){
	
	return readl(ph_base  + PH_DATA);// & 0xfe1f69;//b'1111 1110 0001 1111 0110 1001;
	
}

unsigned int read_pin(int pin){
	
	if ((read_buttons() & (1<<pin)) == 0) return 1;
	else return 0;
	
}

void Emergency_stop(void){
	int x;
 	if ((read_buttons() & 0x800000) == 0){ 


	set_low(AirV);

	for ( x = 0; x<6; x++)
		{
			ch_direction[x]=0;
			ch_steps[x] = 0;
			
		}
	}
}

void set_direction(int pin,int direction){
		// if off return
if ( direction == 0 ) return;	
			// set direction
if ( direction < 0 )		
	portb=readl(pb_base+PB_DATA) | (1<<pin);
	else	
	portb=readl(pb_base+PB_DATA) & ~(1<<pin);
	
writel(portb, pb_base + PB_DATA); 		
}	


// Set enabled channel to LOW after timer expiried.

void air_timer_callback( unsigned long data )
{
	set_low(AirV);
}

enum hrtimer_restart x_timer_callback( struct hrtimer *timer )
{
		set_low(PCHx);
		return HRTIMER_NORESTART;
}
enum hrtimer_restart y1_timer_callback( struct hrtimer *timer )
{
		set_low(PCHy1); 
		return HRTIMER_NORESTART;
}
enum hrtimer_restart z_timer_callback( struct hrtimer *timer )
{
		set_low(PCHz);
		return HRTIMER_NORESTART; 
}
enum hrtimer_restart a_timer_callback( struct hrtimer *timer )
{
		set_low(PCHa);
		return HRTIMER_NORESTART;
}
enum hrtimer_restart y2_timer_callback( struct hrtimer *timer )
{
		set_low(PCHy2);
		return HRTIMER_NORESTART;
}
enum hrtimer_restart t_timer_callback( struct hrtimer *timer )
{
		set_low(PCHt);
		return HRTIMER_NORESTART; 
}



enum hrtimer_restart ch_startX_timer_callback( struct hrtimer *timer_for_restart )
{

ktime_t ktime, currtime , interval;

Emergency_stop();
		// if off return
	if ( ch_direction[CHx] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHx] > 0) {
	 	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHx);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &x_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHx]--;
    ch_steps_done[CHx]++;
    
    if (ch_steps[CHx] == 0) ch_direction[CHx] = 0;
        
						}
						else ch_direction[CHx] = 0;
						

  	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHx)); 
	hrtimer_forward(timer_for_restart, currtime , interval);		
     return HRTIMER_RESTART;					
}



enum hrtimer_restart ch_startY1_timer_callback( struct hrtimer *timer_for_restart )
{

ktime_t ktime, currtime , interval;

Emergency_stop();
 
if ( ch_direction[CHy1] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHy1] > 0) {
	
	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHy1);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &y1_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHy1]--;
	ch_steps_done[CHy1]++;
    
    if (ch_steps[CHy1] == 0) ch_direction[CHy1] = 0;
        
						}
						else ch_direction[CHy1] = 0;
  	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHy1)); 
	hrtimer_forward(timer_for_restart, currtime , interval);							
	return HRTIMER_RESTART;					
}

enum hrtimer_restart ch_startZ_timer_callback( struct hrtimer *timer_for_restart )
{

 ktime_t ktime, currtime , interval;
 
Emergency_stop();

if ( ch_direction[CHz] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHz] > 0) {
	
	 	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHz);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &z_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHz]--;
    ch_steps_done[CHz]++;

    
    if (ch_steps[CHz] == 0) ch_direction[CHz] = 0;
        
						}
						else ch_direction[CHz] = 0;
  	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHz)); 
	hrtimer_forward(timer_for_restart, currtime , interval);							
	return HRTIMER_RESTART;						
}


enum hrtimer_restart ch_startA_timer_callback( struct hrtimer *timer_for_restart )
{

 ktime_t ktime, currtime , interval;
 
Emergency_stop();

if ( ch_direction[CHa] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHa] > 0) {
	
 	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHa);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &a_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHa]--;
    ch_steps_done[CHa]++;
    
    if (ch_steps[CHa] == 0) ch_direction[CHa] = 0;
        
						}
						else ch_direction[CHa] = 0;
						
  	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHa)); 
	hrtimer_forward(timer_for_restart, currtime , interval);							
	return HRTIMER_RESTART;					
}


enum hrtimer_restart ch_startY2_timer_callback( struct hrtimer *timer_for_restart )
{

ktime_t ktime, currtime , interval;

Emergency_stop();

if ( ch_direction[CHy2] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHy2] > 0) {
	
 	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHy2);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &y2_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHy2]--;
	ch_steps_done[CHy2]++;
    
    if (ch_steps[CHy2] == 0) ch_direction[CHy2] = 0;
        
						}
						else ch_direction[CHy2] = 0;
						
  	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHy2)); 
	hrtimer_forward(timer_for_restart, currtime , interval);							
	return HRTIMER_RESTART;					
}


enum hrtimer_restart ch_startT_timer_callback( struct hrtimer *timer_for_restart )
{

ktime_t ktime, currtime , interval;

Emergency_stop();

if ( ch_direction[CHt] == 0 ) return HRTIMER_NORESTART;
	
	if (ch_steps[CHt] > 0) {
	
	 	
// set channel to HIGH  
		portb=readl(pb_base+PB_DATA) | (1<<PCHt);
		writel(portb, pb_base + PB_DATA); // start pulse
		
// start one shot timer
	ktime = ktime_set( 0, PULSE_STEP );
    hrtimer_start( &t_timer, ktime, HRTIMER_MODE_REL );
    
    ch_steps[CHt]--;
    ch_steps_done[CHt]++;
    
    if (ch_steps[CHt] == 0) ch_direction[CHt] = 0;
        
						}
						else ch_direction[CHt] = 0;
	currtime  = ktime_get();
  	interval = ktime_set(0,get_ch_speed(CHt)); 
	hrtimer_forward(timer_for_restart, currtime , interval);							
	return HRTIMER_RESTART;					
}

/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{

	/* 
	 * We don't want to talk to two processes at the same time 
	 */
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	
	/*
	 * Initialize the message 
	 */
	portb=readl(pb_base+PB_DATA);

	//sprintf(Message, "0:%01u:%03lu,1:%01u:%03lu,2:%01u:%03lu,3:%01u:%03lu,4:%01u:%03lu,5:%01u:%03lu,\r\n",
	sprintf(Message, "0:%01u:%lu,1:%01u:%lu,2:%01u:%lu,3:%01u:%lu,4:%01u:%lu,5:%01u:%lu,B:%08x\r\n",
	 read_pin(PHIx),ch_steps[0],read_pin(PHIy1),ch_steps[1],read_pin(PHIz),ch_steps[2],read_pin(PHIa),ch_steps[3],read_pin(PHIy2),
	 ch_steps[4],(read_pin(PHIte)<<1) + read_pin(PHItf),ch_steps[5],read_buttons());
	 
	Message_Ptr = Message;
	Request_Ptr = Request;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	/* 
	 * We're now ready for our next caller 
	 */
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file,	/* see include/linux/fs.h   */
			   char __user * buffer,	/* buffer to be
							 * filled with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/* 
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/* 
	 * If we're at the end of the message, return 0
	 * (which signifies end of file) 
	 */
	if (*Message_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *Message_Ptr) {

		/* 
		 * Because the buffer is in the user data segment,
		 * not the kernel data segment, assignment wouldn't
		 * work. Instead, we have to use put_user which
		 * copies data from the kernel data segment to the
		 * user data segment. 
		 */
		put_user(*(Message_Ptr++), buffer++);
		length--;
		bytes_read++;
	}


	/* 
	 * Read functions are supposed to return the number
	 * of bytes actually inserted into the buffer 
	 */
	return bytes_read;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 */
static ssize_t
	device_write(struct file *file,
	     const char __user * buffer, size_t length, loff_t * offset)
{
    int value;
    int ch;
    char *speed = Request;
    char *pulse = Request;
    char *chn = Request;
	int i;
	ktime_t ktime;
	
	for (i = 0; i < length && i < BUF_LEN-2; i++)
		get_user(Request[i], buffer + i);
		
		
		Request_Ptr = Request;
			
			
		while((chn = strsep(&Request_Ptr,":")) != NULL)
		{
			speed = strsep(&Request_Ptr,":");
			pulse = strsep(&Request_Ptr,",");
			
	
			 if(kstrtoint(chn, 0, &ch) == 0)
					{
						
						
						if (ch == 6) {
							
							
								
								if(kstrtoint(pulse, 0, &value) == 0)
								{
									if (value == 0)
									{
										portb=readl(pb_base+PB_DATA) & ~(1<<AirV);
										writel(portb, pb_base + PB_DATA); 
									
									}else
									{
										portb=readl(pb_base+PB_DATA) | (1<<AirV);
										writel(portb, pb_base + PB_DATA);
										
										
										// start one shot timer
											if(kstrtoint(speed, 0, &value) == 0)
												{
													mod_timer(&air_timer, jiffies + msecs_to_jiffies(value));
												}	
									
									}	
								}							
										
								return i;
						}
						
						
						
						if (ch > 5)  return -1;
						
						if(kstrtoint(speed, 0, &value) == 0)
						{
						
						if (value > MAX_SPEED_PPS) value = MAX_SPEED_PPS;
						//ch_speed[ch] = 10000000 / value;
						ch_speed[ch] = value;
					}
						
						if(kstrtoint(pulse, 0, &value) == 0)
						{
							
					
								ch_direction[ch] = 0;
								ch_steps[ch] = 0;
								ch_steps_done[ch] = 0;
					
								if (value > 0) ch_direction[ch] = 1; else ch_direction[ch] = -1;
								ch_steps[ch] = abs(value);
																
								ktime = ktime_set( 0, get_ch_speed(ch) );
								
								if (ch == CHx){
									hrtimer_start( &ch_startX_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHx,ch_direction[ch]);}
								else if (ch == CHy1){
									hrtimer_start( &ch_startY1_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHy1,ch_direction[ch]);}
								else if (ch == CHz){
									hrtimer_start( &ch_startZ_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHz,ch_direction[ch]);}
								else if (ch == CHa){
									hrtimer_start( &ch_startA_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHa,ch_direction[ch]);}
								else if (ch == CHy2){
									hrtimer_start( &ch_startY2_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHy2,ch_direction[ch]);}
								else if (ch == CHt){
									hrtimer_start( &ch_startT_timer, ktime, HRTIMER_MODE_REL );
									set_direction(DCHt,ch_direction[ch]);}
						
						}
						
		
					}
					
			
	
		}
				
	/* 
	 * Again, return the number of input characters used 
	 */
	return i;
}

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
 static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int i;
	char *temp;
	char ch;

	/* 
	 * Switch according to the ioctl called 
	 */
	switch (ioctl_num) {
	case IOCTL_SET_MSG:
		/* 
		 * Receive a pointer to a message (in user space) and set that
		 * to be the device's message.  Get the parameter given to 
		 * ioctl by the process. 
		 */
		temp = (char *)ioctl_param;

		/* 
		 * Find the length of the message 
		 */
		get_user(ch, temp);
		for (i = 0; ch && i < BUF_LEN; i++, temp++)
			get_user(ch, temp);

		device_write(file, (char *)ioctl_param, i, 0);
		break;

	case IOCTL_GET_MSG:
		/* 
		 * Give the current message to the calling process - 
		 * the parameter we got is a pointer, fill it. 
		 */
		i = device_read(file, (char *)ioctl_param, 99, 0);

		/* 
		 * Put a zero at the end of the buffer, so it will be 
		 * properly terminated 
		 */
		put_user('\0', (char *)ioctl_param + i);
		break;

	case IOCTL_GET_NTH_BYTE:
		/* 
		 * This ioctl is both input (ioctl_param) and 
		 * output (the return value of this function) 
		 */
		return Message[ioctl_param];
		break;
	}

	return SUCCESS;
}

/* Module Declarations */



/* 
 * Initialize the module - Register the character device 
 */
static int __init a20stepm_init(void)
{

	/* 
	 * Register the character device (atleast try) 
	 */
	Major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);


	/* 
	 * Negative values signify an error 
	 */

	if (Major < 0) {
	  printk(KERN_ALERT "STEPM: Registering char device failed with %d\n", Major);
	  return Major;
	}
	

  /* setup your timer to call my_timer_callback */
  hrtimer_init(&ch_startX_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL ); 
  ch_startX_timer.function = &ch_startX_timer_callback;
  
  hrtimer_init(&ch_startY1_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL ); 
  ch_startY1_timer.function = &ch_startY1_timer_callback; 
  
  hrtimer_init(&ch_startZ_timer,  CLOCK_MONOTONIC, HRTIMER_MODE_REL ); 
  ch_startZ_timer.function = &ch_startZ_timer_callback; 
  
  hrtimer_init(&ch_startA_timer,  CLOCK_MONOTONIC, HRTIMER_MODE_REL ); 
  ch_startA_timer.function = &ch_startA_timer_callback;
  
  hrtimer_init(&ch_startY2_timer,  CLOCK_MONOTONIC,HRTIMER_MODE_REL ); 
  ch_startY2_timer.function = &ch_startY2_timer_callback;
    
  hrtimer_init(&ch_startT_timer,  CLOCK_MONOTONIC, HRTIMER_MODE_REL ); 
  ch_startT_timer.function = &ch_startT_timer_callback;  
  
  
  
                  
	setup_timer(&air_timer, air_timer_callback, 0);              
                
   hrtimer_init( &x_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                x_timer.function = &x_timer_callback;
  hrtimer_init( &y1_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                y1_timer.function = &y1_timer_callback;
  hrtimer_init( &z_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                z_timer.function = &z_timer_callback;
  hrtimer_init( &a_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                a_timer.function = &a_timer_callback;
  hrtimer_init( &y2_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                y2_timer.function = &y2_timer_callback;
  hrtimer_init( &t_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
                t_timer.function = &t_timer_callback;

              
                
                
 

//TODO:	
	pb_base = ioremap(SW_PORTB_IO_BASE, 0x20);
	ph_base = ioremap(SW_PORTH_IO_BASE, 0x20);
	
//save configs
	econfig0 = readl(pb_base+PB_CONFIG0);
	econfig1 = readl(pb_base+PB_CONFIG1);
	econfig2 = readl(pb_base+PB_CONFIG2);
	
// set to zeroos
		portb = readl(pb_base + PB_DATA) & ~((1<<PCHx)|(1<<PCHy1)|(1<<PCHz)|(1<<PCHa)|(1<<PCHy2)|(1<<PCHt)|(1<<AirV));
		writel(portb, pb_base + PB_DATA); 
// set to outputs:
	set_output(AirV);
	set_output(PCHx);
	set_output(PCHy1);
	set_output(PCHz);
	set_output(PCHa);
	set_output(PCHy2);
	set_output(PCHt);
	set_output(DCHx);
	set_output(DCHy1);
	set_output(DCHz);
	set_output(DCHa);
	set_output(DCHy2);
	set_output(DCHt);
	
	
	
	set_outputh(POwr);
	set_highh(POwr);
	set_lowh(POwr);
	
	set_input(PHIx);
	set_input(PHIy1);
	set_input(PHIz);
	set_input(PHIa);
	set_input(PHIy2);
	set_input(PHIte);
	set_input(PHItf);
	
	
	printk(KERN_INFO "STEPM: Driver loaded. To use the driver, create a dev file with\n");
	printk(KERN_INFO "STEPM: 'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, MAJOR_NUM);


	return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
static void __exit a20stepm_exit(void)
{
	/* 
	 * Unregister the device 
	 */
	 
/*	 Do Not Restore CONFIGS
//restore configs
	writel(econfig0, pb_base + PB_CONFIG0);
	writel(econfig1, pb_base + PB_CONFIG1);
	writel(econfig2, pb_base + PB_CONFIG2);	
*/
	hrtimer_cancel(&ch_startX_timer);
	hrtimer_cancel(&ch_startY1_timer);	
	hrtimer_cancel(&ch_startZ_timer);	
	hrtimer_cancel(&ch_startA_timer);	
	hrtimer_cancel(&ch_startY2_timer);
	hrtimer_cancel(&ch_startT_timer);
			
	
	del_timer(&air_timer);
	hrtimer_cancel( &x_timer );
	hrtimer_cancel( &y1_timer );
	hrtimer_cancel( &z_timer );
	hrtimer_cancel( &a_timer );
	hrtimer_cancel( &y2_timer );
	hrtimer_cancel( &t_timer );

				
	iounmap(pb_base); //free memory
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_INFO "STEPM: unregister_chrdev /dev/stepm\n");
}

module_init(a20stepm_init);
module_exit(a20stepm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);	
MODULE_SUPPORTED_DEVICE("stepm");
