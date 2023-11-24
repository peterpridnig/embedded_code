#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <asm/uaccess.h>
#include <linux/string.h>

static dev_t hd44780_dev_number = MKDEV(511,0);
static struct cdev *driver_object;
static struct class *hd44780_class;
static struct device *hd44780_dev;
static char textbuffer[1024];

/*
RS = GPIO  7 => 119
E  = GPIO  8 => 12
D4 = GPIO 25 => 13
D5 = GPIO 24 => 35
D6 = GPIO 23 => 36
D7 = GPIO 18 => 37

LcdWrite( register, data );

1. set RS line high or low to designate the register you wish to access
2. set R/W line low to indicate a write
3. set DBPORT to output
4. write data to DBPORT
5. set E line high to begin write cycle
6. pause to allow LCD to accept the data
7. set E line low to finish write cycle


Initsequenz:
msleep( 15 );
NibbleWrite( 0, 0x3 );
msleep( 5 );
NibbleWrite( 0, 0x3 );
udelay( 100 );
NibbleWrite( 0, 0x3 );
NibbleWrite( 0, 0x2 );
LcdWrite( 0, 0x28 ); // CMD: 4-Bit Mode, 2 stellige Anzeige, 5x7 Font
LcdWrite( 0, 

http://www.sprut.de/electronic/lcd/    !!!!!
https://www.mikrocontroller.net/articles/AVR-Tutorial:_LCD
*/

static void NibbleWrite( int reg, int value )
{
	gpio_set_value(119,reg);
	gpio_set_value(13, value & 0x1 );
	gpio_set_value(35, value & 0x2 );
	gpio_set_value(36, value & 0x4 );
	gpio_set_value(37, value & 0x8 );
	gpio_set_value(12, 1 );
	udelay(40);
	gpio_set_value(12, 0 );
}

static void LcdWrite( int reg, int value )
{
	NibbleWrite( reg, value>>4 ); // High-Nibble
	NibbleWrite( reg, value&0xf ); // Low-Nibble
}

static int gpio_request_output( int nr )
{
	char gpio_name[12];
	int err;

	snprintf( gpio_name, sizeof(gpio_name), "rpi-gpio-%d", nr );
	err = gpio_request( nr, gpio_name );
	if (err) {
		printk("gpio_request for %s failed with %d\n", gpio_name, err);
		return -1;
	}
	err = gpio_direction_output( nr, 0 );
	if (err) {
		printk("gpio_direction_output failed %d\n", err);
		gpio_free( nr );
		return -1;
	}
	return 0;
}

/*
static int GetRamAddress(int line, int col)
{
}
*/

static int display_init( void )
{
int RamAddrCmd;
  
	printk("display_init\n");
	if (gpio_request_output(119)==-1)
		return -EIO;
	if (gpio_request_output(12)==-1)
   	        goto free119;
	if (gpio_request_output(37)==-1)
		goto free12;
	if (gpio_request_output(36)==-1)
		goto free37;
	if (gpio_request_output(35)==-1)
		goto free36;
	if (gpio_request_output(13)==-1)
		goto free35;
	msleep( 15 );
	NibbleWrite( 0, 0x3 );
	msleep( 5 );
	NibbleWrite( 0, 0x3 );
	udelay( 100 );
	NibbleWrite( 0, 0x3 );
	msleep( 5 );
	NibbleWrite( 0, 0x2 );
	msleep( 5 );
	LcdWrite( 0, 0x28 ); // CMD: 4-Bit Mode, 2 stellige Anzeige, 5x8 Font
	msleep( 2 );
	LcdWrite( 0, 0x01 );
	msleep( 2 );
	LcdWrite( 0, 0x0c ); // Display ein, Cursor aus, Blinken aus

	//line0 #0..19
	RamAddrCmd=(1<<7)+0;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '0' );

        RamAddrCmd=(1<<7)+19;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '1' );

	//line2 #20..39
        RamAddrCmd=(1<<7)+20;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '2' );
	
        RamAddrCmd=(1<<7)+20+19;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '3' );

        //line1 #64..83
        RamAddrCmd=(1<<7)+64;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '4' );

        RamAddrCmd=(1<<7)+64+19;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '5' );	

        //line3 #84..103
        RamAddrCmd=(1<<7)+64+20+0;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '6' );

        RamAddrCmd=(1<<7)+64+20+19;
	LcdWrite( 0, RamAddrCmd );
	LcdWrite( 1, '7' );	
	
	return 0;

free35:  gpio_free( 35 );
free36:  gpio_free( 36 );
free37:  gpio_free( 37 );
free12:  gpio_free(12 );
free119: gpio_free( 119 );
	return -EIO;
}

static int display_exit( void )
{
	printk( "display_exit called\n" );
	gpio_free( 13 );
	gpio_free( 35 );
	gpio_free( 36 );
	gpio_free( 37 );
	gpio_free( 12 );
	gpio_free( 119 );
	return 0;
}

static ssize_t driver_write(struct file *instanz,const char __user *user,
	size_t count, loff_t *offset )
{
	unsigned long not_copied, to_copy;
	int i;

        char myCmdClear[] ="<clear>"; // "...<clear>..."
	char myCmdSetPos[]="<setpos"; // "<setpos Y,X>TEXT"; Y=0..3; X=0..19

	char* p;
	
	to_copy = min( count, sizeof(textbuffer) );
	not_copied=copy_from_user(textbuffer, user, to_copy);
	//printk("driver_write( %s )\n", textbuffer );

	p = strstr(textbuffer, myCmdSetPos); //setpos
	if (p) {
	  LcdWrite( 0, (1<<7)+64 +5); //lin2,col5
	  //atoi(Y,y);
	  //atoi(X,y);
	  //atoi(textbuffer[p+10
	  for (i=10; i<to_copy && textbuffer[i]; i++) {
	      if (isprint(textbuffer[i]))
		LcdWrite( 1, textbuffer[i] );
	  }
	} else if (strstr(textbuffer, myCmdClear) ) { //clear display
	  LcdWrite( 0, 0x01 );
	} else	   //no command, write from the beginning
	  {
	    LcdWrite( 0, (1<<7) ); //line0
	    for (i=0; i<to_copy && textbuffer[i]; i++) {
	      if (isprint(textbuffer[i]))
		LcdWrite( 1, textbuffer[i] );
	      if (i==20) //line1
		LcdWrite( 0, (1<<7)+64 );
	      if (i==40) //line2
		LcdWrite( 0, (1<<7)+20 );
	      if (i==60) //line3
		LcdWrite( 0, (1<<7)+84 );
	    }
	  }
	
	return to_copy-not_copied;
}

static struct file_operations fops = {
    .owner  = THIS_MODULE,
    .write   = driver_write,
};

static int __init mod_init( void )
{
	if( register_chrdev_region(hd44780_dev_number,1,"hd44780")<0 ) {
		printk("devicenumber (511,0) in use!\n");
		return -EIO;
	}
	driver_object = cdev_alloc(); /* Anmeldeobjekt reserv. */
	if( driver_object==NULL )
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if( cdev_add(driver_object,hd44780_dev_number,1) )
		goto free_cdev;
	hd44780_class = class_create( THIS_MODULE, "hd44780" );
	if( IS_ERR( hd44780_class ) ) {
		pr_err( "hd44780: no udev support\n");
		goto free_cdev;
	}
	hd44780_dev = device_create(hd44780_class,NULL,hd44780_dev_number,
		NULL, "%s", "hd44780" );
	dev_info( hd44780_dev, "mod_init called\n" );
	if (display_init()==0)
		return 0;
free_cdev:
	kobject_put( &driver_object->kobj );
free_device_number:
	unregister_chrdev_region( hd44780_dev_number, 1 );
	printk("mod_init failed\n");
	return -EIO;
}

static void __exit mod_exit( void )
{
	dev_info( hd44780_dev, "mod_exit called\n" );
	display_exit();
	device_destroy( hd44780_class, hd44780_dev_number );
	class_destroy( hd44780_class );
	cdev_del( driver_object );
	unregister_chrdev_region( hd44780_dev_number, 1 );
	return;
}

module_init( mod_init );
module_exit( mod_exit );
MODULE_LICENSE("GPL");
