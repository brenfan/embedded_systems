/* morse code driver */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/leds.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <asm/uaccess.h>

#include "morse_encodings.h"

#define MY_DEVICE_FILE "morse-code"
#define LED_TRIGGER_NAME "morsecode"

#define DEFAULT_DOTTIME 200

#define FIFO_SIZE 256
static DECLARE_KFIFO(morse_fifo, char, FIFO_SIZE);

static unsigned int dottime = DEFAULT_DOTTIME;
static int between_words = 1;
static int letter_last = 0;

module_param(dottime, int, S_IRUGO);
MODULE_PARM_DESC(dottime, " the time for one dot");
/*
	LED
*/

DEFINE_LED_TRIGGER(morsetrigger);

static void led_set(int x)
{
	led_trigger_event(morsetrigger, x ? LED_FULL : LED_OFF);
}

static void led_register(void)
{
	led_trigger_register_simple(LED_TRIGGER_NAME, &morsetrigger);
}

static void led_unregister(void)
{
	led_trigger_unregister_simple(morsetrigger);
}

static void led_pattern(unsigned short x)
{
	while (x) {
		int state = !!(x & 0x8000); // just the most significant bit
		led_set(state);
		x <<= 1;
		msleep(dottime);
	}
	led_set(0);
	msleep(dottime * 3);
}

static int add_to_fifo(unsigned short c)
{
	int count = 0;

	if (letter_last) {
		if (!kfifo_put(&morse_fifo, ' '))
			return -EFAULT;
	}

	do {
		if (c & 0x8000)
			count ++;
		else {
			if (!kfifo_put(&morse_fifo, count == 1 ? '.' : '-'))
				return -EFAULT;
			count = 0;
		}
		c <<= 1;
	} while (c);
	if (!kfifo_put(&morse_fifo, count == 1 ? '.' : '-'))
		return -EFAULT;

	letter_last = 1;
	between_words = 0;
	return 0;
}

static int process_letter(char c)
{
	printk( KERN_DEBUG " processing %c\n", c);
	if (c == ' ' && !between_words) {
		msleep(dottime * 4);
		between_words = 1;
		if (!kfifo_put(&morse_fifo, ' '))
			return -EFAULT;
		if (!kfifo_put(&morse_fifo, ' '))
			return -EFAULT;
		if (!kfifo_put(&morse_fifo, ' '))
			return -EFAULT;
		letter_last = 0;
		return -1;
	}
	if (c > 'z' || c < 'A')
		return -1;
	if (c > 'Z' && c < 'a')
		return -1;
	c = c < 'a' ? c + 32 : c;
	c -= 'a';
	led_pattern(morsecode_codes[(int) c]);
	add_to_fifo(morsecode_codes[(int) c]);
	return 0;
}


/*
	Callbacks
*/

static ssize_t morse_write(struct file *file,
	const char *buff, size_t count, loff_t *ppos)
{
	int buff_idx = 0;

	printk( KERN_DEBUG "morsecode::morse_write(), count %d\n", (int) count);

	for (buff_idx = 0; buff_idx < count; buff_idx++) {
		char ch;
		if (copy_from_user(&ch, buff+ buff_idx, sizeof(ch))) {
			return -EFAULT;
		}

		/* Process character here */
		process_letter(ch);

	}
	if (!kfifo_put(&morse_fifo, '\n')) {
		return -EFAULT;
	}
	*ppos += count;

	return count;
}

static ssize_t morse_read(struct file *file,
	char *buff, size_t count, loff_t *ppos)
{
	int bytes_read = 0;

	printk( KERN_DEBUG "morsecode::morse_read(), buff size %d, fpos %d\n",
		(int) count, (int) *ppos);

	if (kfifo_to_user(&morse_fifo, buff, count, &bytes_read)) {
		return -EFAULT;
	}

	return bytes_read;
}

/*
	Misc support
*/

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.write = morse_write,
	.read = morse_read,
};

static struct miscdevice morsedevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = MY_DEVICE_FILE,
	.fops  = &my_fops
};

/*
	init and exit
*/
static int __init morsecode_init(void)
{
	printk(KERN_INFO "----> morsecode init()\n");
	if (dottime < 1 || dottime > 2000) {
		printk(KERN_WARNING "dottime out of range, using default\n");
		dottime = DEFAULT_DOTTIME;
	}
	INIT_KFIFO(morse_fifo);
	led_register();
	return misc_register(&morsedevice);
}

static void __exit morsecode_exit(void)
{
	printk(KERN_INFO "<---- morsecode exit()\n");
	misc_deregister(&morsedevice);
	led_unregister();
}

module_init(morsecode_init);
module_exit(morsecode_exit);

MODULE_AUTHOR("Brendan Chan");
MODULE_DESCRIPTION("CMPT 433 Assignment 4");
MODULE_LICENSE("GPL");
