/*OBJETIVO: Adaptar el programa a Arduino UNO
 * Arduino Mega
 * 
 * Puerto K: Puerto de salida a segmentos
 *  del pin K0 al K6 se conectan a los segmentos A-G de los display
 * Puerto F: Puerto de salida a transistores de multiplexado
 *  se utilizan del pin F0 al F3 se conectaran a las bases de los transistores
 *  
 *IMPORTANTE: esta preparado para mostrar segundos y minutos para ver el correcto funcionamiento del programa y multiplexado
 *Para cambiar a que muestre minutos y horas
 *hay que cambiar los parametros '(s)' en las lineas 110 y 115 por (m)
 *Y los parametros (m) en las lineas 120 y 125 por (h)
 *
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#define D 5

int s=40,m=36,h=14;       //variables de cada unidad de tiempo (Se setea con algun valor inicial, idealmente seria la hora actual
                          //s=segundos
                          //m=minutos
                          //h=horas
                          
int dur[3];               //Variable de duracion de cada alarma
char V[10]={63,6,91,79,102,109,125,7,127,111};    //Vector con los numeros del 0-9 traducidos de decimal a 7 segmentos
boolean dir=false;

int alarma[3][3]={        //Vector bidimensional de las alarmas
  {5,37,14},              //Cuando las variables de tiempo igualen los valores de alguna fila de vectores, se accionará la alarma
  {13,37,14},             //El orden es segundos/minutos/hora
  {50,36,14}
};

void setup() {
  DDRB=255;               //Puerto B como salida, accionará el timbre o mecanismo de alarma
  DDRK=255;               //Puerto K como salida a display
  DDRF=255;               //Puerto F como salida a transistores
  
  cli();                  //deshabilito las interrupciones
  
  TCCR1A=0;               //limpiamos los registros de control
  TCCR1B=0;               //del timer 1

  OCR1A=15625;            //Prescaler calculado para activar la ISR c/segundo
  TCCR1B|=(1<<WGM12);
  TCCR1B|=(1<<CS10);
  TCCR1B|=(1<<CS12);
    
  TIMSK1=(1<<OCIE1A);     //enciendo el timer1


  TCCR3A=0;                           
  TCCR3B=0;

  OCR3A=1;                //Prescaler del timer 3 calculado a 64uSeg
  TCCR3B|=(1<<WGM12);
  TCCR3B|=(1<<CS10);
  TCCR3B|=(1<<CS12);

  TIMSK3=(1<<OCIE1A);     //Enciendo el timer 3
  
  sei();                  //activamos las interrupciones
  Serial.begin(9600);     //Activamos el monitor Serial para visualizar la hora en el monitor del arduino
  PORTB=0;
  PORTF=1;
  PORTK=0;
}

void loop() {                 //Se utiliza el loop para el funcionamiento de la alarma
  int i;
  for(i=0;i<3;i++){
    dur[i]=alarma[i][0]+D;    //Dur marca la duracion de la alarma, solo hace falta modificar la constante D al principio del programa
  }
  for(i=0;i<3;i++){           //Este bloque analiza si el horario coincide con alguna alarma
    if(alarma[i][2]==h && alarma[i][1]==m && alarma[i][0]==s){//Si las 3 variables de alguna fila del vector coincide con la hora dada
      while(s!=dur[i])
      PORTB=128;              //Se encendera el led hasta que los segundos igualen dur
    }
    else
      PORTB=0;
  }
}

ISR(TIMER1_COMPA_vect){       //rutina de contador
  if(s<59)                    //59=valor maximo que pueden alcanzar los segundos antes de aumentar el minutos
    s++;
  else{
    s=0;
    if(m<59)                  //Idem que los segundos pero con los minutos antes de aumentar la hora
    m++;
    else{
      m=0;
      if(h<23)                //Limite de horas antes de volver a 0
        h++;
      else h=0;
    }
  }
//  Serial.println("Hora:");    //Imprime el horario en el monitor serie
//  Serial.println(h);
  Serial.println("Minutos");
  Serial.println(m);
  Serial.println("Segundos");
  Serial.println(s); /* 
  Serial.println("Multiplexado");  
  Serial.println(PORTF);  
  Serial.println("Segmentos");  
  Serial.println(PORTK);  */
  
}


ISR(TIMER3_COMPA_vect){ //Funcion de multiplexado
  dir=!dir;
  if(dir){
    PORTK=0;
    PORTF=V[ru(s)];      
    PORTK=1;            //Habilito el transistor conectado al display que representa la unidad de segundos
  }
  else{
    PORTK=0;
    PORTF=V[rd(s)];
    PORTK=2;            //Habilito el transistor conectado al display que representa la decena de segundos
  }
  if(dir){
    PORTK=0;
    PORTF=V[ru(m)];
    PORTK=4;            //Habilito el transistor conectado al display que representa la unidad de minutos
  }
  else{
    PORTK=0;
    PORTF=V[rd(m)];
    PORTK=8;            //Habilito el transistor conectado al display que representa la decena de minutos
  }
}

int rd(int p){          //funcion que aisla el digito decimal de la variable de tiempo pasada como parametro
  int r=p/10;           //Se calcula el digito decimal y se lo guarda en 'r'
  return r;             //Se devuelve r
}


int ru(int p){          //funcion que aisla el digito de unidad de la variable de tiempo pasada como parametro
  int r=p%10;           //Se calcula el digito de unidad y se lo guarda en 'r'
  return r;             //Se devuelve r
}
