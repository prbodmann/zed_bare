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
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include <math.h>
#include "golden.h"
#define MAXSIZE 32768
#define MAXWAVES 8
#define  DDC_PI  (3.14159265358979323846)
float RealIn[MAXSIZE];
float ImagIn[MAXSIZE];
float RealOut[MAXSIZE];
float ImagOut[MAXSIZE];
float coeff[MAXSIZE];
float amp[MAXSIZE];

#define NUM_EXEC 1
#define BITS_PER_WORD   (sizeof(unsigned) * 8)

unsigned int buffer[10];
int IsPowerOfTwo ( unsigned x );
int IsPowerOfTwo ( unsigned x )
{
    if ( x < 2 )
        return FALSE;

    if ( x & (x-1) )        // Thanks to 'byang' for this cute trick!
        return FALSE;

    return TRUE;
}

unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo );
unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo )
{
    unsigned i;

    if ( PowerOfTwo < 2 )
    {
        fprintf (
            stderr,
            ">>> Error in fftmisc.c: argument %d to NumberOfBitsNeeded is too small.\n",
            PowerOfTwo );

        exit(1);
    }

    for ( i=0; ; i++ )
    {
        if ( PowerOfTwo & (1 << i) )
            return i;
    }
}


unsigned ReverseBits ( unsigned index, unsigned NumBits );
unsigned ReverseBits ( unsigned index, unsigned NumBits )
{
    unsigned i, rev;

    for ( i=rev=0; i < NumBits; i++ )
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }

    return rev;
}

double Index_to_frequency ( unsigned NumSamples, unsigned Index );
double Index_to_frequency ( unsigned NumSamples, unsigned Index )
{
    if ( Index >= NumSamples )
        return 0.0;
    else if ( Index <= NumSamples/2 )
        return (double)Index / (double)NumSamples;

    return -(double)(NumSamples-Index) / (double)NumSamples;
}


/*--- end of file fftmisc.c---*/

#define CHECKPOINTER(p)  CheckPointer(p,#p)

static void CheckPointer ( void *p, char *name )
{
    if ( p == NULL )
    {
        fprintf ( stderr, "Error in fft_float():  %s == NULL\n", name );
        exit(1);
    }
}

void fft_float (
    unsigned  NumSamples,
    int       InverseTransform
);
void fft_float (
    unsigned  NumSamples,
    int       InverseTransform
     )
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, n;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = 2.0 * DDC_PI;
    double tr, ti;     /* temp real, temp imaginary */

    if ( !IsPowerOfTwo(NumSamples) )
    {
        fprintf (
            stderr,
            "Error in fft():  NumSamples=%u is not power of two\n",
            NumSamples );

        exit(1);
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    CHECKPOINTER ( RealIn );
    CHECKPOINTER ( RealOut );
    CHECKPOINTER ( ImagOut );

    NumBits = NumberOfBitsNeeded ( NumSamples );

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == NULL) ? 0.0 : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        double ar[3], ai[3];


        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar[0] = w*ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w*ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + BlockEnd;
                tr = ar[0]*RealOut[k] - ai[0]*ImagOut[k];
                ti = ar[0]*ImagOut[k] + ai[0]*RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        double denom = (double)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}

void send_message(size_t size);
void send_message(size_t size){
	int i;
	for(i=0;i<size;i++){
		/*for(j=3;j>-1;j--){
			temp=(unsigned char*)&buffer[i];
				XUartPs_Send(&Uart_Ps,&temp[j], 1);
		}*/
		xil_printf( "%08X" , *(unsigned int*)&buffer[i] );
		//fflush(stdout);
	}
	fflush(stdout);
}


int main(int argc, char *argv[]) {
	//unsigned MAXSIZE;
	//unsigned MAXWAVES;
	unsigned i,j;
    int ex;

	int invfft=0;

    int status_app=0;
    //unsigned int port = atoi(argv[2]);
    //setup_socket(argv[1],port);

while(1){



        for(ex=0;ex<NUM_EXEC;ex++){

            status_app=0;
            srand(1);

            /* Makes MAXWAVES waves of random amplitude and period */
            for(i=0;i<MAXWAVES;i++)
            {

                coeff[i] = rand()%1000;

                amp[i] = rand()%1000;

                    //amp[i] = rand()%1000;


            }
            for(i=0;i<MAXSIZE;i++)
            {
                  /*   RealIn[i]=rand();*/
                 RealIn[i]=0;
                 for(j=0;j<MAXWAVES;j++)
                 {
                     /* randomly select sin or cos */
                     if (rand()%2)
                     {

                         RealIn[i]+=coeff[j]*cos(amp[j]*i);


                     }
                     else
                     {
                        RealIn[i]+=coeff[j]*sin(amp[j]*i);
                     }
                     ImagIn[i]=0;
                 }
            }



            /* regular*/
            fft_float (MAXSIZE,invfft);

            status_app=0;
            for (i=0;i<MAXSIZE;i++){
                for(i=0; i<MAXSIZE; i++)
                {
                	//printf("0x%08lX,",*((unsigned int*)&ImagOut[i]));
    		          if((*((unsigned int*)&RealOut[i]) != goldRealOut[i]) || (*((unsigned int*)&ImagOut[i]) != goldImagOut[i]))
                      {
                          if(status_app==0){
                              buffer[0] = 0xDD000000;

                          }else{
                              buffer[0] = 0xCC000000;
                          }

                          buffer[1] = *((uint32_t*)&i);
                          buffer[2] = *((uint32_t*)&RealOut[i]);
                          buffer[3] = *((uint32_t*)&ImagOut[i]); // u32, float has 32 bits

                          send_message(4);
                          status_app=1;
                      }

                }
            }


    }

    if(status_app==0){
        buffer[0] = 0xAA000000; //sem erros
        send_message(1);
    }
}
    return 0;



}