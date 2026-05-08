#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>	     
#include <linux/device.h>     
#include <linux/cdev.h>       
#include <linux/uaccess.h>    
#include <linux/gpio.h>       
#include <linux/string.h>

#define SSD1306_MAX_SEG         128
#define SSD1306_MAX_LINE        7
#define SSD1306_DEF_FONT_SIZE   5
#define MAX_BUFF                256

#define SSD1306_MAGIC 'S'
#define SSD1306_CLEAR        _IO(SSD1306_MAGIC, 1)
#define SSD1306_WRITE       _IOW(SSD1306_MAGIC, 2, uint8_t)

#define DRIVER_AUTHOR "minhhungdenguyn052@gmail.com"
#define DRIVER_DESC   "I2C DEVICE DRIVER FOR OLED 0.96 DISPLAY"


typedef struct ssd1306_i2c_module {
    struct i2c_client *client;
    uint8_t line_num;
    uint8_t cursor_position;
    uint8_t font_size;

    dev_t dev_num;
    struct class *class;
    struct device *device;
    struct cdev m_dev;
}ssd1306_i2c_module_t;

char message[MAX_BUFF];
ssd1306_i2c_module_t* module_ssd1306 = NULL;

static long ssd1306_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = 
{
	.owner		= THIS_MODULE,
    .unlocked_ioctl = ssd1306_ioctl,
};

static const struct i2c_device_id ssd1306_i2c_match_id[] = {
    {"ssd1306", 0},
    { }
};

/*static  const struct of_device_id ssd1306_of_match_ids[] = {
    { .compatible = "ssd1306,match", },
    {  }
};*/

static const unsigned char ssd1306_font[][SSD1306_DEF_FONT_SIZE] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 20 space
    { 0x81, 0x81, 0x18, 0x81, 0x81 }, // 21 !
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, // 22 "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // 23 #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // 24 $
    { 0x23, 0x13, 0x08, 0x64, 0x62 }, // 25 %
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, // 26 &
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, // 27 '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // 28 (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // 29 )
    { 0x14, 0x08, 0x3e, 0x08, 0x14 }, // 2a *
    { 0x08, 0x08, 0x3e, 0x08, 0x08 }, // 2b +
    { 0x00, 0x50, 0x30, 0x00, 0x00 }, // 2c ,
    { 0x08, 0x08, 0x08, 0x08, 0x08 }, // 2d -
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, // 2e .
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, // 2f /
    { 0x3e, 0x51, 0x49, 0x45, 0x3e }, // 30 0
    { 0x00, 0x42, 0x7f, 0x40, 0x00 }, // 31 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 32 2
    { 0x21, 0x41, 0x45, 0x4b, 0x31 }, // 33 3
    { 0x18, 0x14, 0x12, 0x7f, 0x10 }, // 34 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 35 5
    { 0x3c, 0x4a, 0x49, 0x49, 0x30 }, // 36 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 37 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 38 8
    { 0x06, 0x49, 0x49, 0x29, 0x1e }, // 39 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 }, // 3a :
    { 0x00, 0x56, 0x36, 0x00, 0x00 }, // 3b ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 }, // 3c <
    { 0x14, 0x14, 0x14, 0x14, 0x14 }, // 3d =
    { 0x00, 0x41, 0x22, 0x14, 0x08 }, // 3e >
    { 0x02, 0x01, 0x51, 0x09, 0x06 }, // 3f ?
    { 0x32, 0x49, 0x79, 0x41, 0x3e }, // 40 @
    { 0x7e, 0x11, 0x11, 0x11, 0x7e }, // 41 A
    { 0x7f, 0x49, 0x49, 0x49, 0x36 }, // 42 B
    { 0x3e, 0x41, 0x41, 0x41, 0x22 }, // 43 C
    { 0x7f, 0x41, 0x41, 0x22, 0x1c }, // 44 D
    { 0x7f, 0x49, 0x49, 0x49, 0x41 }, // 45 E
    { 0x7f, 0x09, 0x09, 0x09, 0x01 }, // 46 F
    { 0x3e, 0x41, 0x49, 0x49, 0x7a }, // 47 G
    { 0x7f, 0x08, 0x08, 0x08, 0x7f }, // 48 H
    { 0x00, 0x41, 0x7f, 0x41, 0x00 }, // 49 I
    { 0x20, 0x40, 0x41, 0x3f, 0x01 }, // 4a J
    { 0x7f, 0x08, 0x14, 0x22, 0x41 }, // 4b K
    { 0x7f, 0x40, 0x40, 0x40, 0x40 }, // 4c L
    { 0x7f, 0x02, 0x0c, 0x02, 0x7f }, // 4d M
    { 0x7f, 0x04, 0x08, 0x10, 0x7f }, // 4e N
    { 0x3e, 0x41, 0x41, 0x41, 0x3e }, // 4f O
    { 0x7f, 0x09, 0x09, 0x09, 0x06 }, // 50 P
    { 0x3e, 0x41, 0x51, 0x21, 0x5e }, // 51 Q
    { 0x7f, 0x09, 0x19, 0x29, 0x46 }, // 52 R
    { 0x46, 0x49, 0x49, 0x49, 0x31 }, // 53 S
    { 0x01, 0x01, 0x7f, 0x01, 0x01 }, // 54 T
    { 0x3f, 0x40, 0x40, 0x40, 0x3f }, // 55 U
    { 0x1f, 0x20, 0x40, 0x20, 0x1f }, // 56 V
    { 0x3f, 0x40, 0x38, 0x40, 0x3f }, // 57 W
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 58 X
    { 0x07, 0x08, 0x70, 0x08, 0x07 }, // 59 Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 }, // 5a Z
    { 0x00, 0x7f, 0x41, 0x41, 0x00 }, // 5b [
    { 0x02, 0x04, 0x08, 0x10, 0x20 }, // 5c backslash
    { 0x00, 0x41, 0x41, 0x7f, 0x00 }, // 5d ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 }, // 5e ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 }, // 5f _
    { 0x00, 0x01, 0x02, 0x04, 0x00 }, // 60 `
    { 0x20, 0x54, 0x54, 0x54, 0x78 }, // 61 a
    { 0x7f, 0x48, 0x44, 0x44, 0x38 }, // 62 b
    { 0x38, 0x44, 0x44, 0x44, 0x20 }, // 63 c
    { 0x38, 0x44, 0x44, 0x48, 0x7f }, // 64 d
    { 0x38, 0x54, 0x54, 0x54, 0x18 }, // 65 e
    { 0x08, 0x7e, 0x09, 0x01, 0x02 }, // 66 f
    { 0x0c, 0x52, 0x52, 0x52, 0x3e }, // 67 g
    { 0x7f, 0x08, 0x04, 0x04, 0x78 }, // 68 h
    { 0x00, 0x44, 0x7d, 0x40, 0x00 }, // 69 i
    { 0x20, 0x40, 0x44, 0x3d, 0x00 }, // 6a j
    { 0x7f, 0x10, 0x28, 0x44, 0x00 }, // 6b k
    { 0x00, 0x41, 0x7f, 0x40, 0x00 }, // 6c l
    { 0x7c, 0x04, 0x18, 0x04, 0x78 }, // 6d m
    { 0x7c, 0x08, 0x04, 0x04, 0x78 }, // 6e n
    { 0x38, 0x44, 0x44, 0x44, 0x38 }, // 6f o
    { 0x7c, 0x14, 0x14, 0x14, 0x08 }, // 70 p
    { 0x08, 0x14, 0x14, 0x14, 0x7c }, // 71 q
    { 0x7c, 0x08, 0x04, 0x04, 0x08 }, // 72 r
    { 0x48, 0x54, 0x54, 0x54, 0x20 }, // 73 s
    { 0x04, 0x3f, 0x44, 0x40, 0x20 }, // 74 t
    { 0x3c, 0x40, 0x40, 0x20, 0x7c }, // 75 u
    { 0x1c, 0x20, 0x40, 0x20, 0x1c }, // 76 v
    { 0x3c, 0x40, 0x30, 0x40, 0x3c }, // 77 w
    { 0x44, 0x28, 0x10, 0x28, 0x44 }, // 78 x
    { 0x0c, 0x50, 0x50, 0x50, 0x3c }, // 79 y
    { 0x44, 0x64, 0x54, 0x4c, 0x44 }, // 7a z
    { 0x00, 0x08, 0x36, 0x41, 0x00 }, // 7b {
    { 0x00, 0x00, 0x7f, 0x00, 0x00 }, // 7c |
    { 0x00, 0x41, 0x36, 0x08, 0x00 }, // 7d }
    { 0x10, 0x08, 0x08, 0x10, 0x08 }, // 7e ~
    { 0x00, 0x00, 0x00, 0x00, 0x00 }  // 7f
  };

static int ssd1306_i2c_write(struct ssd1306_i2c_module_t *module, unsigned char *buf, unsigned int len)
{
    return i2c_master_send(module->client, buf, len);
}

static int ssd1306_i2c_read(struct ssd1306_i2c_module_t *module, unsigned char *out_buf, unsigned int len)
{
    return i2c_master_recv(module->client, out_buf, len);
}

static void ssd1306_write(struct ssd1306_i2c_module_t *module, bool is_cmd, unsigned char data)
{
    unsigned char buf[2] = {0};

    if (is_cmd == true){
        buf[0] = 0x00;
    }else{
        buf[0] = 0x40;
    }

    buf[1] = data;
    ssd1306_i2c_write(module, buf, 2);
}

static void ssd1306_clear(struct ssd1306_i2c_module_t *module)
{
    unsigned int total = 128*8;
    int i;

    for (i = 0; i < total; i++){
        ssd1306_write(module, false, 0);
    }
}


static int ssd1306_display_init(struct ssd1306_i2c_module_t *module)
{
    msleep(100);
    ssd1306_write(module, true, 0xAE); // Entire Display OFF
    ssd1306_write(module, true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
    ssd1306_write(module, true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency
    ssd1306_write(module, true, 0xA8); // Set Multiplex Ratio
    ssd1306_write(module, true, 0x3F); // 64 COM lines
    ssd1306_write(module, true, 0xD3); // Set display offset
    ssd1306_write(module, true, 0x00); // e offset
    ssd1306_write(module, true, 0x40); // Set first line as the start line of the display
    ssd1306_write(module, true, 0x8D); // Charge pump
    ssd1306_write(module, true, 0x14); // Enable charge dump during display on
    ssd1306_write(module, true, 0x20); // Set memory addressing mode
    ssd1306_write(module, true, 0x00); // Horizontal addressing mode
    ssd1306_write(module, true, 0xA1); // Set segment remap with column address 127 mapped to segment 0
    ssd1306_write(module, true, 0xC8); // Set com output scan direction, scan from com63 to com 0
    ssd1306_write(module, true, 0xDA); // Set com pins hardware configuration
    ssd1306_write(module, true, 0x12); // Alternative com pin configuration, disable com left/right remap
    ssd1306_write(module, true, 0x81); // Set contrast control
    ssd1306_write(module, true, 0x80); // Set Contrast to 128
    ssd1306_write(module, true, 0xD9); // Set pre-charge period
    ssd1306_write(module, true, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK
    ssd1306_write(module, true, 0xDB); // Set Vcomh deselect level
    ssd1306_write(module, true, 0x20); // Vcomh deselect level ~ 0.77 Vcc
    ssd1306_write(module, true, 0xA4); // Entire display ON, resume to RAM content display
    ssd1306_write(module, true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
    ssd1306_write(module, true, 0x2E); // Deactivate scroll
    ssd1306_write(module, true, 0xAF); // Display ON in normal mode
    ssd1306_clear(module);

    return 0;
}

static void ssd1306_set_cursor(struct ssd1306_i2c_module_t *module, uint8_t line_num, uint8_t cursor_position)
{
    if((line_num <= SSD1306_MAX_LINE) && (cursor_position < SSD1306_MAX_SEG)){
        module->line_num = line_num; // Save the specified line number
        module->cursor_position = cursor_position;          // Save the specified cursor position
        ssd1306_write(module, true, 0x21);                  // cmd for the column start and end address
        ssd1306_write(module, true, cursor_position);       // column start addr
        ssd1306_write(module, true, SSD1306_MAX_SEG - 1);   // column end addr
        ssd1306_write(module, true, 0x22);                  // cmd for the page start and end address
        ssd1306_write(module, true, line_num);              // page start addr
        ssd1306_write(module, true, SSD1306_MAX_LINE);      // page end addr
    }
}

static void ssd1306_set_brightness(struct ssd1306_i2c_module_t *module, uint8_t brightness)
{
    ssd1306_write(module, true, 0x81);
    ssd1306_write(module, true, brightness);
}

static void ssd1306_goto_next_line(struct ssd1306_i2c_module_t *module)
{
    module->line_num++;
    module->line_num = (module->line_num & SSD1306_MAX_LINE);
    ssd1306_set_cursor(module, module->line_num, 0);
}

static void ssd1306_print_char(struct ssd1306_i2c_module_t *module, unsigned char c)
{
    uint8_t data_byte;
    uint8_t temp = 0;

    if(((module->cursor_position + module->font_size) >= SSD1306_MAX_SEG) || (c == '\n')) 
        ssd1306_goto_next_line(module);

    if (c != '\n') {
        c -= 0x20;

        do {
            data_byte = ssd1306_font[c][temp];
            ssd1306_write(module, false, data_byte);
            module->cursor_position++;

            temp++;
        }while (temp < module->font_size);

        ssd1306_write(module, false, 0x00);
        module->cursor_position++;
    }
}

static void ssd1306_print_string(struct ssd1306_i2c_module_t *module, unsigned char *str)
{
    while(*str) {
        ssd1306_print_char(module, *str++);
    }
}

static long ssd1306_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    uint8_t brightness;
    int ret = 0;

    if (!module_ssd1306) return -ENODEV;

    switch (cmd) {
        case SSD1306_CLEAR:
            pr_info("SSD1306: Clearing display via ioctl\n");
            ssd1306_clear(module_ssd1306);
            break;

        case SSD1306_WRITE:
            pr_info("SSD1306: Setting brightness via ioctl\n");
            if (copy_from_user(&brightness, (uint8_t __user *)arg, sizeof(brightness))) {
                pr_err("SSD1306: Failed to copy brightness from user\n");
                ret = -EFAULT;
                break;
            }
            ssd1306_set_brightness(module_ssd1306, brightness);
            break;

        default:
            pr_warn("SSD1306: Unknown ioctl command: 0x%x\n", cmd);
            ret = -ENOTTY;
            break;
    }

    return ret;
}

static int ssd1306_create_device_file(ssd1306_i2c_module_t *module)
{
    int ret = 0;

    pr_info("[%s - %d]\n", __func__, __LINE__);

    ret = alloc_chrdev_region(&(module->dev_num), 0, 1, "ssd1306_devnum");
    if (ret < 0){
        pr_err("[%s - %d] can not register major number\n", __func__, __LINE__);
        goto alloc_dev_failed;
    }

    pr_info("Major = %d , Minor = %d\n", MAJOR(module->dev_num), MINOR(module->dev_num)); 

    module->class = class_create(THIS_MODULE, "ssd1306_class");
    if (module->class == NULL) {
        pr_err("[%s - %d] can not create class device\n", __func__, __LINE__);
        goto create_class_failed;
    }

    module->device = device_create(module->class, NULL, module->dev_num, NULL, "ssd1306_device");
    if (module->device == NULL) {
        pr_err("[%s - %d] can not register major number\n", __func__, __LINE__);
        goto device_create_failed;
    }

    cdev_init(&module->m_dev, &fops);
    (module->m_dev).owner = THIS_MODULE;
    (module->m_dev).dev = module->dev_num, 1;

    ret = cdev_add(&module->m_dev, module->dev_num,1);
    if (ret) {
        pr_info("error occur when add properties for struct cdev\n");
        goto cdev_add_fail;
    }

    pr_info("I2C driver module\n");
    return 0;
    
    cdev_add_fail:
        device_destroy(module->class, module->dev_num);
    device_create_failed:
        class_destroy(module->class);
    create_class_failed:
        unregister_chrdev_region(module->dev_num, 1);
    alloc_dev_failed:
        return ret;
}

int ssd1306_i2c_probe (struct i2c_client *client)
{   
    struct ssd1306_i2c_module_t *module;

    module = kmalloc(sizeof(*module), GFP_KERNEL);
    if (!module){
        pr_err("[%s -%d] kmalloc failed\n", __func__, __LINE__);
        return -ENOMEM;
    }

    module->client = client;
    module->line_num = 0;
    module->cursor_position = 0;
    module->font_size = SSD1306_DEF_FONT_SIZE;
    i2c_set_clientdata(client, module);

    ssd1306_display_init(module);
    ssd1306_set_cursor(module,0,0);
    ssd1306_print_string(module, "Hello\nWorld\n");

    if(ssd1306_create_device_file(module) != 0)
    {
        kfree(module);
        pr_err("[%s -%d] create device file failed\n", __func__, __LINE__);
        return -ENODEV;
    }

    module_ssd1306 = module;
    pr_info("[%s -%d]\n", __func__, __LINE__);
    return 0;
}


void ssd1306_i2c_remove (struct i2c_client *client)
{
    struct ssd1306_i2c_module_t *module = i2c_get_clientdata(client);

    ssd1306_print_string(module, "END!!");
    msleep(1000);
    ssd1306_clear(module);
    ssd1306_write(module, true, 0xAE);


    cdev_del(&module->m_dev);
    device_destroy(module->class, module->dev_num);
    class_destroy(module->class);
    unregister_chrdev_region(module->dev_num, 1);

    kfree(module);
    pr_info("SSD1306 driver removed\n");
}

static  const struct i2c_driver ssd1306_i2c_driver = {
        .driver = {
            .name = "ssd1306",
            .owner = THIS_MODULE,
            //.of_match_table = of_match_ptr(ssd1306_of_match_ids), // tao node trong makefile (obj-m = ssd1306.o)
        },
        .probe = ssd1306_i2c_probe,
        .remove = ssd1306_i2c_remove,
        .id_table = ssd1306_i2c_match_id,
};



module_i2c_driver(ssd1306_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_VERSION("1.0"); 