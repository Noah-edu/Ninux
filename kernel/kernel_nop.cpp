//#include<ctime>
//#include"os_init.h"
//#include"run_cmd.h"
//#include"run_cmd.cpp"

void print_string(const char*,unsigned char);

#ifndef __linux__
print_string("Must be compiled on linux!",RED);
#endif

//Colours
#define BLACK 0
#define GREEN 2
#define RED 4
#define YELLOW 14
#define WHITE_COLOUR 15





void count_memory(void)
{
	register unsigned long *mem;

	unsigned long mem_count, a;
	unsigned short memkb;
	unsigned char irq1, irq2;
	unsigned long cr0;

	//Save IRQ's
	irq1 = inb(0x21);
	irq2 = inb(0xA1);

	// KILL all irq's
	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);


	mem_count = 0;
	memkb = 0;
// store a copy of cr0 variable
	__asm__ __volatile("movl %%cr0, %%eax" : "=a"(cr0)::"eax");


// write-back and invalidate the cache
	__asm__ __volatile__ ("wbindvd");

// plug cr0 with just PE/CD/NW
// cache disable(486+), no writeback(486+), 32bit mode(386+)
__asm__ __volatile__("movl %%eax, %%cr0", :: "a" (cr0 | 0x00000001 | 0x40000000 | 0x20000000 ) : "eax");

	do 
	{
	memkb++;
	mem_count += 1024*1024;
	mem = (unsigned long*) mem_count;

	a = *mem;
	*mem = 0x55AA55AA;
	// The empty asm calls tell g++ not to rely on what's in its register
	// As saved variables (this avoids g++ optimisations)
	asm("":::"memory");


	if(*mem != 0x55AA55AA) mem_count = 0;
	else
	{ 
		
		*mem = 0xAA55AA55;
		asm("":::"memory");
		if(*mem!=0xAA55AA55)
		mem_count = 0;
	}
	asm("":::"memory");
	*mem = a;

	}while(memkb < 4096 && mem_count != 0);
	__asm__ __volatile__("movl %%eax, %%cr0", :: "a" (cr0) : "eax");

	mem_end = memkb<<20;
	mem = (unsigned long*) 0x413;
	bse_end = (*mem & oxFFFF) << 6;

	outb(0x21, irq1);
	outb(0xA1, irq2);

}

void * laihost_map(size_t addr,size_t count)
{
return (void*)(addr + count);
}

//new line
#define newln vga_index += 80

//VGA (video) address
#define VGA_ADDRESS 0xB8000


unsigned short* terminal_buffer;
unsigned int vga_index;


//Replaces all characters on the screen with the ascii empty charcter (25 line support)
void clear_terminal(void)
{
for(int i = 0; i < 25*80*2;)
{
	terminal_buffer[i] = ' ';
	i += 2;
}


}

void get_Input(char* str, int length)
{

for(int i = 0;i < length;)
{
__asm__
(
"char:"
"db %0;"
"mov ah,0;"
"int 0x16"
"mov [char],al;":"=a"(str[i])
);
i++;
}

}
//Prints a string with a colour
void print_string(const char* str, unsigned char colour)
{
	
	for (int i = 0; str[i];) 
	{
		switch(*str)
		{
			case '\n':
		newln;
		break;
			default:
		terminal_buffer[vga_index] = (unsigned short)str[i] | (unsigned short)colour << 8;
		i++;
		vga_index++;
		break;
		}
	}
}

//Main (referenced in boot.asm)
int main(void)
{
	
	//VGA_INDEX = 0 (Normal)
	//VGA_INDEX = 80 (NEW LINE)
	

	laihost_map(ptr,1);

	terminal_buffer = (unsigned short*)VGA_ADDRESS;
	vga_index = 0;
	clear_terminal();
	print_string("Boot successful", YELLOW);

	newln;


	print_string(vga_index.c_str(),WHITE);

	while(true)
	{
//waitingCommand();
	}

	return 0;
}
