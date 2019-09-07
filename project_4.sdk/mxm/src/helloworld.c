#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "golden.h"
#define MOD 1000
#define PI 3.1415926535897932384626433

#define MATRIX_SIZE 128 // matrix size

#define US_TO_S 0.000001
#define US_TO_MS 0.001


#define APP_SUCCESS            0xAA000000
#define US_TO_MS 0.001
#define APP_SDC            	   0xDD000000 //SDC

float mA[MATRIX_SIZE][MATRIX_SIZE];
float mB[MATRIX_SIZE][MATRIX_SIZE];
float mCS0[MATRIX_SIZE][MATRIX_SIZE];

int s;
//struct sockaddr_in server;
unsigned int buffer[4];
//float *float_golden=(float*)t_float_golden;
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

#define NUM_EXEC 10
//---------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//int Status = 0;
    //unsigned int port = atoi(argv[2]);
    //setup_socket(argv[1],port);

    int i = 0;
    int j = 0;
    int k = 0;
    ///int e = 0;
    int status_app;
    int ex;
    //int count = 0;

    for(i=0; i<MATRIX_SIZE; i++)
    {
    	for(j=0; j<MATRIX_SIZE; j++)
    	{
        	//xil_printf("i");
        	mA[i][j] = (float)(i * PI+4.966228736338716478); // generate nice longs/longs
        	mB[i][j] = (float)(i / PI+2.726218736218716238);
        	mCS0[i][j] = 0;
    	}
    }
    //XTime tStart, tEnd, endexec;
    //int cont=0;


        	//XTime_GetTime(&tStart);
        	//XTime tStart, tEnd;
        	//XTime_GetTime(&tStart);
        	//printf("0\n");
        	//########### control_dut ###########
        while(1){
            for(ex=0;ex<NUM_EXEC;ex++){
                status_app    = 0x00000000;
                	//########### control_dut ###########

                for (i=0; i<MATRIX_SIZE; i++)
                {
                	for(j=0; j<MATRIX_SIZE; j++)
                	{
                		mCS0[i][j] = 0.0;
                		for (k=0; k<MATRIX_SIZE; k++)
                			mCS0[i][j] += mA[i][k] * mB[k][j];
                	}
                }
                	//XTime_GetTime(&endexec);
                	//if (count == 5)
                	//{mCS0[30][47] = 2.35; count=0;}

                    // check for errors
                	//mCS0[10][20]--;
                	//mCS0[30][20]--;

                for (i=0; i<MATRIX_SIZE; i++)
                {
                    //printf("{");
                	for(j=0; j<MATRIX_SIZE; j++)
                	{///
                		//printf("0x%X,",*((unsigned int*)&mCS0[i][j]));
                		if((*(unsigned int*)&mCS0[i][j] != float_golden[i][j]))
                		{
                			if(status_app==0){
                				buffer[0] = 0xDD000000;

                			}else{
                				buffer[0] = 0xCC000000;
                			}

                            //printf("%X %X",*((unsigned int*)&mCS0[i][j]),float_golden[i][j]);
                            status_app = 1;
                			buffer[1] = *((unsigned int*)&i);
                			buffer[2] = *((unsigned int*)&j);
                			buffer[3] = *((unsigned int*)&mCS0[i][j]); // u32, float has 32 bits
                			send_message(4);
                    		}
                	}
                    //printf("},\n");
                	///printf("a");
                }
                //printf("end");
            }

    	//########### control_dut ###########
    	if (status_app == 0x00000000) // sem erros
    	{
    		buffer[0] = APP_SUCCESS; //sem erros
    		send_message(1);
    	}
    }
			return 0;

}
