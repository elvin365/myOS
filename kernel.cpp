int kmain();
__declspec(naked) void startup()
{
	__asm {
		call kmain;
	}
}
/*Эта инструкция ДОЛЖНА быть первой , так как код компилится в бинарный и загрузчик передает управление по адресу первой инструкции*/
#pragma pack(push, 1) // Выравнивание членов структуры запрещено  
//Структура описывает данные об обработчике прерывания
struct idt_entry
{
	unsigned short base_lo;   // Младшие биты адреса обработчика   
	unsigned short segm_sel; // Селектор сегмента кода  
	unsigned char always0;     // Этот байт всегда 0  
	unsigned char flags;       // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE... 
	unsigned short base_hi;    // Старшие биты адреса обработчика
};
//Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
};
#pragma pack(pop)
struct idt_entry g_idt[256]; // Реальная таблица IDT 
struct idt_ptr g_idtp;   // Описатель таблицы для команды lidt 
#define GDT_CS          (0x8)
int i = 0;
#define IDT_TYPE_INTR (0x0E) 
#define VIDEO_BUF_PTR (0xb8000) 
#define PIC1_PORT (0x20) 
#define ENTERABLE_BUF_RAZ 40
char masskwithbuf[ENTERABLE_BUF_RAZ];
int otstupistolb = 47;
int choosedcolour;
int nomer_stroki = 0;

void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, void* hndlr)
{
	unsigned int hndlr_addr = (unsigned int)hndlr;
	g_idt[num].base_lo = (unsigned short)(hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short)(hndlr_addr >> 16);

} // для регистрации обработчика клавиатуры 


char scancode[] = {
	0,
	0, // escape
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+',
	14, // backspace
	'\t', // табуляция
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'_', // энтер
	0, // контрол
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '<', '>', '*',
	0, // левый шифт
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0, // правый шифт
	'*', // для умножения
	0, // альт
	' ', // пробел
	0, // капс
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F1 - F10
	0, // NUMLOCK
	0, // SCROLLLOCK
	0, // HOME
	0,
	0, // PAGE UP
	'-', // NUMPAD
	0, 0,
	0,
	'+', // NUMPAD
	0, // END
	0,
};

// ПУстой обработчик ПрЕрываний
__declspec(naked) void default_intr_handler() {
	__asm
	{
		pusha;

	}
	// ... (реализация обработки)  
	__asm {
		popa;
		iretd;
	}
}
/*Функция для инициализации системы прерываний , с её помощью массив заполняется адресами обработчиков*/
void intr_init()
{
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for (i = 0; i < idt_count; i++)
	{
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR, default_intr_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
	}
}

char search_for_skancode(unsigned char a)
{
	char finding;
	finding = scancode[a];
	return finding;
}


void print_on_screen(int color, const char* ptr, unsigned int strnum)
{
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	video_buf += 80 * 2 * strnum;
	while (*ptr)
	{
		video_buf[0] = (unsigned char)*ptr; // Символ (код) 
		video_buf[1] = color;
		video_buf += 2;// один символ, один атрибут 
		ptr++;
	}
}

void print_on_screen2(int color, const char* ptr, unsigned int strnum, int count)
{

	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	video_buf += 80 * 2 * strnum;
	while (*ptr) {
		if (count < 0)
		{
			video_buf[0] = (unsigned char)*ptr;
			video_buf[1] = color;
			video_buf += 2;
			ptr++;
		}
		count--;
		video_buf++;
	}

}
__inline unsigned char inb(unsigned short port) //чтение из порта
{
	unsigned char data;
	__asm {
		push dx;
		mov dx, port;
		in al, dx;
		mov data, al;
		pop dx;
	}
	return data;
}
void intr_enable()
{
	__asm
	sti;
}
void intr_disable()
{
	__asm
	cli;
}
/*int compareexpr(char* masskwithbuf)
{
	const char* expression = "expr ";
	int beg = 0;
	while (masskwithbuf[beg] != ' ')
	{
		if (masskwithbuf[beg] != expression[beg])
			return 0;
		beg++;
	}
	return 1;

}*/
/*int comparetooff(char* masskwithbuf)
{
	const char* shutdown = "shutdown";
	int beg = 0;
	return 1;
	else return 0;
}*/
/*int compareinfo(char* masskwithbuf)
{
	const char* information = "info";
	int beg = 0;
	while (masskwithbuf[beg] != '_')
	{
		if (masskwithbuf[beg] != information[beg])
			return 0;
		beg++;
	}
	return 1;
}*/
int protection(char* string)
{
	int i = 0;
	while (string[i]!='\0')
	{
		if ( (string[i] == '*' && string[i + 1] == '*' ) || (string[i] == '+' && string[i + 1] == '+') )
			return 1;
		else
			i++;
	}
	return 0;

}
int  backspace(char a)
{
	if (otstupistolb == 47)
		return 1;
	otstupistolb = otstupistolb - 2;// atribut
	print_on_screen2(choosedcolour, " ", nomer_stroki, otstupistolb);
	//otstupistolb = otstupistolb - 2;
	i--;
	return 1;
}
void vanish_the_screen()
{
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	for (int temp = 0; temp < 80 * 25; temp++)
	{
		video_buf[0] = '\0'; // Символ (код) 
		video_buf += 2;
	}
	nomer_stroki = 0;
}
void output(int answer, int b)
{
	const char* instruction = " instruction# ";
	const char* error = "Error";
	char str3[3];
	char str4[4];
	char str5[5];
	char str6[6];
	char str[2];
	char out[6];
	int c = 0;
	int k;

	int temp;
	int ou;
	temp = answer;
	while (temp != 0)
	{
		temp /= 10;
		c++;
	}
	k = c;
	while (c != 0)
	{
		ou = answer % 10;
		if (ou == 1)
			out[c] = '1';
		if (ou == 2)
			out[c] = '2';
		if (ou == 3)
			out[c] = '3';
		if (ou == 4)
			out[c] = '4';
		if (ou == 5)
			out[c] = '5';
		if (ou == 6)
			out[c] = '6';
		if (ou == 7)
			out[c] = '7';
		if (ou == 8)
			out[c] = '8';
		if (ou == 9)
			out[c] = '9';
		if (ou == 0)
			out[c] = '0';
		answer /= 10;
		c--;
	}
	switch (k)
	{
	case 1:
		str[0] = out[1];
		str[1] = 0;
		nomer_stroki++;
		print_on_screen(choosedcolour, str, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	case 2:
		str3[0] = out[1];
		str3[1] = out[2];
		str3[2] = 0;
		nomer_stroki++;
		print_on_screen(choosedcolour, str3, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	case 3:
		str4[0] = out[1];
		str4[1] = out[2];
		str4[2] = out[3];
		str4[3] = 0;
		nomer_stroki++;
		print_on_screen(choosedcolour, str4, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	case 4:
		str5[0] = out[1];
		str5[1] = out[2];
		str5[2] = out[3];
		str5[3] = out[4];
		str5[4] = 0;
		nomer_stroki++;
		print_on_screen(choosedcolour, str5, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	case 5:
		str6[0] = out[1];
		str6[1] = out[2];
		str6[2] = out[3];
		str6[3] = out[4];
		str6[4] = out[5];
		str6[5] = 0;
		nomer_stroki++;
		print_on_screen(choosedcolour, str6, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	default:
		nomer_stroki++;
		print_on_screen(choosedcolour, error, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
		break;
	}

}
void on_key(unsigned char a)
{
	if (nomer_stroki >= 25)
	{
		vanish_the_screen();
	}
	/*if (nomer_stroki >= 25)
	{
		unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
		for (int temp = 0; temp < 80 * 25; temp++)
		{
			video_buf[0] = '\0'; // Символ (код) 
			video_buf += 2;
		}
		nomer_stroki = 0;
	}*/
	char infoG[35] = "Bootloader parameters: green color";
	char infoB[34] = "Bootloader parameters: blue color";
	char infoR[33] = "Bootloader parameters: red color";
	char infoY[36] = "Bootloader parameters: yellow color";
	char infoGray[34] = "Bootloader parameters: gray color";
	char infoW[35] = "Bootloader parameters: white color";
	const char* instruction = " instruction# ";
	const char* info = "Calc OS: v.01. Developer: Gasanov Elvin, 23656/4, SpbPU, 2019";
	const char* info2 = "Compilers: bootloader: yasm, kernel: Microsoft C Compilier";
	if (a == 14)
	{
		int temp=backspace(a);
		if (temp == 1)
			return;
	}
	/*if (a == 14)
	{
		if (otstupistolb == 47)
			return;
		otstupistolb = otstupistolb - 2;
		print_on_screen2(choosedcolour, " ", nomer_stroki, otstupistolb);
		//otstupistolb = otstupistolb - 2;
		i--;
		return;
	}*/
	const char* error = "Unknown command";
	const char* division = "Division by 0";
	int b = 0;
	a = search_for_skancode(a);
	char str[] = { a, 0 };
	print_on_screen2(choosedcolour, str, nomer_stroki, otstupistolb);
	otstupistolb = otstupistolb + 2;
	masskwithbuf[i] = a;
	int cou = 0;
	i++;
	if (masskwithbuf[i - 1] == '_')
	{

		if ((masskwithbuf[0] == 's') && (masskwithbuf[1] == 'h') && (masskwithbuf[2] == 'u') && (masskwithbuf[3] == 't') && (masskwithbuf[4] == 'd') && (masskwithbuf[5] == 'o') && (masskwithbuf[6] == 'w') && (masskwithbuf[7] == 'n'))
		{
			__asm
			{
				mov dx, 0x0604;
				mov ax, 0x2000;
				out dx, ax;

			}
		}
		else
			cou++;
		if ((masskwithbuf[0] == 'i') && (masskwithbuf[1] == 'n') && (masskwithbuf[2] == 'f') && (masskwithbuf[3] == 'o'))
		{
			i = 0;
			nomer_stroki++;
			print_on_screen(choosedcolour, info, nomer_stroki);
			nomer_stroki++;
			print_on_screen(choosedcolour, info2, nomer_stroki);
			nomer_stroki++;
			if (choosedcolour == 0x02)
				print_on_screen(choosedcolour, infoG, nomer_stroki);
			if (choosedcolour == 0x02)
			{
				print_on_screen(choosedcolour, infoG, nomer_stroki);
			}
			else if (choosedcolour == 0x01)
			{
				print_on_screen(choosedcolour, infoB, nomer_stroki);
			}
			else if (choosedcolour == 0x04)
			{
				print_on_screen(choosedcolour, infoR, nomer_stroki);
			}
			else if (choosedcolour == 0x0E)
			{
				print_on_screen(choosedcolour, infoY, nomer_stroki);
			}
			else if (choosedcolour == 0x07)
			{
				print_on_screen(choosedcolour, infoGray, nomer_stroki);
			}
			else
			{
				print_on_screen(choosedcolour, infoW, nomer_stroki);
			}
			nomer_stroki++;
			print_on_screen(choosedcolour, instruction, nomer_stroki);
			for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
			{
				masskwithbuf[b] = 0;
			}
			otstupistolb = 47;


		}
		else
			cou++;
		if ((masskwithbuf[0] == 'e') && (masskwithbuf[1] == 'x') && (masskwithbuf[2] == 'p') && (masskwithbuf[3] == 'r') && (masskwithbuf[4] == ' '))
		{
			if (protection(masskwithbuf))
			{
				nomer_stroki++;
				print_on_screen(choosedcolour, "Expression is incorrect", nomer_stroki);
				for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
				{
					masskwithbuf[b] = 0;
				}
				return;
			}
			b = 5;
			int stack[6];
			int stack2[10];
			char stack3[10];
			for (int i = 0; i <= 5; i++)
			{
				stack[i] = 0;
			}
			for (int i = 0; i <= 9; i++)
			{
				stack2[i] = 0;
			}
			int i;
			int j = 1;
			int l;
			while (masskwithbuf[b] != ' ')
			{
				i = 1;
				while ((masskwithbuf[b] != '*') && (masskwithbuf[b] != '/') && (masskwithbuf[b] != '+') && (masskwithbuf[b] != '-') && (masskwithbuf[b] != ' '))
				{
					if (masskwithbuf[b] == '1')
						stack[i] = 1;
					if (masskwithbuf[b] == '2')
						stack[i] = 2;
					if (masskwithbuf[b] == '3')
						stack[i] = 3;
					if (masskwithbuf[b] == '4')
						stack[i] = 4;
					if (masskwithbuf[b] == '5')
						stack[i] = 5;
					if (masskwithbuf[b] == '6')
						stack[i] = 6;
					if (masskwithbuf[b] == '7')
						stack[i] = 7;
					if (masskwithbuf[b] == '8')
						stack[i] = 8;
					if (masskwithbuf[b] == '9')
						stack[i] = 9;
					if (masskwithbuf[b] == '0')
						stack[i] = 0;
					b++;
					i++;
				}
				switch (i)
				{
				case 2:
					stack2[j] = stack[1];
					break;
				case 3:
					stack2[j] = stack[1] * 10 + stack[2];
					break;
				case 4:
					stack2[j] = stack[1] * 100 + stack[2] * 10 + stack[3];
					break;
				case 5:
					stack2[j] = stack[1] * 1000 + stack[2] * 100 + stack[3] * 10 + stack[4];
					break;
				case 6:
					stack2[j] = stack[1] * 10000 + stack[2] * 1000 + stack[3] * 100 + stack[4] * 10 + stack[5];
					break;
				default:
					break;
				}
				if (masskwithbuf[b] == ' ')
					break;
				stack3[j] = masskwithbuf[b];
				b++;
				j++;
			}
			if (j == 1) {
				output(stack2[j], b);
				
			}
			for (i = 1; i <= j; i++)
			{
				if (j == 1)
					break;
				if (stack3[i] == '*')
				{
					stack2[i] = stack2[i] * stack2[i + 1];
					for (l = i + 1; l <= j; l++)
					{
						stack2[l] = stack2[l + 1];
						stack3[l - 1] = stack3[l];
					}
					j--;
					if (j == 1)
						output(stack2[j], b);
					if (i == 1)
						i--;
				}
				if (stack3[i] == '/')
				{
					if (stack2[i + 1] == 0)
					{
						nomer_stroki++;
						print_on_screen(choosedcolour, division, nomer_stroki);
						nomer_stroki++;
						print_on_screen(choosedcolour, instruction, nomer_stroki);
						for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
						{
							masskwithbuf[b] = 0;
						}
						otstupistolb = 47;
						i = 0;
						break;
					}
					stack2[i] = stack2[i] / stack2[i + 1];
					for (l = i + 1; l <= j; l++)
					{
						stack2[l] = stack2[l + 1];
						stack3[l - 1] = stack3[l];
					}
					j--;
					if (j == 1)
						output(stack2[j], b);
					if (i == 1)
						i--;
				}
			}
			for (i = 1; i <= j; i++)
			{
				if (j == 1)
					break;
				if (stack3[i] == '+')
				{
					stack2[i] = stack2[i] + stack2[i + 1];
					for (l = i + 1; l <= j; l++)
					{
						stack2[l] = stack2[l + 1];
						stack3[l - 1] = stack3[l];
					}
					j--;
					if (j == 1)
						output(stack2[j], b);
					if (i == 1)
						i--;
				}
				if (stack3[i] == '-')
				{
					stack2[i] = stack2[i] - stack2[i + 1];
					for (l = i + 1; l <= j; l++)
					{
						stack2[l] = stack2[l + 1];
						stack3[l - 1] = stack3[l];
					}
					j--;
					if (j == 1)
						output(stack2[j], b);
					if (i == 1)
						i--;
				}
			}
		}

		else
			cou++;
		if (cou == 3)
		{

			nomer_stroki++;
			print_on_screen(choosedcolour, error, nomer_stroki);
			nomer_stroki++;
			print_on_screen(choosedcolour, instruction, nomer_stroki);
			for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
			{
				masskwithbuf[b] = 0;
			}
			otstupistolb = 47;
			i = 0;
			cou = 0;
		}
	}
	if (i == ENTERABLE_BUF_RAZ)
	{


		nomer_stroki++;
		print_on_screen(choosedcolour, error, nomer_stroki);
		nomer_stroki++;
		print_on_screen(choosedcolour, instruction, nomer_stroki);
		for (b = 0; b < ENTERABLE_BUF_RAZ; b++)
		{
			masskwithbuf[b] = 0;
		}
		otstupistolb = 47;
		i = 0;
	}

}
void keyb_process_keys()
{
	// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует) 
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры 
		if (scan_code < 128)  // Скан-коды выше 128 - это отпускание клавиши   
			on_key(scan_code);
	}
}
__inline void outb(unsigned short port, unsigned char data)//запись
{
	__asm {
		push dx;
		mov dx, port;
		mov al, data;
		out dx, al;
		pop dx;
	}
}
__declspec(naked) void keyb_handler()
{
	//intr_enable();
	intr_disable(); //для корректной обработки поступивших данных

	__asm pusha;
	// Обработка поступивших данных 
	keyb_process_keys();

	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано.Нужно так делать,иначе контроллер
	// не будет посылать новые сигналы о прерываниях
	outb(PIC1_PORT, 0x20);

	__asm {
		popa;
		iretd;
	}
}
void keyb_init()
{
	// Регистрация обработчика прерывания  
	intr_reg_handler(0x09, GDT_CS, 0x8E, keyb_handler);
	// segm_sel=0x8, P=1, DPL=0, Type=Intr   
	// Разрешение только прерываний клавиатуры от контроллера 8259  	
	outb(PIC1_PORT + 1, 0xFF ^ 0x02);
	// 0xFF - все прерываний, 0x02 - только IRQ1 (клавиатура) , то есть прерывания чьи биты установлены в 0
}
void intr_start() // регистрация таблицы прерываниий
{
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int)(&g_idt[0]);
	g_idtp.limit = (sizeof(struct idt_entry) * idt_count) - 1;

	__asm {
		lidt g_idtp;
	}
}
int kmain()
{
	const char* hello = "Welcome to CalcOS!";

	unsigned char col = '0';
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	col = *video_buf;
	__asm
	{
		mov al, [edi];
		mov col, al;
	}



	switch (col)
	{
	case '1':
			choosedcolour = 0x02; // green
			break;
	case '2':
		choosedcolour = 0x01;
		break;
	case '3':
		choosedcolour = 0x04;
		break;
	case '4':
		choosedcolour = 0x0E;
		break;
	case '5':
		choosedcolour = 0x07;
		break;
	case '6':
		choosedcolour = 0x0F;
		break;
	default:
		break;
	}


	print_on_screen(choosedcolour, hello, 0);
	intr_init(); //инициализация подсистемы прерываний
	keyb_init(); //регестрирует обработчик прерываний клавиатуры
	intr_start(); //регестрация таблицы дескрипторов прерываний
	intr_enable(); //включение прерываний
	while (1)
	{
		__asm hlt; //приостановки процессора выполняется до тех пор ,пока не возникнет аппаратное прерывание
	}
	return 0;
}
