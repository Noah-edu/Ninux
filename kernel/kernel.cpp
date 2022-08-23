#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



//#ifdef __linux__
//#error "Must be compiled with a cross-compiler"
//#endif



//#ifndef(__i386__)
//#error "Outdated"
//#endif

#define UCHAR unsigned char


void outportb(uint16_t port, uint8_t value){
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inportb(uint16_t port)
{
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
  return ret;
}



enum vga_colour 
{
	VGA_COLOUR_BLACK = 0,
	VGA_COLOUR_BLUE = 1,
	VGA_COLOUR_GREEN = 2,
	VGA_COLOUR_CYAN = 3,
	VGA_COLOUR_RED = 4,
	VGA_COLOUR_MAGENTA = 5,
	VGA_COLOUR_BROWN = 6,
	//More will be added later
	VGA_COLOUR_WHITE = 15,
};


// memory allocation
void malloc(int x,unsigned int bytes)
{

}

static inline uint16_t vga_entry_colour(enum vga_colour fg, enum vga_colour bg){
	return fg | bg << 4;
}


static inline uint16_t vga_entry(UCHAR uc, uint8_t colour){
	return (uint16_t) uc | (uint16_t) colour << 8;
}


size_t strlen(const char* str)
{
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}

size_t strlen(char* str)
{
	size_t len = 0;
	while(str[len])
		len++;
	return len;

}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_colour;
uint16_t* terminal_buffer;


void terminal_init(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_colour = vga_entry_colour(VGA_COLOUR_WHITE, VGA_COLOUR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for(size_t y = 0; y < VGA_HEIGHT; y++){
		for(size_t x = 0; x < VGA_WIDTH; x++)	{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_colour);
		}
	}

}

void terminal_setcolour(uint8_t colour)
{
	terminal_colour = colour;
}


//terminal writes

void terminal_putentryat(char c, uint8_t colour, size_t x, size_t y)\
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, colour);
}


void terminal_putchar(char c)
{
	switch(c)
	{
	case '\n':
		terminal_row++;
		terminal_column = 0;

		break;

	case '\t':
		terminal_column += 5;
		break;
	default:
		terminal_putentryat(c, terminal_colour, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) 
		{
		terminal_column = 0;
			if(++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
}

void terminal_putchar(unsigned char c)
{
	switch(c)
	{
		case '\n':
			terminal_row++;
			terminal_column = 0;
			break;
		case '\t':
			terminal_column += 5;
			break;
		defualt:
			terminal_putentryat(c, terminal_colour, terminal_column, terminal_row);
			if(++terminal_column == VGA_WIDTH)
			{
				terminal_column = 0;
					if(++terminal_row == VGA_HEIGHT)
						terminal_row = 0;
			}
	}
}

void terminal_write(const char* data, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		 terminal_putchar(data[i]);	
	}
}

void terminal_write(char* data, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		terminal_putchar(data[i]);
	}
}

void terminal_writestring(const char* data){
	terminal_write(data, strlen(data));
}

void terminal_writestring(char* data){
	terminal_write(data, strlen(data));
}
/*
===============*
KEYBOARD DRIVER|
===============*
*/
enum KYBRD_ENCODER_IO
{
	KYBRD_ENC_INPUT_BUF = 0x60,
	KYBRD_END_CMD_REG = 0x60
};

enum KYBRD_CTRL_IO
{
	KYBRD_CTRL_STATS_REG = 0x64,
	KYBRD_CTRL_CMD_REG = 0x64
};

enum KYBRD_CTRL_STATS_MASK{

	KYBRD_CTRL_STATS_MASK_OUT_BUF = 1,
	KYBRD_CTRL_STATS_MASK_IN_BUF = 2,
	KYBRD_CTRL_STATS_MASK_SYSTEM = 4,
	KYBRD_CTRL_STATS_MASK_CMD_DATA = 8,
	KYBRD_CTRL_STATS_MASK_LOCKED  = 0x10,
        KYBRD_CTRL_STATS_MASK_AUX_BUF = 0x20,
	KYBRD_CTRL_STATS_MASK_TIMEOUT = 0x40,
	KYBRD_CTRL_STATS_MASK_PARITY = 0x80	

};

uint8_t kybrd_ctrl_read_status(){
	return inportb(KYBRD_CTRL_STATS_REG);
}

void kybrd_ctrl_send_cmd(uint8_t cmd)
{
    // wait for keyboard controller input to be clear
    while(1)
    {
	 if((kybrd_ctrl_read_status() && KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
		 break;
    }
    outportb(KYBRD_CTRL_CMD_REG, cmd);
}
// read keyboard encoder buffer
uint8_t kybrd_end_read_buf(){
	return inportb(KYBRD_ENC_INPUT_BUF);
}

void kybrd_enc_send_cmd(uint8_t cmd)
{
     while(1)
     {
	  if((kybrd_ctrl_read_status() && KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
		  break;
     }
     outportb(KYBRD_END_CMD_REG, cmd);
}

//Testing whether it is operational

bool kybrd_self_test()
{
   kybrd_ctrl_send_cmd(0xAA);
   
   while(1)
   {
	   
   	if(kybrd_ctrl_read_status() && KYBRD_CTRL_STATS_MASK_OUT_BUF)
		break;
   }

  
   outportb(0,0x60);
   uint8_t outputbuf = inportb(0x60);
	
   if( outputbuf == 0x55)
   {
   return true;
   }
   return false;
}

//END OF KEYBOARD DRIVER


//uint8_t to string
char* uint8_toString(uint8_t value, char* str, int base )
{
	char* rc;
	char* ptr;
	char* low;

	if( base < 2 || base > 36 )
	{
		*str = '\0';
		return str;
	}
	rc = ptr = str;

	if( value < 0 && base == 10){
		*ptr++ = '-';
	}

	low = ptr;

	do
	{
	
		*ptr++ = "zyxwvutrqponmkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
		value /= base;
	}  while( value ) ;

	*ptr-- = '\0';

	while( low < ptr  )
	{
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

}

 //memory counter
uint8_t CMOS()
{	
   uint8_t total;
   unsigned char lowmem,highmem;
   
   outportb(0x70, 0x30);
   lowmem = inportb(0x71);
   outportb(0x70, 0x31);
   highmem = inportb(0x71);

   total = lowmem | highmem << 8;
   return total;
}

int main(void)
{
	char* buffer;
  terminal_init();
  terminal_setcolour(VGA_COLOUR_GREEN);
  
  uint8_toString(CMOS(),buffer,10);

  terminal_writestring("Boot succesful\n\n");
  terminal_writestring("\nTotal mem:\t0x");
  
  terminal_writestring(buffer);

  terminal_writestring("\n\n");
  
 /*if(kybrd_self_test() == 1)
  {
  terminal_writestring("true! :3");
  }
*/
  //RGB showcase
  terminal_putentryat(219,VGA_COLOUR_RED,75,20);
  terminal_putentryat(219,VGA_COLOUR_GREEN,76,20);
  terminal_putentryat(219,VGA_COLOUR_BLUE,77,20);
  
 
 
//terminal_writestring(x);

}



