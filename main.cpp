// Блок индикации ВЕГА 122С
#include "device.h"


#define INTERRUPT_USE
#define WATCHDOG_USE
device dev;

ISR(TIMER0_OVF_vect) { dev.tick(); }
ISR(ADC_vect)        { dev.adc (); }

int main (void){
    
    DDRD  =  (1 << PIN_WR_REG_LED) |  (1 << PIN_WR_REG_DIGIT) | (1 << PIN_COUNTER_DIGECT_O) | 
             (1 << PIN_COUNTER_DIGECT_1) | (1 << PIN_COUNTER_DIGECT_2) | (1 << PIN_ZERO_SIGNAL);
    /** Подтягивающий резистор для входа направления счета */
    PORTD =  (1 << PIN_INPUT_REWIND);
    DDRC  =  (1 << PIN_LED_LK) |  (1 << PIN_LED_PK);
    DDRB  =  0xFF;
    /* Подтяжка к питанию кнопки сброса счетчика */
    PORTC =  (1 << PIN_CLEAR);
   
    /*
    CS02    CS01    CS00        Description
    0       0       0           No clock source (Timer/Counter stopped)
    0       0       1           clk(No prescaling)
    0       1       0           clk/8 (From prescaler)
    0       1       1           clk/64 (From prescaler)
    1       0       0           clk/256 (From prescaler)
    1       0       1           clk/1024 (From prescaler)
    1       1       0           External clock source on T0 pin. Clock on falling edge.
    1       1       1           External clock source on T0 pin. Clock on rising edge.
    */
    TCCR0B = (1 << CS00) ;      /* Тактовый сигнал для счетчика брать CLK без предделителя */
    TIMSK0 = (1 << TOIE0) ;     /* Разрешить прерывания от счетчика 0 */

    /* Брать тактовый сигнал со входа T1 (PD5) */
    TCCR1B = (1 << CS10) | (1 << CS11) | (1 << CS12); 
    /*
    REFS1   REFS0   Voltage Reference Selection
        0       0   AREF, Internal Vref turned off
        0       1   AVCC with external capacitor at AREF pin
        1       0   Reserved
        1       1   Internal 1.1V Voltage Reference with external capacitor at AREF pin
    */
    ADMUX  = (1 << REFS0) ;//| (1 << REFS1) ; // источник опорного напряжения.
    /*
    ADTS2   ADTS1   ADTS0       Trigger Source
        0       0       0       Free Running mode
        0       0       1       Analog Comparator
        0       1       0       External Interrupt Request 0
        0       1       1       Timer/Counter0 Compare Match A
        1       0       0       Timer/Counter0 Overflow
        1       0       1       Timer/Counter1 Compare Match B
        1       1       0       Timer/Counter1 Overflow
        1       1       1       Timer/Counter1 Capture Event
    */
    ADCSRB = 0; //(1 << ADTS2) ;
    /*
    BIT 7    BIT 6    BIT 5    BIT 4    BIT 3    BIT 2    BIT 1    BIT 0
    ADEN     ADSC     ADATE    ADIF     ADIE     ADPS2    ADPS1    ADPS0

    ADEN    -   Включает или выключает АЦП (1-включен).
    ADSC    -   Запускает преобразование если в него записать 1 (для многоразового режима запуск первого преобразования).
    ADATE   -   Позволяет запускать преобразование по прерыванию от периферийных устройств микроконтроллера если установить в 1.
    ADIF    -   Флаг прерывания от АЦП.
    ADIE    -   Разрешает прерывания от АЦП если установлен в 1.
    ADPS2 - ADPS0 - Выбирают режим работы предделителя тактовой частоты:
                    000 - CLK/2
                    001 - CLK/2
                    010 - CLK/4
                    011 - CLK/8
                    100 - CLK/16
                    101 - CLK/32
                    110 - CLK/64
                    111 - CLK/128
    */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADSC);
    wdt_enable (WDTO_250MS);
    sei();
    while (true){
        dev.run();     
    }
    return 0;
}