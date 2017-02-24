//http://www.gnu.org/software/pth/pth-manual.html
#include <stdio.h>
#include <unistd.h>
#include "pth/pth.h"

pth_uctx_t g_uctx_m;
pth_uctx_t g_uctx;

void rount(void *ctx)
{
    int i = 10000;
    printf("rount %d,sleep 3s\n",i++);
    sleep(3);
    pth_uctx_switch(g_uctx, g_uctx_m);
    printf("rount %d,sleep 3s\n",i++);
    sleep(3);
}

int main()
{
    pth_uctx_create((pth_uctx_t *)&g_uctx_m);
    pth_uctx_create((pth_uctx_t *)&g_uctx);
    pth_uctx_make(g_uctx, NULL, 16*1024, NULL, rount, NULL, g_uctx_m);

    int i = 100;
    printf("main %d\n",i++);
    pth_uctx_switch(g_uctx_m, g_uctx);
    printf("main %d,sleep 3s\n",i++);
    sleep(3);
    pth_uctx_switch(g_uctx_m, g_uctx);
    printf("main %d\n",i++);

    pth_uctx_destroy(g_uctx_m);
    pth_uctx_destroy(g_uctx);
}

