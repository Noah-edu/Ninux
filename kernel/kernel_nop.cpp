#include<stdint.h>
#include<string>
print_string(const char*, unsigned char);
#ifndef __linux__
print_string("Must be compiled on linux!",RED);
#endif

typedef struct SMAP_entry
{
	uint32_t BaseL;
	uint32_t BaseH;
	uint32_t LengthL;
	uint32_t LengthH;
	uint32_t Type;
	uint32_t ACPI;
}__attribute__((packed)) SMAP_entry_t;

int __attribute__((noinline)) __attribute__((regparm(3))) detectMemory(SMAP_entry_t* buffer, int maxentries)
{
	uint32_t contID = 0;
	int entries = 0, signature, bytes;
	do
	{
		__asm__ __volatile__ ("int $0x15"
				     : "=a"(signature), "=c"(bytes), "=b"(contID)
				     : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(buffer));
		if (signature != 0x534D4150)
			return -1;
		if (!(bytes > 20 && (buffer->ACPI & 0x0001) == 0))
		{
			buffer++;
			entries++;
		}
	}while(contID != && entries < maxentries);
	return entries;
}

//Colours
#define BLACK 0
#define GREEN 2
#define RED 4
#define YELLOW 14
#define WHITE_COLOUR 15






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

/*void get_Input(char* str, int length)
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
*/
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
	
	SMAP_entry_t* smap = (SMAP_entry_t*) 0x1000;
	const int smap_size = 0x2000;
	
	int entry_count = detectMemory(smap,smap_size / sizeof(SMAP_entry_t))
	
		if(entry_count == -1)
		{
			print_string("ERROR",RED);
		}

	terminal_buffer = (unsigned short*)VGA_ADDRESS;
	vga_index = 0;
	clear_terminal();
	print_string("Boot successful", YELLOW);

	
	vga_index = 80;
	
	print_string(std::to_string(entry_count),WHITE);

	while(true)
	{

	}

	return 0;
}
