//--------------------Serial_library for AVR----------------------//
//---------------bye Benachour Sohaib (dz inventors)--------------//
//----------------------26.12/2017--------------------------------//
#ifndef Serialdefine
#define Serialdefine

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <avr/pgmspace.h>


#ifndef SERIAL_BAUD
# warning "Serial BAUD not defined for Serial comunication default value will be 1000000"
#define SERIAL_BAUD 1000000UL
#endif

#ifndef SERIAL_DOUBLE_SPEED
#define SERIAL_DOUBLE_SPEED 1
#endif
 
#if ((SERIAL_DOUBLE_SPEED > 2) || (SERIAL_DOUBLE_SPEED < 1))
# warning "Serial double speed value can be (1 or 2) change it or it will set automatically to 1"
#define SERIAL_DOUBLE_SPEED 1
#endif 

//#ifndef SERIAL_BUFFER_SIZE
//# warning "Serial Buffer size not defined for Serial communication default value will be 50"
//#define SERIAL_BUFFER_SIZE 20
//#endif

#define SERIAL_BAUDC  ((F_CPU/(16/SERIAL_DOUBLE_SPEED)/SERIAL_BAUD)-1)

inline void Serial_Setup()__attribute__((always_inline));//setup serial communication
       void Serial_PrintStr(const char * data);//data string transmission
inline void Serial_PrintBytes(const uint8_t * data,uint16_t length);//data string transmission
inline void Serial_PrintIn(const int data,const uint8_t base)__attribute__((always_inline));//data int transmission
inline void Serial_PrintFl(const double data,uint8_t accuracy)__attribute__((always_inline));//data float transmission
inline void Serial_WaitBufferStr(char stop)__attribute__((always_inline));//freeze until string data wrote
inline char* Serial_ReadBufferStr()__attribute__((always_inline));//read data of serial all buffer
inline char * Serial_ReadBufferDataStr()__attribute__((always_inline));//read data of serial buffer data
inline uint8_t * Serial_ReadBufferBytes()__attribute__((always_inline));//read bytes of serial buffer
inline uint8_t * Serial_ReadBufferDataBytes()__attribute__((always_inline));//read bytes of serial buffer data
inline void Serial_ClearBuffer()__attribute__((always_inline));//clear data serial buffer
inline void Serial_WaitBufferBytes(uint16_t length)__attribute__((always_inline));//freeze until new data get in the buffer with  knowing of data length
inline void Serial_WaitBufferBytes()__attribute__((always_inline));//freeze until new data get in buffer withe sending 2 bytes of size in the first data
inline bool Serial_BufferFlagBytes(uint16_t length)__attribute__((always_inline));//this function return 1 if the buffer is wrote to the length input(give buffer size and know if it wrote without freeze)
inline bool Serial_BufferFlagBytes()__attribute__((always_inline));//this function return 1 if the buffer is wrote to the length you send from 2 first bytes form total data (send 2 bytes buffer size and know if it wrote without freeze)
inline bool Serial_BufferFlagStr()__attribute__((always_inline));//this function return 1 if the buffer is wrote and the last index is 0 (not freeze fucntion)
inline void Serial_EnableRx()__attribute__((always_inline));//enable RX interrupt
inline void Serial_DisableRx()__attribute__((always_inline));//disable RX interrupt

volatile char* Serial_buffer;//add one in size buffer for incoming data size
volatile uint16_t Serial_index_buffer=0;//buffer counter (volatile for using both in interrupt and out)

void Serial_Setup()
{
    UCSR0A |= 1<<U2X0;//double speed
	UBRR0H = SERIAL_BAUDC>>8; //Set the BAUD to BAUD Register (high Register)
	UBRR0L = SERIAL_BAUDC;//Set the BAUD to BAUD Register (low Register)
	UCSR0B |=(1<<TXEN0)|(1<<RXEN0); //Enable TX and RX Pins
	//UCSR0B |=1<<TXCIE0; //Enable TX transmission complete interrupt to clear complete transmission flag
	UCSR0C |=(1<<UCSZ00)|(1<<UCSZ01); //Set the USART to 8bit mode
	UCSR0B|=1<<RXCIE0; //Enable RX interrupt
	sei(); //set the I bit (global interrupt)

	char buffer[0x7D0];//max size is 2kbytes (SRAM=2kbytes)
	Serial_buffer=buffer;//write the address to the global Serial_buffer pointer
}

void Serial_EnableRx()
{
	UCSR0B|=1<<RXCIE0; //Enable RX interrupt
}

void Serial_DisableRx()
{
	UCSR0B&=~(1<<RXCIE0); //disable RX interrupt
}

void Serial_PrintBytes(const uint8_t * data,uint16_t length)
{
   for (uint16_t i=0;i<length;i++)
   {
    while (!(UCSR0A&(1<<UDRE0))){}//wait transmission buffer to be empty
   	UDR0 = *data; //transmit data
   	//while((UCSR0A&(1<<TXC0))==0){} //wait the transmission to complete
   	//UCSR0A|=1<<TXC0; //Clear the transmission complete flag
   	data++;
   }
}

void Serial_PrintStr(const char * data)
{
	while(*data!=0)
	{
	    while (!(UCSR0A&(1<<UDRE0))){}//wait transmission buffer to be empty
		UDR0 = *data; //transmit data
		//while((UCSR0A&(1<<TXC0))==0){} //wait the transmission to complete
		//UCSR0A|=1<<TXC0; //Clear the transmission complete flag
		data++;
	}
}

void Serial_PrintIn(const int data,const uint8_t base=10)
{
	char array[7];//6 bytes of size for holding 5 digits and sign from int variable last byte for holding the end of the array (0)
	itoa(data,array,base);//convert data to array with giving base
	Serial_PrintStr(array);
}

void Serial_PrintFl(double data,uint8_t accuracy=2)
{
	const int PartOne=(const int)data;//get the first part of double number
	double PartTwo=data-(double)PartOne;//get the second part of double number
	Serial_PrintIn((const int)data,10);//print first part
	Serial_PrintStr(".");//print the comma
	while (accuracy-->0)
	{
		PartTwo*=10.0;//get next digit after comma
		const char digit_buffer=(const char)PartTwo;//convert the digit to int
		const char digit=(const char) digit_buffer;//convert the digit to char (do it in this way to optimize size)
		Serial_PrintStr(&digit);//print digit
		PartTwo-=(double)digit_buffer;//delete digit
	}	
}

char * Serial_ReadBufferStr()
{
	return (char*)Serial_buffer;//return the address of Serial buffer without buffer size
}

char * Serial_ReadBufferDataStr()
{
	return (char*)Serial_buffer+2;//return the address of Serial buffer without buffer size
}

uint8_t * Serial_ReadBufferBytes()
{
	return (uint8_t *)Serial_buffer;//return the address of Serial buffer without buffer size
}

uint8_t * Serial_ReadBufferDataBytes()
{
  return (uint8_t *)Serial_buffer+2;//return the address of Serial buffer without buffer size
}

void Serial_ClearBuffer()
{
	Serial_index_buffer=0;//reset Serial buffer
}

void Serial_WaitBufferBytes(uint16_t length)
{
	while(Serial_index_buffer<length){}//wait until data wrote to buffer
	Serial_index_buffer=0;//clear index of the buffer
}

void Serial_WaitBufferBytes()
{
	uint16_t Size=0;
	while (Size==0)
	{
		while(Serial_index_buffer<2){}//wait until data size wrote to the fist 2 bytes
		Size=(uint16_t)Serial_buffer[0]<<8|(uint16_t)Serial_buffer[1];//convert 2bytes of data to number to define the size of the really data
		Serial_WaitBufferBytes(Size+2);//wait the really data
	}
}

void Serial_WaitBufferStr(char stop=0)
{	
	while(Serial_buffer[Serial_index_buffer-1]!=stop||Serial_index_buffer==0){}//wait until data wrote to buffer
	Serial_buffer[Serial_index_buffer]=0;
	Serial_index_buffer=0;//clear index of the buffer
}

bool Serial_BufferFlagStr()
{
	if (Serial_buffer[Serial_index_buffer-1]==0)//if buffer have 0 in the last index
	{
		Serial_index_buffer=0;//clear index of the buffer
		return 1;//return true
	}
	return 0;//return false
}

bool Serial_BufferFlagBytes(uint16_t length)
{
	if (Serial_index_buffer>=length)//if buffer is overflow and buffer size send is not 0
	{
		Serial_index_buffer=0;//clear index of the buffer
		return 1;//return true
	}
	return 0;//return false
}

uint16_t Serial_BufferFlagSize=0;//define the buffer size for on freeze function

bool Serial_BufferFlagBytes()
{
	if(Serial_index_buffer>=2)//keep checking if buffer size not set yet
	{
		Serial_BufferFlagSize=(uint16_t)Serial_buffer[0]<<8|(uint16_t)Serial_buffer[1];//get the first 2 bytes data from total data and convert it to number
		if((Serial_BufferFlagBytes(Serial_BufferFlagSize+2)==1)&&(Serial_BufferFlagSize!=0))//check if the really data is wrote
		{
			Serial_BufferFlagSize=0;//reset buffer size
			return 1;//return true
		}
	}
	return 0;//return false
}

ISR(USART_RX_vect)//interrupt for RX end receiving
{
	Serial_buffer[Serial_index_buffer]=UDR0;//save data to buffer
	Serial_index_buffer++;//next buffer index
}

#endif