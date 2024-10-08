/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Advanced
Automatic Program Generator
� Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Leitura de bot�o e apresenta��o de informa��es no LCD
Version : 1.0
Date    : 02/09/2024
Author  : Henrique Andrade Pancotti, Matheus Luiz Silva Felix, Vinicius Franklin
Company : UFJF
Comments: 


Chip type               : ATmega16
Program type            : Application
AVR Core Clock frequency: 14,745600 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/

#include <mega16.h>

// Alphanumeric LCD Module functions
#include <alcd.h>

// Standard Input/Output functions 
#include <string.h>
#include <stdio.h>
#include <delay.h>
#include <stdlib.h>

// PINA0..3 will be row inputs
#define KEYIN PINA
// PORTA4..7 will be column outputs
#define KEYOUT PORTA
#define FIRST_COLUMN 0x80
#define LAST_COLUMN 0x10

typedef unsigned char byte;
// store here every key state as a bit,
// bit 0 will be KEY0, bit 1 KEY1,...
unsigned int keys;

int andar_elevador;
int andar_atual;
int andares_acionados[3]; // Adaptar para 3 andares
int fila_de_andares[3] = {0,1,0};
int controle_delay_parado = 0;
int proximo_andar = -1;

typedef struct {
    int distancia;
    int andar;
} DistanciaAteAndar;

void swap(DistanciaAteAndar *a, DistanciaAteAndar *b) {
    DistanciaAteAndar temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

int partition(DistanciaAteAndar arr[], int low, int high) {
    int pivot = arr[high].distancia; // Escolhe o valor do �ltimo elemento como piv�
    int i = (low - 1); // �ndice do menor elemento 
    int j;

    for (j = low; j < high; j++) {
        if (arr[j].distancia < pivot) {
            i++;
            swap(&arr[i], &arr[j]); // Troca
        }
    }
    swap(&arr[i + 1], &arr[high]); // Coloca o piv� na posi��o correta
    return (i + 1);
}

void quickSort(DistanciaAteAndar arr[], int low, int high) {
    if (low < high) {
        // Particiona o array e obt�m o �ndice do piv�
        int pi = partition(arr, low, high);

        // Ordena recursivamente os elementos antes e depois da parti��o
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void qsort_custom(DistanciaAteAndar arr[], int n) {
    quickSort(arr, 0, n - 1);
}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
static byte key_pressed_counter=20;
static byte key_released_counter,column=FIRST_COLUMN;
static unsigned int row_data,crt_key;
// Reinitialize Timer 0 value
TCNT0=0x8D; // para 2ms
// Place your code here
row_data<<=4;
// get a group of 4 keys in in row_data
row_data|=~KEYIN&0xf;
column>>=1;
if (column==(LAST_COLUMN>>1))
   {
   column=FIRST_COLUMN;
   if (row_data==0) goto new_key;
   if (key_released_counter) --key_released_counter;
   else
      {
      if (--key_pressed_counter==9) crt_key=row_data;
      else
         {
         if (row_data!=crt_key)
            {
            new_key:
            key_pressed_counter=10;
            key_released_counter=0;
            goto end_key;
            };
         if (!key_pressed_counter)
            {
            keys=row_data;
            key_released_counter=20;
            };
         };
      };
   end_key:;
   row_data=0;
   };
// select next column, inputs will be with pull-up
KEYOUT=~column;
}

unsigned inkey(void)
{
unsigned k;
if (k=keys) keys=0;
return k;
}

void init_keypad(void)
{
// PORT D initialization
// Bits 0..3 inputs
// Bits 4..7 outputs
DDRA=0xf0;
// Use pull-ups on bits 0..3 inputs
// Output 1 on 4..7 outputs
PORTA=0xff;
// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 57.600 kHz
// Mode: Normal top=FFh
// OC0 output: Disconnected
//TCCR0=0x03;
//INIT_TIMER0;
TCCR0=0x04;
TCNT0=0x8D;
OCR0=0x00;

// External Interrupts are off
//MCUCR=0x00;
//EMCUCR=0x00;
// Timer 0 overflow interrupt is on
//TIMSK=0x02;
// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x01;
#asm("sei")
}

// Declare your global variables here
// LCD display buffer
char lcd_buffer[33];
int estado_sensor_andares[8];

// Passo atual do motor
int step_number = 0;

void atualiza_estado_sensores() {
    estado_sensor_andares[0] = PIND.0;
    estado_sensor_andares[1] = PIND.1;
    estado_sensor_andares[2] = PIND.2;
    estado_sensor_andares[3] = PIND.3;
    estado_sensor_andares[4] = PIND.4;
    estado_sensor_andares[5] = PIND.5;
    estado_sensor_andares[6] = PIND.6;
    estado_sensor_andares[7] = PIND.7;
}

// Fun��o para ordenar um array usando o algoritmo Bubble Sort
void bubble_sort(int arr[], int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        // �ltimos i elementos j� est�o na posi��o correta
        for (j = 0; j < n - i - 1; j++) {
            // Troca se o elemento encontrado for maior do que o pr�ximo elemento
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void print_array(int array[], int size){  
    int i;
    printf("\r\n");
    for(i = 0; i < size; i++) {
        printf("%i ", array[i]);
        if(i == (size-1)){
            printf("\r\n");        
        }
    }
}

unsigned identifica_andar_atual(){
    // Completar COLOCAR NAS PORTAS D    

    int k;
    
    if(estado_sensor_andares[0]==1)
            andar_atual =0;
    if(estado_sensor_andares[3]==1)
            andar_atual = 1;
    if(estado_sensor_andares[2]==1)
            andar_atual = 2;
    
            
    printf("\r\nAndar Atual %i",andar_atual);
    return andar_atual; // remover
}

void sobe_elevador(){
    PORTB.0 = 1;
    PORTB.1 = 0;   
    PORTB.2 = 1;
    PORTB.3 = 1;           
    controle_delay_parado=1;
}


void desce_elevador(){
    PORTB.0 = 0;
    PORTB.1 = 1;   
    PORTB.2 = 1;   
    PORTB.3 = 1;                
    controle_delay_parado=1;
}

void para_elevador(){
    andares_acionados[andar_atual] = 0;
    PORTB.0 = 0;
    PORTB.1 = 0;   
    if(controle_delay_parado==1){
        PORTB.2 = 0;      
        PORTB.3 = 0;
        delay_ms(2000);     
        controle_delay_parado=0;
    }  
      
}

int compareDistanciasAteAndares(const void *a, const void *b) {
    DistanciaAteAndar *elem1 = (DistanciaAteAndar *)a;
    DistanciaAteAndar *elem2 = (DistanciaAteAndar *)b;
    return elem1->distancia - elem2->distancia; // Compara��o em ordem crescente
}               

void sortDistanciasAteAndares(int arr[], int n) {
    int i;
    DistanciaAteAndar *elements = malloc(n * sizeof(DistanciaAteAndar));

    // Preencher a estrutura com valores e �ndices
    for (i = 0; i < n; i++) {
        elements[i].distancia = arr[i];
        elements[i].andar = i;
    }

    // Ordenar usando qsort
    qsort_custom(elements, n);

    // Imprimir os valores ordenados e seus �ndices
    printf("Dist�ncias ordenadas e seus andares:\n");
    
    for (i = 0; i < n; i++) {
        printf("Dist�ncia: %d, Andar: %d\n", elements[i].distancia, elements[i].andar);
    }

    // Liberar mem�ria
    free(elements);
}

void atualiza_fila_de_andares(char andares_acionados[], int fila[], int size) {
    //int andar_atual = identifica_andar_atual();
    int i;
    int subtracoes_andares[3];
    
    for(i = 0; i<3; i++){
        subtracoes_andares[i] = abs(andares_acionados[i]*(andar_atual-(andares_acionados[i]*i)));
    }     
    
    print_array(subtracoes_andares, sizeof(subtracoes_andares) / sizeof(subtracoes_andares[0]));
    
    sortDistanciasAteAndares(subtracoes_andares, (sizeof(subtracoes_andares) / sizeof(subtracoes_andares[0])));
    
    /*
    bubble_sort(subtracoes_andares, 3);
    printf("\r\nArray sub andares\r\n ");
    print_array(subtracoes_andares, 3);
                                       
    
    for(i = 0; i<3; i++) {
        int resultado_atual = abs(subtracoes_andares[i]-andar_atual);
        if (resultado_atual == andar_atual) fila[i] = 0;
        else fila[i] = resultado_atual;
    }
    
    if(fila[1] == 1 && fila[2] == 1) 
        fila[2]=3;                                           
    
    print_array(fila, 3);
    */
}

void define_proximo_andar() {
    //int andar_atual = identifica_andar_atual();

    switch(andar_atual) {
        case 0:
            if(andares_acionados[1] == 1){
                proximo_andar = 1;
                sobe_elevador();
            }
            else if(andares_acionados[2] == 1){
                proximo_andar = 2;
                sobe_elevador();
                //delay_ms(100);
            }
            break;
        case 1:
            if(andares_acionados[0] == 1){
                proximo_andar = 0;
                desce_elevador();
            }
            else if(andares_acionados[2] == 1){
                proximo_andar = 2;
                sobe_elevador();
                //delay_ms(100);
            }
            break;
        case 2:
            if(andares_acionados[1] == 1){
                proximo_andar = 1;
                desce_elevador();
            }
            else if(andares_acionados[0] == 1){
                proximo_andar = 0;
                desce_elevador();
            }
            break;
    };
}

void verifica_chegada_andar_objetivo() {
    //int andar_atual = identifica_andar_atual();
    
    if(andar_atual == proximo_andar || proximo_andar == -1)
        para_elevador();    
}

void rotate_stepper_motor(int dir) {
    if (dir) {
        switch(step_number){
            case 0:
                PORTB.1 = 1;
                PORTB.2 = 0;
                PORTB.3 = 0;
                PORTB.4 = 0;
                break;
            case 1:
                PORTB.1 = 0;
                PORTB.2 = 1;
                PORTB.3 = 0;
                PORTB.4 = 0;
                break;
            case 2:
                PORTB.1 = 0;
                PORTB.2 = 0;
                PORTB.3 = 1;
                PORTB.4 = 0;
                break;
            case 3:
                PORTB.1 = 0;
                PORTB.2 = 0;
                PORTB.3 = 0;
                PORTB.4 = 1;
                break;
        } 
    } else {
        switch(step_number){
            case 0:
                PORTB.1 = 0;
                PORTB.2 = 0;
                PORTB.3 = 0;
                PORTB.4 = 1;
                break;
            case 1:
                PORTB.1 = 0;
                PORTB.2 = 0;
                PORTB.3 = 1;
                PORTB.4 = 0;
                break;
            case 2:
                PORTB.1 = 0;
                PORTB.2 = 1;
                PORTB.3 = 0;
                PORTB.4 = 0;
                break;
            case 3:
                PORTB.1 = 1;
                PORTB.2 = 0;
                PORTB.3 = 0;
                PORTB.4 = 0;  
                break;
        }
    }
}

void main(void)
{
// Declare your local variables here
unsigned int k;

// Input/Output Ports initialization
// Port A initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTA=0x00;
DDRA=0x00;

// Port B initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=0 State0=0 
PORTB=0x00;
DDRB=0xFF;

// Port C initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTD=0x00;
DDRD=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 14745,600 kHz
// Mode: Normal top=0xFF
// OC0 output: Disconnected
TCCR0=0x01;
TCNT0=0x00;
OCR0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 14745,600 kHz
// Mode: Fast PWM top=0xFF
// OC2 output: Non-Inverted PWM
ASSR=0x00;
TCCR2=0x69;
TCNT2=0x00;
OCR2=200;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
MCUCR=0x00;
MCUCSR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x01;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud Rate: 19200
UCSRA=0x00;
UCSRB=0x18;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x2F;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC disabled
ADCSRA=0x00;

// SPI initialization
// SPI disabled
SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;

// Alphanumeric LCD initialization
// Connections specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTC Bit 0
// RD - PORTC Bit 1
// EN - PORTC Bit 2
// D4 - PORTC Bit 4
// D5 - PORTC Bit 5
// D6 - PORTC Bit 6
// D7 - PORTC Bit 7
// Characters/line: 16
lcd_init(16);

// Global enable interrupts
#asm("sei")
//PORTD.7=1;// para apagar o led
init_keypad();  
PORTC.3=1;

while (1)
    {
        atualiza_estado_sensores();   
        printf("\r\nArray de estados dos sensores");
        print_array(estado_sensor_andares, 8);
             
        identifica_andar_atual();  
  
                                      
        printf("\r\nArray de andares acionados");
        print_array(andares_acionados, 3);
        
        define_proximo_andar();
        verifica_chegada_andar_objetivo();
          
        
        printf("\r\nPr�ximo andar: %i\r\n", proximo_andar);

        //printf("\r\nArray de fila de andares\r\n");
        //atualiza_fila_de_andares(andares_acionados, fila_de_andares, 3);
                
        if (k=inkey())
        {   
            if (k >= 0x1000 && k <= 0x8000) {
                switch(k) {
                    case 0x8000:              
                        sprintf(lcd_buffer, "Terreo");
                        andar_elevador = 0;
                        andares_acionados[0] = 1;
                        break;
                    case 0x4000:             
                        sprintf(lcd_buffer, "1\xdf andar");                                           
                        andar_elevador = 1;                      
                        andares_acionados[1] = 1;
                        break;
                    case 0x2000:             
                        sprintf(lcd_buffer, "2\xdf andar");
                        andar_elevador = 2; 
                        andares_acionados[2] = 1;
                        break;     
                    /*
                    case 0x1000:             
                        sprintf(lcd_buffer, "3\xdf andar");
                        andar_elevador = 3;
                        andares_acionados[3] = 1;
                        break;
                    */
                }    
                printf("\r\nBot�o interno do elevador pressionado\r\n");
            }
            else if (k >= 0x1 && k <= 0x8) {  
                switch(k) {
                    case 0x8:   
                        sprintf(lcd_buffer, "Terreo");
                        andares_acionados[0] = 1;
                        break;
                    case 0x4:
                        sprintf(lcd_buffer, "1\xdf andar");
                        andares_acionados[1] = 1;
                        break;
                    case 0x2:
                        sprintf(lcd_buffer, "2\xdf andar");
                        andares_acionados[2] = 1;
                        break;
                    /*  
                    case 0x1:    
                        sprintf(lcd_buffer, "3\xdf andar");
                        andares_acionados[3] = 1;
                        break;
                    */                          
                }   
                printf("\r\nBot�o externo de chamada do elevador pressionado\r\n");
            }
            else 
            {
                printf("\r\nNenhum bot�o foi pressionado\r\n");
            }
            lcd_clear();
            lcd_gotoxy(3,1);
            lcd_puts(lcd_buffer);                    
        }
        else 
        {
            printf("\r\nNenhum bot�o foi pressionado\r\n ");
        }
        
        //rotate_stepper_motor(0);             
        
        //step_number++;
        //if(step_number > 3){
        //    step_number = 0;
        //}   
          
        /*
        printf("\r\nDIRE��O 1\r\n ");
        sobe_elevador();              
        delay_ms(3000);
        printf("\r\nDIRE��O 2\r\n ");
        para_elevador();
        delay_ms(3000); 
        printf("\r\nDIRE��O 3\r\n ");
        desce_elevador();
        */
        delay_us(10);
    }
}
