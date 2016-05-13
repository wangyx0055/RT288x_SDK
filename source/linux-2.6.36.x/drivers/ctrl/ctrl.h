#ifndef _GPIO_CTRL_FOR_ANDROID_H_
#define _GPIO_CTRL_FOR_ANDROID_H_
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#define MAX_BUFFER_COUNT 128
#define DELAY_TIME_US  30
#define DELAY_TIME_MS  20

//#define CMD_CTRL_LEARN 0x8001
#define CMD_CTRL_CSN_H 0x8002
#define CMD_CTRL_CSN_L 0x8003
#define CMD_CTRL_DAT_H 0x8005
#define CMD_CTRL_DAT_L 0x8006
#define CMD_CTRL_CLK_H 0x8007
#define CMD_CTRL_CLK_L 0x8008

#define CTRL_DEVICE_NODE_NAME "ctrl"
#define CTRL_DEVICE_FILE_NAME "ctrl"
#define CTRL_DEVICE_CLASS_NAME "ctrl"
typedef struct _MobileCtrlDevice
{
	char   val_in[MAX_BUFFER_COUNT];
	char   val_out[MAX_BUFFER_COUNT];
	//struct semaphore sem;
	struct cdev dev;
}MobileCtrlDevice;




/*#define REG_GPIO_CTRL 0xE0200040
#define REG_GPIO_DATA 0xE0200044
#define REG_GPIO_PUD  0xE0200048
#define GPIO_INPUT 0x0000
#define GPIO_OUTPUT 0x0001

#define PDAT 7
#define PCLK 6
#define PCSN 5*/
void gpio_init(void);
void gpio_learn(void);
void gpio_open(void);
void gpio_close(void);
void gpio_start(void);
void gpio_stop(void);
void gpio_send_ack(void);
int gpio_check_ack(void);
int gpio_write_data(char data);
int gpio_read_data(char* data);
//for dat pin
void gpio_dat_input(void);
void gpio_dat_output(void);
void gpio_dat_low(void);
void gpio_dat_hight(void);
int gpio_dat_status(void);
//for clk pin
void gpio_clk_input(void);
void gpio_clk_output(void);
void gpio_clk_low(void);
void gpio_clk_hight(void);
//for csn pin
void gpio_csn_input(void);
void gpio_csn_output(void);
void gpio_csn_low(void);
void gpio_csn_hight(void);
int gpio_csn_status(void);
#endif
