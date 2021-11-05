#include <MKL25Z4.H>
#include <arm_math.h>
#include "LCD_4bit.h"

//Sampling variables
volatile int CaptureFlag = 0;
volatile int CurrentSample = 0;

//Audio Buffers
volatile int16_t AudioBuffer0[128];
volatile int16_t AudioBuffer1[128];

//Pointers to Audio Buffers
volatile int16_t *ActiveBuffer;
volatile int16_t *BackBuffer;
volatile int CurrentBuffer = 0;

//DSP Buffer
volatile q15_t AudioFFT[256];
volatile q15_t AudioFFT_Mag[17];

/* create a delay function */
void Delay(volatile unsigned int time_del) {
														while (time_del--) {}
}

void Init_ADC() {
	
			SIM->SCGC6 |= (1UL << SIM_SCGC6_ADC0_SHIFT) ;
	
			/* Set the ADC0_CFG1*/
			
			ADC0->CFG1 = 0x54;
			ADC0->CFG2 = 0x4;
			
			/* Set the ADC0_SC2 register to 0*/
			
			ADC0->SC2 = 0;
}

//use systick interrupt to sample as quickly as possible
void SysTick_Handler(void){
	if(CaptureFlag){
		//record a sample and increment the sample counter
		BackBuffer[CurrentSample] = (uint16_t)ADC0_RA;
		CurrentSample++;
		
		//enable read from the ADC
		if(CurrentSample<128)
		{
			ADC0->SC1[0] = 8 ;
			while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
		}
		//if sampling is completed set CaptureFlag to 0 and reset sample counter
		else
		{
			CurrentSample = 0;
			CaptureFlag = 0;
		}
	}
}

//sample at 8khz
void InitSysTick()
{
	SysTick_Config(SystemCoreClock / 6000); 
}
/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
volatile unsigned res;


int main (void) {
	int i;
	int temp;
	
	//create variable for fft instance
	arm_rfft_instance_q15	RealFFT_Instance;

	//initialize ADC, LCD, and systick interrupt
	Init_LCD();
	Clear_LCD();
	
	Init_ADC();
	InitSysTick();
	
	//reset buffers and set pointers
	CurrentBuffer = 0;
	ActiveBuffer = AudioBuffer0;
	BackBuffer = AudioBuffer1;
	
	//initialize the FFT module
	arm_rfft_init_q15(&RealFFT_Instance,128,0,1);
	
	//reset sample counter and set captue to active
	CurrentSample = 0;
	
	CaptureFlag = 1;
	
	
	while(1){
		//perform an FFT on the data in ActiveBuffer and store it in the AudioFFT array
		arm_rfft_q15(&RealFFT_Instance,(q15_t*)ActiveBuffer,(q15_t*)AudioFFT);
		
		//scale the input for magnitude	calc
		for(i=0; i<256; i++){
			AudioFFT[i]<<=6;
		}

		//compute magnitudes of FFT and store in AudioFFT_Mag array which is 1 bin larger than the resolution of our lcd		
		arm_cmplx_mag_q15((q15_t*)AudioFFT,(q15_t*)AudioFFT_Mag,17);
		
		//print out a character to the lcd based on the magnitude of the frequencies in the magnitude array
		for(i = 0; i < 17; i++){
				//scale the result by bitshifting the result should not be between 0 and 16
				temp = (AudioFFT_Mag[i+1]>>8);
							
				//clear any residual characters on the top row
				Set_Cursor(i,0);
				Delay(100);
			  lcd_putchar(0x10);
				Delay(100);
			
			
			//print 1 bar even if the magnitude is 0
			if(temp <= 0){
				Set_Cursor(i,1);
				Delay(100);
				lcd_putchar(0);
				Delay(100);
			//if the scaled magnitude is less than 8 print a corresponding number of bars on the bottom
			}else if(temp > 0 && temp < 8){
				Set_Cursor(i,1);
				Delay(100);
				lcd_putchar(temp);
				Delay(100);
			//if the magnitude is greater than 8 print a full bar on the bottom, move the cursor, and print the remaining bars on top
			}else if(temp >= 8){
				Set_Cursor(i,1);
				Delay(50);
				lcd_putchar(7);
				Delay(50);
				Set_Cursor(i,0);
				Delay(50);
				lcd_putchar(temp-7);
				Delay(50);
				}
				Delay(1000);
			
				
			}

		//wait until capturing is completed
		while(CaptureFlag == 1){}
		
		//switch buffers
		if(CurrentBuffer == 0)
		{
			CurrentBuffer = 1;
			ActiveBuffer = AudioBuffer1;
			BackBuffer = AudioBuffer0;
		}
		else
		{
			CurrentBuffer = 0;
			ActiveBuffer = AudioBuffer0;
			BackBuffer = AudioBuffer1;
		}
		CaptureFlag = 1;
	}
}
