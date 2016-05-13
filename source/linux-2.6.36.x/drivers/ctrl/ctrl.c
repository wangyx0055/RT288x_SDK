#include "ctrl.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mobile Ctrl Driver");
MODULE_AUTHOR("ET.Song QQ:87224511");


static int isOne = 0;
static int ctrl_major = 240;
static int ctrl_minor = 0;
static struct class* __class = NULL;
static MobileCtrlDevice* __dev = NULL;
//FOR GPIO
//static void *vGPIOCTRL , *vGPIODATA, *vGPIOPUD;
//#define GPIOCTRL (*(volatile unsigned int *) vGPIOCTRL)
//#define GPIODATA (*(volatile unsigned int *) vGPIODATA)
//#define GPIOPUD  (*(volatile unsigned int *) vGPIOPUD)

int ctrl_open(struct inode* inode, struct file* filp);
int ctrl_release(struct inode* inode, struct file* filp);
int ctrl_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg);
ssize_t ctrl_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos);
ssize_t ctrl_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos);

struct file_operations __fops =
{
	.owner = THIS_MODULE,
	.open = ctrl_open,
	.release = ctrl_release,
	.ioctl = ctrl_ioctl,
	.read = ctrl_read,
	.write = ctrl_write,
};

int
ctrl_open(struct inode* inode, struct file* filp)
{
	MobileCtrlDevice* dev;
	dev = container_of(inode->i_cdev, MobileCtrlDevice, dev);
	filp->private_data = dev;
	return 0;
}

int
ctrl_release(struct inode* inode, struct file* filp)
{
	return 0;
}
int
ctrl_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{
	//MobileCtrlDevice* dev = filp->private_data;
	switch(cmd)
	{
//	case CMD_CTRL_LEARN:
//		gpio_learn();
//		break;
	case CMD_CTRL_CSN_H:
		gpio_csn_output();
		gpio_csn_hight();
		break;
	case CMD_CTRL_CSN_L:
		gpio_csn_output();
		gpio_csn_low();
		break;
	case CMD_CTRL_DAT_H:
		gpio_dat_output();
		gpio_dat_hight();
		break;
	case CMD_CTRL_DAT_L:
		gpio_dat_output();
		gpio_dat_low();
		break;
	case CMD_CTRL_CLK_H:
		gpio_clk_output();
		gpio_clk_hight();
		break;
	case CMD_CTRL_CLK_L:
		gpio_clk_output();
		gpio_clk_low();
		break;
	default:
		return -ENOTTY;
	}
	gpio_csn_input();
	return 0;
}
ssize_t
ctrl_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos)
{
	int i = 0;
	size_t _count = 0;
	char ch;
	char checksum;
	MobileCtrlDevice* dev = filp->private_data;
//	if(down_interruptible(&(dev->sem)))
//	{
//		return -ERESTARTSYS;
//	}
	if(count <= 0)
	{
		//up(&(dev->sem));
		return 0;
	}
	if(count > sizeof(dev->val_in) - 1)
	{
		_count = sizeof(dev->val_in) - 1;
	}
	else
	{
		_count = count;
	}
	memset(dev->val_in, '\0', MAX_BUFFER_COUNT);
	mdelay(1000);
	if (gpio_csn_status() == 0)
	{
		//up(&(dev->sem));
		printk(KERN_ERR "Ctrl:[1(%d)]\n", 0);
		return 0;
	}
	gpio_open();
	udelay(DELAY_TIME_US);
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_hight();
	mdelay(DELAY_TIME_MS);
	gpio_start();
	udelay(DELAY_TIME_US);

	gpio_write_data(0x30);
	udelay(DELAY_TIME_US);
	if (isOne == 0x00)
	{
		gpio_write_data(0x52);
	}
	else
	{
		gpio_write_data(0x62);
	}
	udelay(DELAY_TIME_US);

	gpio_start();
	udelay(DELAY_TIME_US);
	gpio_write_data(0x31);
	udelay(DELAY_TIME_US);
	gpio_read_data(&ch);
	if(ch != 0x00)
	{
		gpio_stop();
		udelay(DELAY_TIME_US);
		gpio_close();
		udelay(DELAY_TIME_US);
		//up(&(dev->sem));
		return -1;
	}
	*(dev->val_in) = ch;
	if (isOne == 0x00)
	{
		checksum = 0xB3;
	}
	else
	{
		checksum = 0xC3;
	}
	
	for(i = 1; i < _count; i++)
	{
		gpio_read_data(&ch);
		udelay(DELAY_TIME_US);
		*(dev->val_in + i) = ch;
		checksum += ch;
	}
	gpio_read_data(&ch);
	udelay(DELAY_TIME_US);
	gpio_stop();
	udelay(DELAY_TIME_US);
	gpio_close();
	udelay(DELAY_TIME_US);


	if(ch != checksum)  //其他项目没做校验，如果读数据通不过，可以关掉本函数；
	{
	
		if (isOne == 0){
			return -EFAULT;
		}
	}



	if(copy_to_user(buf, &(dev->val_in), _count))
	{
	//	up(&(dev->sem));
		printk(KERN_ERR "Ctrl:[3(%d)]\n", -1);
		return -EFAULT;
	}
	//up(&(dev->sem));
	printk(KERN_ERR "Ctrl:[OK]\n");
	return _count;
}
ssize_t
ctrl_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos)
{
	int i = 0;
	size_t _count = 0;
	MobileCtrlDevice* dev = filp->private_data;
//	if(down_interruptible(&(dev->sem)))
//	{
//		return -ERESTARTSYS;
//	}
	if(count > sizeof(dev->val_out) - 1)
	{
		_count = sizeof(dev->val_out) - 1;
	}
	else
	{
		_count = count;
	}
	if(copy_from_user(&(dev->val_out), buf, _count))
	{
//		up(&(dev->sem));
		return -EFAULT;
	}

	if (dev->val_out[0] == 0x30 && dev->val_out[1] == 0x10 && dev->val_out[2] == 0x40)
	{
		isOne = 0;
	}
	else if (dev->val_out[0] == 0x30 && dev->val_out[1] == 0x20 && dev->val_out[2] == 0x50)
	{
		isOne = 1;
	}
	gpio_open();
	udelay(DELAY_TIME_US);
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_hight();

	mdelay(DELAY_TIME_MS);
	gpio_start();
	udelay(DELAY_TIME_US);
	for(i = 0; i < _count; i++)
	{
		gpio_write_data(dev->val_out[i]);
		udelay(DELAY_TIME_US);
	}
	udelay(DELAY_TIME_US);
	gpio_stop();
	udelay(DELAY_TIME_US);
	gpio_close();
	udelay(DELAY_TIME_US);
	memset(dev->val_out, '\0', MAX_BUFFER_COUNT);
//	up(&(dev->sem));
	return _count;
}

static int
__setup_cdev(MobileCtrlDevice* dev)
{
	int retval;
	dev_t devno = MKDEV(ctrl_major, ctrl_minor);
	memset(dev, 0, sizeof(MobileCtrlDevice));
	cdev_init(&(dev->dev), &__fops);
	dev->dev.owner = THIS_MODULE;
	dev->dev.ops = &__fops;
	retval = cdev_add(&(dev->dev), devno, 1);
	if(retval)
	{
		return retval;
	}
	//init_MUTEX(&(dev->sem));
	memset(dev->val_in, '\0', sizeof(dev->val_in));
	memset(dev->val_out, '\0', sizeof(dev->val_out));
	return 0;
}
int __init
ctrl_init(void)
{
	int retval = 0;
	dev_t dev = MKDEV(ctrl_major, 0);
	if (ctrl_major > 0)
	{
		retval = register_chrdev_region(dev, 1, CTRL_DEVICE_NODE_NAME);
	}
	else
	{
		retval = alloc_chrdev_region(&dev, 0, 1, CTRL_DEVICE_NODE_NAME);
	}
	if(retval < 0)
	{
		return retval;
	}
	ctrl_major = MAJOR(dev);
	ctrl_minor = MINOR(dev);
	__dev = kmalloc(sizeof(MobileCtrlDevice), GFP_KERNEL);
	if(!__dev)
	{
		unregister_chrdev_region(MKDEV(ctrl_major, ctrl_minor), 1);
		return -ENOMEM;
	}
	retval = __setup_cdev(__dev);
	if(retval)
	{
		kfree(__dev);
		return retval;
	}
	__class = class_create(THIS_MODULE, CTRL_DEVICE_CLASS_NAME);
	if(IS_ERR(__class))
	{
		retval = PTR_ERR(__class);
		cdev_del(&(__dev->dev));
		return retval;
	}
	device_create(__class, NULL, dev, "%s", CTRL_DEVICE_FILE_NAME);
	gpio_init();
	printk(KERN_ALERT "Succedded To Initialize Mobile Ctrl Device./n");
	return 0;
}
void __exit
ctrl_exit(void)
{
	dev_t devno = MKDEV(ctrl_major, ctrl_minor);
	if(__class)
	{
		device_destroy(__class, MKDEV(ctrl_major, ctrl_minor));
		class_destroy(__class);
	}
	if(__dev)
	{
		cdev_del(&(__dev->dev));
		kfree(__dev);
	}
	unregister_chrdev_region(devno, 1);
}



void
gpio_init(void)
{
	//vGPIOCTRL = ioremap(REG_GPIO_CTRL, 0x10);
	//vGPIODATA = ioremap(REG_GPIO_DATA, 0x10);
	//vGPIOPUD  = ioremap(REG_GPIO_PUD,  0x10);

			 //7 6  5 4  3 2  1 0
	//GPIOPUD = 0x5255;//1010 0010 1010 1010
	gpio_dat_output();
	gpio_clk_output();
	gpio_csn_input();

	gpio_dat_hight();
	gpio_clk_hight();
}

void
gpio_open(void)
{
	gpio_dat_output();
	gpio_clk_output();
	gpio_clk_hight();
	gpio_dat_hight();
}

void
gpio_close(void)
{
	//printk(KERN_ALERT "[liusong] gpio_close B/n");
	gpio_dat_output();
	gpio_clk_output();
	gpio_clk_hight();
	gpio_dat_hight();
}

void
gpio_start(void)
{
	//printk(KERN_ALERT "[liusong] gpio_start B/n");
	gpio_dat_output();
	gpio_clk_output();
	gpio_clk_hight();
	gpio_dat_hight();

	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_dat_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
}


void
gpio_stop(void)
{
	//printk(KERN_ALERT "[liusong] gpio_stop B/n");
	gpio_dat_output();
	gpio_clk_output();
	gpio_clk_low();
	gpio_dat_low();
	udelay(DELAY_TIME_US);
	gpio_clk_hight();
	udelay(DELAY_TIME_US);
	gpio_dat_hight();
	udelay(DELAY_TIME_US);
}


int
gpio_check_ack(void)
{
	//printk(KERN_ALERT "[liusong] gpio_check_ack B/n");
	int ACKSign;
	gpio_dat_input();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_hight();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	ACKSign = gpio_dat_status();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	return ACKSign;
}

void
gpio_send_ack(void)
{
	//printk(KERN_ALERT "[liusong] gpio_send_ack B/n");
	gpio_dat_output();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);

	gpio_clk_hight();
	udelay(DELAY_TIME_US);
	gpio_clk_low();
}

int
gpio_read_data(char* data)
{
	//printk(KERN_ALERT "[liusong] gpio_read_data B/n");
	char readdata = 0;
	char i = 8;
	gpio_dat_input();
	while (i--)
	{
		readdata <<= 1;
		gpio_clk_hight();
		udelay(DELAY_TIME_US);
		readdata |= gpio_dat_status();
		gpio_clk_low();
		udelay(DELAY_TIME_US);
		udelay(DELAY_TIME_US);
	}
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	*data = readdata;
	gpio_send_ack();
	return 0;
}
int
gpio_write_data(char data)
{
	//printk(KERN_ALERT "[liusong] gpio_write_data B/n");
	int Data_Bit,ACKSign;
	int i;

	gpio_dat_output();
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	for(i = 7; i >= 0; i--)
	{
		udelay(DELAY_TIME_US);
		Data_Bit = (data >> i) & 0x01;
		if(Data_Bit)
		{
			gpio_dat_hight();
		}
		else
		{
			gpio_dat_low();
		}
		udelay(DELAY_TIME_US);
		gpio_clk_hight();
		udelay(DELAY_TIME_US);
		gpio_clk_low();
	}
	ACKSign = gpio_check_ack();
	return ACKSign;
}

void
gpio_learn(void)
{
	//printk(KERN_ALERT "[liusong] gpio_learn B/n");
	gpio_open();
	udelay(DELAY_TIME_US);
	gpio_clk_low();
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_clk_hight();
	mdelay(DELAY_TIME_MS);
	gpio_start();
	udelay(DELAY_TIME_US);
	gpio_write_data(0x30);
	udelay(DELAY_TIME_US);
	gpio_write_data(0x10);
	udelay(DELAY_TIME_US);
	gpio_write_data(0x40);
	udelay(DELAY_TIME_US);
	udelay(DELAY_TIME_US);
	gpio_stop();
	udelay(DELAY_TIME_US);
	gpio_close();
	udelay(DELAY_TIME_US);
}

//for dat pin
void
gpio_dat_input(void)
{
	////printk(KERN_ALERT "[liusong] gpio_dat_input B/n");
				 //7 6  5 4  3 2  1 0
	//GPIOPUD = 0x2255;//0010 0010 1010 1010
	//udelay(DELAY_TIME_US);
	mt_set_gpio_mode(69,0);
	mt_set_gpio_dir(69,0);//in
	//GPIOCTRL &= ~(GPIO_OUTPUT << PDAT * 4);
}
void
gpio_dat_output(void)
{
	////printk(KERN_ALERT "[liusong] gpio_dat_output B/n");
		 //7 6  5 4  3 2  1 0
	//GPIOPUD = 0x5255;//1010 0010 1010 1010
	udelay(DELAY_TIME_US);
	mt_set_gpio_mode(69,0);
	mt_set_gpio_dir(69,1);//out
	//GPIOCTRL |= (GPIO_OUTPUT << PDAT * 4);
}
void
gpio_dat_low(void)
{
	mt_set_gpio_mode(69,0);
	mt_set_gpio_out(69,0);
	//GPIODATA &= ~(1 << PDAT);
}
void
gpio_dat_hight(void)
{
	mt_set_gpio_mode(69,0);
	mt_set_gpio_out(69,1);
	//GPIODATA |= (1 << PDAT);
}
int
gpio_dat_status(void)
{
	return mt_get_gpio_in(69);//((GPIODATA >> PDAT) & 0x01);
}
//for clk pin
void
gpio_clk_input(void)
{
	mt_set_gpio_mode(71,0);
	mt_set_gpio_dir(71,0);//in
	//GPIOCTRL &= ~(GPIO_OUTPUT << PCLK * 4);
}
void
gpio_clk_output(void)
{
	mt_set_gpio_mode(71,0);
	mt_set_gpio_dir(71,1);//out
	//GPIOCTRL |= (GPIO_OUTPUT << PCLK * 4);
}
void
gpio_clk_low(void)
{
	mt_set_gpio_mode(71,0);
	mt_set_gpio_out(71,0);
	//GPIODATA &= ~(1 << PCLK);
}
void
gpio_clk_hight(void)
{
	mt_set_gpio_mode(71,0);
	mt_set_gpio_out(71,1);
	//GPIODATA |= (1 << PCLK);
}
//for csn pin
void
gpio_csn_input(void)
{
	mt_set_gpio_mode(57,0);
	mt_set_gpio_dir(57,0);//in
	//GPIOCTRL &= ~(GPIO_OUTPUT << PCSN * 4);
}
void
gpio_csn_output(void)
{
	mt_set_gpio_mode(57,0);
	mt_set_gpio_dir(57,1);//out
	//GPIOCTRL |= (GPIO_OUTPUT << PCSN * 4);
}
void
gpio_csn_low(void)
{
	mt_set_gpio_mode(57,0);
	mt_set_gpio_out(57,0);
	//GPIODATA &= ~(1 << PCSN);
}
void
gpio_csn_hight(void)
{
	mt_set_gpio_mode(57,0);
	mt_set_gpio_out(57,1);
	//GPIODATA |= (1 << PCSN);
}
int
gpio_csn_status(void)
{
	return mt_get_gpio_in(57);//((GPIODATA >> PCSN) & 0x01);
}


module_init(ctrl_init);
module_exit(ctrl_exit);
