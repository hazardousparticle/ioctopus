#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/init.h>     /* Needed for the macros */
#include <linux/fs.h> //connect to char file
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <asm/uaccess.h>	/* for put_user */
#include <linux/random.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

//AES-256-CBC password: chocolatefudge69


// the device file to be used
#define DEVICE_NAME "flag"
#define CLASS_NAME "ioctopus"

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file*, unsigned int, unsigned long);

/* By default the device can only be opend by root.
either add the following UDEV rule:  KERNEL=="flag", SUBSYSTEM=="ioctopus", MODE="0666"
or use sudo to cat the file
*/

// value to change via ioctl, answer is 73
unsigned char XORKey = 0;

//the flag
//FLAG-Pb4SXbkpjGN8quuyV7fZ7fJvV2vrVMwY
const char key[] = "KLYIaqcyxecVayipeBOuymZo3QTmLOIiPbqza";
const char crypted[] = {0x44, 0x49, 0x51, 0x47, 0x05, 0x68, 0x48, 0x04, 0x62, 0x74, 0x48, 0x74, 0x58, 0x5a, 0x67, 0x77, 0x14, 0x7a, 0x73, 0x49, 0x49, 0x72, 0x24, 0x40, 0x20, 0x2f, 0x7b, 0x6e, 0x73, 0x50, 0x32, 0x56, 0x6b, 0x7d, 0x75, 0x44, 0x71};
char *flag;

// device parameters
static int Major;
static struct class* charClass  = NULL; // The device driver class pointer
static struct device* charDev  = NULL; // The device pointer

static int isOpen = 0; //only allow the device to open once.

//char *msg_Ptr;

struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};

static char* GetFlag(const char*key, const char* crypted, unsigned char Xorkey)
{
    //number chars: "FLAG-" + 32 byte flag + '\n' + null
    
    int len = strlen(key);
    
    char *msg = vmalloc(len + 1);
    int i;
    for (i = 0; i < len; i++)
    {
        msg[i] = (key[i] ^ crypted[i]) ^ Xorkey;
    }
    
    msg[len] = 0;
    
    return msg;
}

/* Initialize the device driver */
static int __init device_init(void)
{
    // get the kernel to auto assign a major number to bind the device to the driver
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    
	if (Major < 0) {
	  printk(KERN_ALERT "ioctopus: Registering char device failed with %d\n", Major);
	  return Major;
	}
    
    // register the device class
    charClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(charClass)){                // Check for error and clean up if there is
      unregister_chrdev(Major, DEVICE_NAME);
      printk(KERN_ALERT "ioctopus: Failed to register device class\n");
      return PTR_ERR(charClass);          // Correct way to return an error on a pointer
    }

    // Register the device driver
    charDev = device_create(charClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(charDev)){               // Clean up if there is an error
        class_destroy(charClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(Major, DEVICE_NAME);
        printk(KERN_ALERT "ioctopus: Failed to create the device\n");
        return PTR_ERR(charDev);
    }
    
    // make sure the random start key doesn't reveal the ioctl key
    char msg[5] = "FLAG";
    char *msg_ptr;
    while (!strncmp(msg, "FLAG", 4))
    {
        get_random_bytes(&XORKey, 1);
        // randomize the initial XOR KEY
        msg_ptr = GetFlag(key,crypted, XORKey);
        strncpy(msg, msg_ptr,4);
        vfree(msg_ptr);
        msg_ptr = 0;
    }
    
	printk(KERN_INFO "ioctopus: Loaded the device /dev/%s\n", DEVICE_NAME);
    return 0;
}

/* "eject" the device from the system */
static void __exit device_exit(void)
{
    // Unregister the device
	device_destroy(charClass, MKDEV(Major, 0));
	class_unregister(charClass);
	class_destroy(charClass);
	
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "ioctopus: Removed the device\n");
}

/* Info about the  module */
module_init(device_init);
module_exit(device_exit);


MODULE_LICENSE("GPL"); /* Tell the kernel it is a GPL to stop it getting tainted.
Doesnt apply to fedora :( */


/* Called when a process writes to dev file */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "ioctopus: Can't write to the device.\n");
	return -EINVAL;
}

/* Called when a process tries to open the device file */
static int device_open(struct inode *inode, struct file *file)
{
	if (isOpen)
	{
	    printk("ioctopus: Device in use\n");
	    return -EBUSY;
	}
    
    isOpen++;
    // Take the device, so no one else can use it
	try_module_get(THIS_MODULE);
	
	flag = GetFlag(key,crypted, XORKey);
	// wierdly this has to be in the open otherwise it gets stuck in an infinite loop.

	return 0;
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file)
{
	isOpen--;
	/* 
	 * Decrement the usage count, or else once you opened the file, you'll/dev/flag
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);
	return 0;
}


/* Called when a process, which already opened the dev file, attempts to
   read from it. */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
    /*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*flag == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	char * tmp = flag;
	while (length && *flag) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(flag++), buffer++);

		length--;
		bytes_read++;
	}
    put_user('\n', buffer);

	vfree(tmp);

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return ++bytes_read;
}

//ioctl
static long device_ioctl(
    struct file *file,
    unsigned int ioctl_num,/* The number of the ioctl */
    unsigned long ioctl_param) /* The parameter to it */
{
    
    if (!ioctl_num)
    {
        //ioctl 0
        XORKey = (unsigned char)(ioctl_param & 0xff);
        printk("ioctopus: received %i.\n", XORKey);
    }
    
    return 0;
}

