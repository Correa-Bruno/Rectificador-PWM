//######################################################### SISTEMAS DIGITALES IV ##################################################################################################################
// ALUMNOS: Correa Bruno, De Battista Cristian; Errecart Matias
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000ul
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int modoant = 0;
int contador = 0;
int i,k;
int indice = 0;
int cData;
int DSPI;
int cen=0;
int dec=0;
int uni=0;

long ValorPote;

char local[]="L";
char remoto[]="R";
char on_off[]="O";
char cambio[]="E";
char modo[5];
char estado[5];
char valor[5];
char DATO[5];									// String para armar cadena de caracteres

unsigned char Resto;

volatile unsigned char bPB8=1;					// Bandera estado de PB0
volatile unsigned char bTX=0;					// Bandera para transmitir dato

void CONTROL(char DATO[]);						// Funcion control PWM por puerto serie
void onoff(void);								// Funicion para mostrar estado PWM
void SPI_MasterTransmit(int cData);				// Funcion para transimitir al display
void SPI_initial(void);							// Funcion inicializar display
uint8_t LecturaPin7(void);						// Funcion de lectura de PD7 int On_Off_PWM			


int main(void)
{
	strcpy(modo,local);
	
	DDRD |=(1<<DDD5)|(0<<DDD7);			// PD5 como salida PWM (OC0B) y PD7(7) como entrada (On/Off_PWM en modo local)
	DDRB |=(1<<DDB5)|(1<<DDB3)|(1<<DDB2)|(1<<DDB1)|(1<<DDB0);	// Config como salida PB5(13,CSK), PB3(11,MOSI), PB2(10,SS), PB0(8)semiciclo positivo, PB1(9) semiciclo negativo
	PORTD |= (1<<PORTD2)|(1<<PORTD3)|(1<<PORTD7);			// Configuro Restistencias pull up INT0, INT1, On_Off_PWM
	PORTB |= (1<<PORTB2);									// Resistencia Pull-UP

	TCCR0A= (0<<COM0B1)|(0<<COM0B0)|(0<<WGM01)|(1<<WGM00);	// PWM fase correcta,
	TCCR0B= (0<<CS02)|(1<<CS01)|(0<<CS00)|(1<<WGM02);		// Selecciono prescaler en 8

	OCR0A= 199;												// Define la frecuencia pwm 5KHz
	OCR0B = 100;											// Define el ancho de pulso

	EIMSK= (1<<INT1)|(1<<INT0);								// Config INT0 INT1
	EICRA= (1<<ISC11)|(0<<ISC10)|(1<<ISC01)|(1<<ISC00);		// Config INT1 flanco descendente, INT0 flanco ascendente

	TCCR1A=0;
	TCCR1B=4;												// Selector de reloj CKl/256
	TCNT1=30360;											// 1s
	TIMSK1=(1<<TOIE1);										// Habilitaci�n de interrupci�n por desbordamiento

	UCSR0A=0;
	UBRR0=103;												// Velociadad de transmicion en 9600
	UCSR0C=(0<<UMSEL01)|(0<<UMSEL00)|(1<<UCSZ01)|(1<<UCSZ00)|(0<<USBS0)|(0<<UPM01)|(0<<UPM00);	// Modo Asincrono, size caracter 8-bit, 1-bit de stop, paridad OFF
	UCSR0B=(1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);			// Habilito receptor y transceptor, interrupcion recepcion completa

	ADMUX= (1<<REFS0);										// Tension de referencia con capacitor externo
	ADCSRA=(1<<ADEN)|(0<<ADSC)|(1<<ADPS1)|(1<<ADPS0);		// Habilito el ADC, Configuro prescaler en 8
	
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);					// Configuracion del SPI Modo Maestro, frecuencia SCK 1Mhz
	SPI_initial();
	
	sei();
	
    while(1)
	{  
		if(bTX==1)											// cuando se pone en alto la bandera de recepcion retransmite el dato
		{
			bTX=0;											//pongo en 0 la bandera
		}
		if ((strcmp(modo,local)==0)&&(LecturaPin7()==1))	//si es modo local leo el y PB0 esta en 1
		{														
			TCCR0A ^= (1<<COM0B1);						   //Cambio de estado PWM
		    onoff();
	    }
		if ((strcmp(modo,remoto)==0)&&(strcmp(estado,on_off)==0))
		{
			TCCR0A ^= (1<<COM0B1);							//Cambio de estado PWM
			strcpy(estado,cambio);
			onoff();
		}	
		if(contador==0)
		{
			PORTB = (0<<PORTB2);				//Indico inicio de transferencia
			_delay_us(1);
			DSPI = 0x09;
			SPI_MasterTransmit(DSPI);
			_delay_us(1);
			DSPI = 0x00;						//Sin decodificaci�n
			SPI_MasterTransmit(DSPI);
			PORTB = (1<<PORTB2);				//Indico fin de transferencia
			_delay_us(1);
			
			PORTB = (0<<PORTB2);				//Indico inicio de transferencia
			_delay_us(1);
			DSPI = 0x01;						//Bit 0
			SPI_MasterTransmit(DSPI);
			_delay_us(1);
			DSPI = 0x47;
			SPI_MasterTransmit(DSPI);
			PORTB = (1<<PORTB2);				//Indico fin de transferencia
			_delay_us(1);
			
			PORTB = (0<<PORTB2);				//Indico inicio de transferencia
			_delay_us(1);
			DSPI = 0x02;						//Bit 1
			SPI_MasterTransmit(DSPI);
			_delay_us(1);
			DSPI = 0x47;
			SPI_MasterTransmit(DSPI);
			PORTB = (1<<PORTB2);				//Indico fin de transferencia
			_delay_us(1);
			
			PORTB = (0<<PORTB2);				//Indico inicio de transferencia
			_delay_us(1);
			DSPI = 0x03;						//Bit 2
			SPI_MasterTransmit(DSPI);
			_delay_us(1);
			DSPI =0x7E;
			SPI_MasterTransmit(DSPI);
			PORTB = (1<<PORTB2);				//Indico fin de transferencia
			_delay_us(1);
		}
		_delay_ms(25);
	}
}

//######################################################### FUNCION PARA ESTABLECER CONTROL LOCAL/REMOTO #######################################################################################

void CONTROL(char DATO[])
{
	char mremoto[]="Modo Remoto\n";
	char mlocal[]="Modo Local\n";
	int cen = 0,dec = 0,uni = 0,resultado = 0;
	
	if (strcmp(DATO,remoto)==0||strcmp(DATO,local)==0)
	{
		strcpy(modo,DATO);
	}
	if ((strcmp(DATO,on_off)==0)&&(strcmp(modo,remoto)==0))
	{
		strcpy(estado,DATO);
	}
	else
	{
		strcpy(valor,DATO);
	}
	if (strcmp(modo,remoto)==0)
	{
		if (modoant!=1)
		{
			for (i=0;i<=strlen(mremoto);i++)
			{
				while ( !(UCSR0A &(1<<UDRE0)));
				UDR0=mremoto[i];
			}
		}
		ADCSRA = (0<<ADEN)|(0<<ADSC);						//OFF CAD
		modoant=1;
	}
	if (strcmp(modo,local)==0)								//Pregunta si es modo LOCAL
	{
		if (modoant!=2)
		{
			for (i=0;i<=strlen(mlocal);i++)
			{
				while ( !(UCSR0A &(1<<UDRE0)));
				UDR0=mlocal[i];
			}
		}													//Si es local:
		ADCSRA = (1<<ADEN)|(0<<ADSC)|(1<<ADPS1)|(1<<ADPS0);	//ON CAD
		modoant=2;
	}
	if (strcmp(modo,remoto)==0)
	{														//Si es REMOTO:
		cen = (valor[0] - 48) * 100;
		dec = (valor[1] - 48) * 10;
		uni = valor[2] - 48;
		resultado = cen + dec + uni;						//ON PWM
		if(resultado<=100)
		{
			ValorPote = resultado;
		}
	}
}

//######################################################### FUNCION PARA ENCENDER/APAGAR PWM EN MODO LOCAL ##################################################################################################################

uint8_t LecturaPin7(void)
{
	if((PIND & (1<<PIND7)) == 0)					// Si el bot�n esta presionado.
	{ 
		_delay_ms(50);								// Retardo de entrada para el valor le�do.
	}
	if((PIND & (1<<PIND7)) == 0)					// Verifica que la la lectura sea correcta.
	{ 
		return 1;									// Si todav�a es 0 es porque si ten�amos pulsado el bot�n
	}
	else 
	{	
		return 0;									// Si el valor cambio la lectura es incorrecta.
	}		
}

//######################################################### VECTOR DE INTERRUPCION RECEPCION DE DATOS ##################################################################################################################

ISR(USART_RX_vect)
{
	char datoRX=UDR0;	// Leemos el buffer de recepcion
	
	if(datoRX=='$')		// si se cumple pone en 1 la bandera de dato recibido
	{
		bTX=1;
		indice=0;
	}
	else
	{
		if((datoRX>='0') &&  (datoRX<='9'))		// Si es un numero lo guarda en DATO
		{
			DATO[indice]=datoRX;
			indice++;							
			}
				else
				{				// Si es una letra pone indice en 0, guarda el dato recibido y limpia lo demas
				indice=0;
				DATO[indice]=datoRX;
				DATO[1]='\0';
				DATO[2]='\0';
				}
	}
	CONTROL(DATO);
	return;
}


//######################################################### RUTINA DE TRATAMIENTO DE INTERRUPCION EXTERNA ##################################################################################################################

ISR (INT1_vect) // PD3(pin3)
{

	PORTB &= ! (1<<PORTB0); // En 0 PINB0
	_delay_us(100);
	PORTB |= (1<<PORTB1); // En 1 PB1(9) Semiciclo negativo

}

ISR (INT0_vect) // PD2(pin2)
{
	PORTB &= !(1<<PORTB1); // Apago  PB1
	_delay_us(100);
	PORTB |= (1<<PORTB0); // En 1 PB0(8) Semiciclo positivo

}

//######################################################### RUTINA DE TRATAMIENTO DE INTERRUPCION DEL CAD ##################################################################################################################

ISR(TIMER1_OVF_vect)
{
	ADCSRA |= (1<<ADSC);										// Iniciamos conversion
	while(ADCSRA & (1<<ADSC));
	if (strcmp(modo,local)==0)
	{
		ValorPote = ADC;
		ValorPote = (long)ValorPote*100/1023;				// Guardamos el valor ADC en una variable
	}
	
	cen = (ValorPote/100);
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x03;						//Bit 2
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = (0x00 + cen);
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	UDR0 = cen + 48;
	while(!(UCSR0A & (1<<UDRE0)));		// Espera a que se env�e el dato
	
	Resto = (ValorPote%100);
	
	dec = (Resto/10);
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x02;						//Bit 1
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = (0x00 + dec);
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	UDR0 = dec + 48;
	while(!(UCSR0A & (1<<UDRE0)));
	
	uni = (Resto%10);
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x01;						//Bit 0
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = (0x00 + uni);
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	UDR0 = (uni + 48);
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = 10;
	while(!(UCSR0A & (1<<UDRE0)));
	TCNT1 = 30360;						// Espera 1 seg para realizar la conversion
	OCR0B = ValorPote*199/100;			// PD5 salida PWM
}
//######################################################### FUNCION DE MOSTRAR ON/OFF ##################################################################################################################

void onoff ()
{
	char on[]="PWM on\n";
	char off[]="PWM off\n";
	
	contador++;
	
	if ((contador>1))
	{
		contador=0;
	}
	if ((contador == 1))
	{
		for (i=0;i<=strlen(on);i++)
		{
			while ( !(UCSR0A &(1<<UDRE0)));
			UDR0=on[i];
			SPI_initial();
		}
	}
	if ((contador == 0))
	{
		
			for (i=0;i<=strlen(off);i++)
			{
				while ( !(UCSR0A &(1<<UDRE0)));
				UDR0=off[i];	
			}
			
	}
	return;
}
//######################################################### SPI_MasterTransmit ##################################################################################################################

void SPI_MasterTransmit(int cData)
{
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)))
	;
}

//################################################################ SPI ##################################################################################################################

void SPI_initial()
{
	////////////////////////////////////////// Intensidad de brillo /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x0A;						//Intensidad minima
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0x00;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	//////////////////////////////////////// Modo de decodificacion /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x09;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0xFF;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	////////////////////////////////////////// Cantidad de digitos /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x0B;						//Mostrar 3 digitos
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0x02;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	////////////////////////////////////////// Modo de operacion /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x0C;						//Opercion normal
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0x01;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	////////////////////////////////////////// Modo de testeo /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x0F;						//Opercion normal
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0x00;						//Decodificaci�n de c�digo B para d�gitos 7-0
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	////////////////////////////////////////// Inician todos apagados /////////////////////////////////////////
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x01;						//Bit 0
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0xFF;
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x02;						//Bit 1
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0xFF;
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
	
	PORTB = (0<<PORTB2);				//Indico inicio de transferencia
	_delay_us(1);
	DSPI = 0x03;						//Bit 2
	SPI_MasterTransmit(DSPI);
	_delay_us(1);
	DSPI = 0xFF;
	SPI_MasterTransmit(DSPI);
	PORTB = (1<<PORTB2);				//Indico fin de transferencia
	_delay_us(1);
}

