/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "golden.h"
#include "ff.h"

#define MOD 1000

#define NUM_EXEC 100
#define MAXARRAY 50000
#define US_TO_S 0.000001
#define US_TO_MS 0.001

#define APP_SUCCESS            0xAA000000
#define APP_SDC            	   0xDD000000 //SDC
#define US_TO_MS 0.001

// 1 if using control_dut

static FATFS fs_header;
static FIL fin;

unsigned int buffer[4];

// TIMER instance
//XTmrCtr timer_dev;

//void qsort (void *__base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
void send_message(size_t size);
void send_message(size_t size){
	int i;

	for(i=0;i<size;i++){
		/*for(j=3;j>-1;j--){
			temp=(unsigned char*)&buffer[i];
				XUartPs_Send(&Uart_Ps,&temp[j], 1);
		}*/
		printf( "%08X" , *(unsigned int*)&buffer[i] );
		fflush(stdout);
	}
}

//void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*));
int compare(const void *elem1, const void *elem2);
int compare(const void *elem1, const void *elem2)
{
  /* D = [(x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2]^(1/2) */
  /* sort based on distances from the origin... */
 // printf("hello\n\r");
  double distance1, distance2;

  distance1 = *((double*)elem1);
  distance2 = *((double*)elem2);
//printf("%f %f %d",distance1,distance2,(distance1 > distance2) ? 1 : ((distance1 < distance2) ? -1 : 0));
  return (distance1 > distance2) ? 1 : ((distance1 < distance2) ? -1 : 0);
}

int array[3*MAXARRAY];
double distance[MAXARRAY];
long long temp_gold[MAXARRAY];

//---------------------------------------------------------------------------
int main(int argc, char **argv)
{
    unsigned int i,br;
    //long charcnt=0;
     FRESULT res=0,rc;

    int count=0,count2=0;
    if ((rc = f_mount(&fs_header,"0:/",0)) != FR_OK) {
    	buffer[0]=0xE1000000;
		buffer[1]=*(unsigned int*)&rc;
		send_message(2);
	   return(-1);
	}

    res=f_open(&fin, "0:/input_qsort.bin", FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
    	 buffer[0]=0xE2000000;
    	 buffer[1]=*(unsigned int*)&res;
		  send_message(2);
        exit(-1);
    }
//printf("started\n\r");
    res=f_read(&fin,array,3*MAXARRAY*sizeof(int),&br);
    if (res != FR_OK) {
    	buffer[0]=0xE3000000;
		 buffer[1]=*(unsigned int*)&res;
		  send_message(2);
        exit(-1);
    }

    while(1){
    	int num_SDCs = 0;
         count=0;
         count2=0;
    	//printf("0\n");
        initGold(temp_gold);
    	//status_app    = 0x00000000;
    	//########### control_dut ###########
        while((count < 3*MAXARRAY)) {
         distance[count2] = sqrt(pow(array[count], 2) + pow(array[count+1], 2) + pow(array[count+2], 2));
         count+=3;
         count2++;
        }

    	qsort(distance,MAXARRAY,sizeof(double),compare);


        for (i=0;i<MAXARRAY;i++)
        {
            /*char test[200];
            long *test1=(long*)&distance[i];
            long *test2=(long*)&temp_gold[i];
            sprintf(test,"gold[%d]=0x%08lx%08lx;\n\r",i,test1[1],test1[0]);
            //sprintf(test,"gold[%d]=0x%lx%lx;\n\r",i,test1[1],test1[0]);
            printf (test);*/
            //printf("gold[%d]=0x%llx;\n",i,*(unsigned long long*)&distance[i]);
            if (*((long long *)&distance[i]) != temp_gold[i])
              {

                  num_SDCs++;


              }
        	//printf("a");
        }
        //return 0;
        //printf("end");
        //while(1);
    	//########### control_dut ###########
    	if (num_SDCs == 0) // sem erros
    	{
            //printf("ok\n");
    		buffer[0] = APP_SUCCESS; //sem erros
    		send_message(1);


        }
        else{
            buffer[0] = 0xDD000000; // SDC
            buffer[1] = *((uint32_t*)&num_SDCs);
            send_message(2);
        }

    }

    return 0;
}
