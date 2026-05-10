#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SSD1306_MAGIC   'S'
#define SSD1306_CLEAR   _IO(SSD1306_MAGIC, 1)
#define SSD1306_PRINT   _IOW(SSD1306_MAGIC, 2, char *)
#define SSD1306_PATH    "/dev/ssd1306_device"

static void unescape_newline(char *str) {
    char buf[128];
    char *src = str, *dst = buf;

    while (*src && (dst - buf) < 126) {
        if (*src == '\\' && *(src+1) == 'n') {
            *dst++ = '\n';
            src += 2;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    strncpy(str, buf, 127);
}

static int read_option(void) {
    char buf[32];
    if (fgets(buf, sizeof(buf), stdin) == NULL)
        return -1;
    return atoi(buf);
}

int main(void)
{
    int fd;
    int option;
    char message[128];

    fd = open(SSD1306_PATH, O_RDWR);
    if (fd < 0) {
        perror("Cannot open SSD1306 device");
        return -1;
    }

    printf("SSD1306 OLED Controller\n");

    while (1) {
        printf("\n========================\n");
        printf("1. Clear OLED\n");
        printf("2. Print String\n");
        printf("99. Exit\n");
        printf("========================\n");
        printf("Select: ");
        fflush(stdout);

        option = read_option();

        switch (option) {

            case 1:
                if (ioctl(fd, SSD1306_CLEAR) < 0)
                    perror("SSD1306_CLEAR failed");
                else
                    printf("Screen cleared!\n");
                break;

            case 2:
                printf("Enter string (dung \\n de xuong dong): ");
                fflush(stdout);

                if (fgets(message, sizeof(message), stdin) == NULL) {
                    printf("Read error!\n");
                    break;
                }

             
                message[strcspn(message, "\n")] = '\0';

          
                unescape_newline(message);

                if (strlen(message) == 0) {
                    printf("Empty string, skipping!\n");
                    break;
                }

                if (ioctl(fd, SSD1306_PRINT, message) < 0)
                    perror("SSD1306_PRINT failed");
                else
                    printf("Printed: \"%s\"\n", message);
                break;

            case 99:
                printf("Goodbye!\n");
               
                ioctl(fd, SSD1306_CLEAR);
                close(fd);
                return 0;

            case -1:
                printf("EOF detected, exiting...\n");
                close(fd);
                return 0;

            default:
                printf("Invalid option: %d\n", option);
                break;
        }
    }

    close(fd);
    return 0;
}
