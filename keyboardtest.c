#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	int devfile;
	struct input_event event;

	devfile = open("/dev/input/by-id/usb-Arduino_LLC_Arduino_Micro_HIDPC-if02-event-kbd", O_RDONLY);
	while (1)
	{
		if (read(devfile, &event, sizeof(struct input_event)))
		{
			//printf("%d %d %d\n", event.type, event.code, event.value);
			if (event.type == 1 && event.value == 1)
			{
				if (event.code == 2)
				{
					printf("1");
				}
				if (event.code == 3)
				{
					printf("2");
				}
				if (event.code == 4)
				{
					printf("3");
				}
				if (event.code == 5)
				{
					printf("4");
				}
				if (event.code == 6)
				{
					printf("5");
				}
				if (event.code == 7)
				{
					printf("6");
				}
				if (event.code == 8)
				{
					printf("7");
				}
				if (event.code == 9)
				{
					printf("8");
				}
				if (event.code == 10)
				{
					printf("9");
				}
				if (event.code == 11)
				{
					printf("0");
				}
				if (event.code == 28)
				{
					printf("\n");
				}
			}
		}
	}
	
	return 0;
}