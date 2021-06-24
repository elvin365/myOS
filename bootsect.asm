[BITS 16]
[ORG 0x7c00]
start: ; Инициализация адресов сегментов. Эти операции требуется не для любого BIOS, но их рекомендуется проводить.
mov ax, cs ; Сохранение адреса сегмента кода в ax
mov ds, ax ; Сохранение этого адреса как начало сегмента данных
mov ss, ax ; И сегмента стека
mov sp, start ; Сохранение адреса стека как адрес первой инструкции этого кода. Стек будет расти вверх и не перекроет код.


; переход в видео режим
mov ah, 0x00
mov al, 0x03
int 0x10


mov ah, 0x0e ; В ah номер функции BIOS: 0x0e - вывод символа наактивную видео страницу (эмуляция телетайпа)
mov al, 'P' ; В al помещается код символа
int 0x10 ; Вызывается прерывание. Обработчиком является код BIOS.Символ будет выведен на экран.
mov al, 'r'
int 0x10
mov al, 'e'
int 0x10
mov al, 's'
int 0x10
int 0x10
mov al,':'
int 0x10
mov al,' '
int 0x10
mov al,'1'
int 0x10
mov al,'-'
int 0x10
mov al,'g'
int 0x10
mov al,'r'
int 0x10
mov al,'e'
int 0x10
int 0x10
mov al,'n'
int 0x10
mov al,','
int 0x10
mov al,'2'
int 0x10
mov al,'-'
int 0x10
mov al,'b'
int 0x10
mov al,'l'
int 0x10
mov al,'u'
int 0x10
mov al,'e'
int 0x10
mov al,','
int 0x10
mov al,'3'
int 0x10
mov al,'-'
int 0x10
mov al,'r'
int 0x10
mov al,'e'
int 0x10
mov al,'d'
int 0x10
mov al,','
int 0x10
mov al,'4'
int 0x10
mov al,'-'
int 0x10
mov al,'y'
int 0x10
mov al,'e'
int 0x10
mov al,'l'
int 0x10
int 0x10
mov al,'o'
int 0x10
mov al,'w'
int 0x10
mov al,','
int 0x10
mov al,'5'
int 0x10
mov al,'-'
int 0x10
mov al,'g'
int 0x10
mov al,'r'
int 0x10
mov al,'a'
int 0x10
mov al,'y'
int 0x10
mov al,','
int 0x10
mov al,'6'
int 0x10
mov al,'-'
int 0x10
mov al,'w'
int 0x10
mov al,'h'
int 0x10
mov al,'i'
int 0x10
mov al,'t'
int 0x10
mov al,'e'
int 0x10
mov al, 0x0a   ; перевод строки    
int 0x10


; загрузка кода
mov ax,0x1100
mov es,ax
mov bx,0x00
mov dl,1  
mov dh,0  
mov ch,0 
mov cl,2   
mov al,13 
mov ah,0x02
int 0x13
;загрузка данных
mov ax,0x1300
mov es,ax
mov bx,0x00
mov dl,1; N диска b  
mov dh,0 ;    
mov cl,15 ;
mov ch,0  
mov al,7
mov ah,0x02
int 0x13

colour:
mov ah,0x00 
int 0x16
cmp al,0x31
jae next1
jmp colour


next1:
cmp al,0x36
jle next2
jmp colour

next2:
;//mov ah, 0x0e
;//mov al,'Q'
;//int 0x10
mov [edi],al ;сохранение в произвольном месте
mov ah, 0x00
mov al, 0x03
int 0x10
jmp po
;переход в защищенный режим
po:
;отключение прерываний
cli 
lgdt [gdt_info] ;загрузка размера и адреса таблицы дескрипторов 
;Включение адресной линии А20
in al, 0x92 
or al, 2 
out 0x92, al 
;СТавим бит PE регистра СR0 и тоогда процессов в защищенном режиме
mov eax, cr0 
or al, 1 
mov cr0, eax 
jmp 0x8:protected_mode; дальний переход, чтобы зашрузить инфу в cs 

;таблица дескрипторов
gdt:
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00 
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00 
gdt_info:  
dw gdt_info - gdt
dw gdt, 0 





  ; первые инструкции в защищенном режиме

[BITS 32] 
protected_mode: 
mov ax, 0x10 
mov es, ax 
mov ds, ax 
mov ss, ax 
call 0x11000 ;передача управления ядру



times (512 - ($ - start) - 2) db 0 
db 0x55, 0xAA 