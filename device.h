
#ifndef __DEVICE_H__
#define __DEVICE_H__
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/wdt.h>

#define F_CPU F_OSC
#include <util/delay.h>

#include <inttypes.h>



#define sbi(reg,bit) reg |= (1<<bit)
#define cbi(reg,bit) reg &= ~(1<<bit)
#define ibi(reg,bit) reg ^= (1<<bit)
#define isBit(reg,bit) (reg&(1<<bit))
/* Включен ли детектор пиков */
#define   PEAKS_ENABLE                      true

#define    DATA_PORT                        PORTB

#define    PORT_WR_REG_LED                  PORTD
#define    PIN_WR_REG_LED                   PD3

#define    PORT_WR_REG_DIGIT                PORTD
#define    PIN_WR_REG_DIGIT                 PD4


#define    PORT_SELECT_CHANNEL_INDICATOR    PORTC
#define    PIN_LED_LK                       PC2
#define    PIN_LED_PK                       PC3

#define    CHANNEL_LED_OFF                  2
#define    CHANNEL_LED_LK                   0
#define    CHANNEL_LED_PK                   1

#define    DIGIT_COUNTER_TAPE_0             0
#define    DIGIT_COUNTER_TAPE_1             1
#define    DIGIT_COUNTER_TAPE_2             2
#define    DIGIT_COUNTER_TAPE_OFF           3

#define   PORT_SELECT_DIGIT_TAPE            PORTD
#define   PIN_COUNTER_DIGECT_O              PD2
#define   PIN_COUNTER_DIGECT_1              PD1
#define   PIN_COUNTER_DIGECT_2              PD0

#define   COUNTER_DELAY_TIMER               500

#define   PORT_ZERO_SIGNAL                  PORTD
#define   PIN_ZERO_SIGNAL                   PD7
#define   DURATION_ZERO_SIGNAL_MKS          500

#define  PORT_OUTPUT_CLEAR                  PORTC
#define  PORT_INPUT_CLEAR                   PINC
#define  PIN_CLEAR                          PC4

#define  PORT_INPUT_REWIND                  PIND
#define  PIN_INPUT_REWIND                   PD6
/* Количество импульсов на оборот шпинделя  */
#define CAPACITOR_TAPE_VALUE               10

#define INTRO_DELAYED                       3
#define ADC_COUNTER_DELAY                   5
/*  Питание +5В используемого в качестве опорного */
#define ADC_REFERENCE                       4.9

/* 
-   -   -   -   -   Установка уровней сигналов -    -   -   -   -   - 
Установка уровня сигнала в вольтах на входе АЦП. Примерное соответствие 
уровней в различных точках.

Уровень,дБ	- Уровень сигнала.
Л.вход,мв   - Напряжение сигнала на линейном входе Vrms.
Вход инд,В  - Напряжение на входе платы индикации Vrms
Амплитуда   - Амплитудное значение на входе платы индикации
Пик.детек,В - Напряжение после пикового детектора, амплитудное значение.

Уровень,дБ	| Л.вход,мв | Вход инд,В| Амплитуда |Пик.детек,В|Угол, гр |
-----------------------------------------------------------------------
|   -18	    |   63      |   0,20    |   0,28    |	0,15    |   75	  |
|   -12	    |   126	    |   0,40	|   0,57	|   0,33	|   73    |
|   -9	    |   177	    |   0,57	|   0,80	|   0,75	|   62	  |
|   -6	    |   251	    |   0,80	|   1,13	|   1,30	|   55	  |
|   -3	    |   354	    |   1,13	|   1,60	|   2,18	|   47	  |
|   0	    |   500	    |   1,60	|   2,26	|   3,41	|   41	  |
|   +1,5	|   594	    |   1,90	|   2,69	|   4,18	|   39	  |
|   +3	    |   706	    |   2,26	|   3,20	|   4,97	|   39	  |
-----------------------------------------------------------------------
*/
#define     ADC_LEVEL(value)               (uint16_t)((value*1024)/ADC_REFERENCE)

#define    LEVEL_M18_DB                     ADC_LEVEL(0.15) //  -18 дБ
#define    LEVEL_M12_DB                     ADC_LEVEL(0.32) //  -12 дБ
#define    LEVEL_M9_DB                      ADC_LEVEL(0.73) //  -9 дБ
#define    LEVEL_M6_DB                      ADC_LEVEL(1.26) //  -6 дБ
#define    LEVEL_M3_DB                      ADC_LEVEL(2.12) //  -3 дБ
#define    LEVEL_0_DB                       ADC_LEVEL(3.31) //  0 дБ
#define    LEVEL_1P5_DB                     ADC_LEVEL(4.05) //  +1.5 дБ
#define    LEVEL_P3_DB                      ADC_LEVEL(4.81) //  +3 дБ




class device {
	public:
		device();
        /*  Точка входа синхронизации от таймера                */
        void tick ();
        /*  Обработка прерывания окончания преобразования АЦП   */
        void adc ();
        /* Тело программы */
        void run ();
	protected:
        /* Преобразование измеренного значения в АЦП в код индикатора (светодиоды) */
        uint8_t decodeLevel (uint16_t value);
        /* Перевод числа (цифры 0-9) в семисегментный код*/
        uint8_t decode7segment (uint8_t value);
        /* Разбор числа на цифры и помещение результата в массив с кодом */
        void factor_digits(int value);
        /* Таймер 100 мс в основном цикле   */
        void timer ();
    private:
        /* Формирование импульса "НУЛЯ" для блока управления "А" */
        void resetCounterTape ();
        /* Нажата ли кнопка очистка счетчика расхода ленты */
        bool isClearCounter ();
        /* Записать данные в регистр индикации */
        void writeRegisterIndicator (uint8_t value);
        /* Записать данные в регистр индикации счетчика расхода ленты   */
        void writeRegisterCounterTape (uint8_t value);
        /* Выбор индикатора расхода ленты  */
        void selectCounterTapeDigit  (uint8_t value);
        /* Выбор канала для индикации (левый, правый, никакой)  */
        void selectChannelIndicator (uint8_t value);
        /*Сброс мусора из сегментов в ноль (когда количество разрядов уменьшилось в меньшую сторону)*/
        void resetSegments (uint8_t digits);
        /**/
        uint8_t peaks (uint8_t level);
        /**/
        void peakReset ();
        /* Массив с данными для индикации       */
        uint8_t LED[2];
        /* Массив с данными для индикатора расхода ленты    */
        uint8_t DIGIT[3];
        static const uint8_t SEGMENTS [10];
        /* Флаговая переменная для понимание какой индикатор на данный момент показывать */
        uint8_t current_digit;
        /* Флаговая переменная, для понимания какой канал показывать в индикации */
        bool current_channel;
        /*  Текущее значение показаний АЦП по каналам   */
        uint16_t levelADC_LK;
        uint16_t levelADC_PK;
        uint16_t counterTimer;
        int16_t  counterTape;
        /** Флаг устанавливаемый при нажатии кнопки сброс, и до того момента пока будет идти счетчик */
        bool     oneClear;
        void clearDigit ();
        void showIntro ();
        bool sync;
        bool intro;
        bool intro_direction;
        uint8_t intro_delayed;
        uint8_t last_peak;
        uint8_t hold_peak;
        /* Включен ли детектор пиков */
        const bool peaksEnable = PEAKS_ENABLE; 
        bool adcRelease;
        uint8_t adcCounterDelay;
};
#endif //__DEVICE_H__
