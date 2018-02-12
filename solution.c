#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>      /* ioctl */

#define DEVICE "/dev/flag"

int main(int argc, char*argv[])
{
    // send the ioctl to the device
    int fileDescriptor = open(DEVICE,0);
    if (fileDescriptor < 0)
    {
        printf("Error\n");
        return -1;
    }

    ioctl(fileDescriptor,0,73);
    close(fileDescriptor);
    
    // read the file
    fileDescriptor = open(DEVICE,0);
    if (fileDescriptor < 0)
    {
        printf("Error\n");
        return -1;
    }
    char fileContent[39] = {0};
    read(fileDescriptor, fileContent, sizeof(fileContent));
    close(fileDescriptor);
    
    printf("%s", fileContent);

    return 0;
}
