#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

static dev_t fastgpio_dev_number = MKDEV(248,0);
static struct cdev *driver_object;
static struct class *fastgpio_class;
static struct device *fastgpio_dev;

static int driver_open( struct inode *geraete_datei, struct file *instanz )
{
	int err;

	printk("driver_open\n");
	err = gpio_request( 4, "rpi-gpio-4" );
	if (err) {
		printk("gpio_request failed %d\n", err);
		return -1;
	}
	err = gpio_direction_output( 4, 0 );
	if (err) {
		printk("gpio_direction_output failed %d\n", err);
		gpio_free( 4 );
		return -1;
	}
	printk("gpio 4 successfull configured\n");
	return 0;
}


static int driver_close( struct inode *geraete_datei, struct file *instanz )
{
	printk( "driver_close called\n" );
	gpio_free( 4 );
	return 0;
}

static ssize_t driver_write(struct file *instanz,const char __user *user,
	size_t count, loff_t *offset )
{
	unsigned long not_copied, to_copy;
	u32 value=0;

	to_copy = min( count, sizeof(value) );
	not_copied=copy_from_user(&value, user, to_copy);
	printk("driver_read( %d )\n", value );
	if (value==0)
		gpio_set_value(4,0);
	else
		gpio_set_value(4,1);
	//gpio_set_value( 4, value?1:0 );
	return to_copy-not_copied;
}

static struct file_operations fops = {
    .owner  = THIS_MODULE,
    .open   = driver_open,
    .release= driver_close,
    .write   = driver_write,
};

static int __init mod_init( void )
{
	if( register_chrdev_region(fastgpio_dev_number,1,"fastgpio")<0 ) {
		printk("devicenumber (248,0) in use!\n");
		return -EIO;
	}
	driver_object = cdev_alloc(); /* Anmeldeobjekt reserv. */
	if( driver_object==NULL )
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if( cdev_add(driver_object,fastgpio_dev_number,1) )
		goto free_cdev;
	fastgpio_class = class_create( THIS_MODULE, "fastgpio" );
	if( IS_ERR( fastgpio_class ) ) {
		pr_err( "fastgpio: no udev support\n");
		//goto free_cdev;
		return 0;
	}
	fastgpio_dev = device_create(fastgpio_class,NULL,fastgpio_dev_number,
		NULL, "%s", "fastgpio" );
	dev_info( fastgpio_dev, "mod_init called\n" );
	return 0;
free_cdev:
	kobject_put( &driver_object->kobj );
free_device_number:
	unregister_chrdev_region( fastgpio_dev_number, 1 );
	printk("mod_init failed\n");
	return -EIO;
}

static void __exit mod_exit( void )
{
    dev_info( fastgpio_dev, "mod_exit called\n" );
    if (fastgpio_dev_number)
	device_destroy( fastgpio_class, fastgpio_dev_number );
    if (fastgpio_class)
	class_destroy( fastgpio_class );
    cdev_del( driver_object );
    unregister_chrdev_region( fastgpio_dev_number, 1 );
    return;
}

module_init( mod_init );
module_exit( mod_exit );
MODULE_LICENSE("GPL");
