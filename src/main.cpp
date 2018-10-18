#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "led.h"

using namespace std;
int main()
{
    int ret = -1;
    pthread_detach(pthread_self());
    //need to do some delay
    sleep(5);
    ret = init_led();
    if(ret != 0){
        printf("led init failed \n");
        goto EXIT;
    }
    return 0;

EXIT:
    printf("Cannot do init for controller!\n");
    return -1;
}
