#include "device.h"
const uint8_t device::SEGMENTS[10] =    {
                                        0b11000000, // 0 - Инверсный режим - активный ноль.
                                        0b11111001, // 1
                                        0b10100100, // 2
                                        0b10110000, // 3
                                        0b10011001, // 4
                                        0b10010010, // 5
                                        0b10000010, // 6
                                        0b11111000, // 7
                                        0b10000000, // 8
                                        0b10010000  // 9
                                        };                                   

device::device(){
    
    current_digit   = 0;
    counterTape     = 0;
    counterTimer    = 1;
    oneClear        = true;
    intro           = true;
    intro_direction = true;
    last_peak       = 0;
    intro_delayed   = INTRO_DELAYED;
    levelADC_PK     = LEVEL_M18_DB;
    levelADC_LK     = LEVEL_M18_DB;
    adcRelease      = false;
    adcCounterDelay = ADC_COUNTER_DELAY;
    clearDigit();
}

void device::tick (){
        selectChannelIndicator(CHANNEL_LED_OFF);
        if (current_channel){
            writeRegisterIndicator(LED[CHANNEL_LED_LK]);
            selectChannelIndicator(CHANNEL_LED_LK);
        } else {
            writeRegisterIndicator(LED[CHANNEL_LED_PK]);
            selectChannelIndicator(CHANNEL_LED_PK);
        }
        current_channel = !current_channel;
        selectCounterTapeDigit(DIGIT_COUNTER_TAPE_OFF);
        if (current_digit>2) current_digit = 0;
        writeRegisterCounterTape(DIGIT[current_digit]);
        selectCounterTapeDigit(current_digit);
        current_digit++;
        if (isClearCounter() && oneClear) {
            TCNT1 = 0;
            counterTape = 0;
            oneClear  = false;
            clearDigit();
        }
        sync = true;
       

}
void device::run (){
        sbi (PORT_ZERO_SIGNAL,PIN_ZERO_SIGNAL);
        wdt_reset(); 
        if (sync) {
            counterTimer--;
            sync = false;
            
            if (!counterTimer) {
                counterTimer = COUNTER_DELAY_TIMER;
                timer();
                peakReset();
               
            }
            
        }
        if (intro) {
            showIntro();
        }
        cli ();
        if (adcRelease){
            if (!adcCounterDelay--){
                 /*Запускаем измерение, следующий цикл измерения*/
                sbi (ADCSRA, ADSC) ;
                adcRelease = false;
                adcCounterDelay = ADC_COUNTER_DELAY;
            }
        }
        sei ();
}
void device::adc (){
        if (intro) return;
        /*Проверить какой канал был измерен (левый или правый)*/
        if (ADMUX & 0x01){
            /* Правый канала, записываем значения в локальную переменную */
            levelADC_PK = ADC;
            /* Конвертируем в светодиодную линейку и записываем в массив */
            LED [CHANNEL_LED_PK] = decodeLevel(levelADC_PK);
            /* Переключить на левый канал*/
            cbi (ADMUX,MUX0);
            /*То же только для левого канала */
        } else {
            levelADC_LK = ADC;
            LED [CHANNEL_LED_LK] = decodeLevel(levelADC_LK);
            /* Переключить на правый канал*/
            sbi (ADMUX,MUX0);
        }
            adcRelease = true;
        // _delay_us(3);
        // sbi (ADCSRA, ADSC) ;
}
void device::timer(){
        uint16_t  capacitorTape = TCNT1;
        if (capacitorTape<CAPACITOR_TAPE_VALUE){
            return;
        }else {
            TCNT1 = 0;
            if (isBit(PORT_INPUT_REWIND,PIN_INPUT_REWIND)){
                counterTape += capacitorTape/CAPACITOR_TAPE_VALUE;
            } 
            else {
                counterTape -= capacitorTape/CAPACITOR_TAPE_VALUE;
            }
            TCNT1 += capacitorTape%CAPACITOR_TAPE_VALUE;
        }
       
        if (counterTape>999) {
            counterTape = 0;
            clearDigit();
            }
        if (counterTape<0){
            resetCounterTape();
            counterTape = 999;
        }
        if (!counterTape) {
            resetCounterTape();
        }
        oneClear = (bool) counterTape;
        factor_digits(counterTape);
}
bool device::isClearCounter (){
    return !isBit(PORT_INPUT_CLEAR,PIN_CLEAR);
}
void device::clearDigit (){
    DIGIT [DIGIT_COUNTER_TAPE_0] = SEGMENTS[0];
    DIGIT [DIGIT_COUNTER_TAPE_1] = SEGMENTS[0];
    DIGIT [DIGIT_COUNTER_TAPE_2] = SEGMENTS[0];
}
void device::resetCounterTape (){
    cbi (PORT_ZERO_SIGNAL,PIN_ZERO_SIGNAL);
    _delay_us(DURATION_ZERO_SIGNAL_MKS);
    sbi (PORT_ZERO_SIGNAL,PIN_ZERO_SIGNAL);
}
void device::showIntro (){
    if (!(--intro_delayed)){
        if (intro_direction){
            levelADC_PK++;
            levelADC_LK++;
            if ((levelADC_LK > 1000) || (levelADC_PK > 1000)){
                intro_direction = false;
            }
        } else {
            levelADC_PK--;
            levelADC_LK--;
            if (!(levelADC_LK) || (!levelADC_PK)){
                intro = false;
                sbi (ADCSRA, ADSC) ;
            }
        }
        LED [CHANNEL_LED_PK] = decodeLevel(levelADC_PK);
        LED [CHANNEL_LED_LK] = decodeLevel(levelADC_LK);
        intro_delayed = INTRO_DELAYED;
    }
}
uint8_t device::peaks (uint8_t level){
    if (last_peak > level) level = last_peak;
    if (!level || !peaksEnable ) return 0;
    uint8_t value = 0b00000001;
    for (--level; level; level--)
    {
        value = value << 1;
    }
    return value;
}
void device::peakReset(){
    if (last_peak==hold_peak) {
        last_peak = 0;
    }
    else {
        hold_peak = last_peak;
    }
}
uint8_t device::decodeLevel (uint16_t value){
    if (value> LEVEL_P3_DB)  return  0b11111111 | peaks(7);
    if (value> LEVEL_1P5_DB) return  0b01111111 | peaks(6);
    if (value> LEVEL_0_DB)   return  0b00111111 | peaks(5);
    if (value> LEVEL_M3_DB)  return  0b00011111 | peaks(4);
    if (value> LEVEL_M6_DB)  return  0b00001111 | peaks(3);
    if (value> LEVEL_M9_DB)  return  0b00000111 | peaks(2);
    if (value> LEVEL_M12_DB) return  0b00000011 | peaks(1);
    if (value> LEVEL_M18_DB) return  0b00000001 | peaks(0);
    return 0;
}
uint8_t device::decode7segment (uint8_t value){
    if (value>9) return 0xFF;
    return SEGMENTS[value];
}
void device::writeRegisterCounterTape (uint8_t value){
    DATA_PORT = value;
    sbi (PORT_WR_REG_DIGIT,PIN_WR_REG_DIGIT);
    //_delay_us(1);
    cbi (PORT_WR_REG_DIGIT,PIN_WR_REG_DIGIT);
}
void device::writeRegisterIndicator(uint8_t value){
    DATA_PORT = value;
    // PORTD &= 0x3F;
    // PORTD |= value & 0xC0;
    sbi (PORT_WR_REG_LED,PIN_WR_REG_LED);
    //_delay_us(1);
    cbi (PORT_WR_REG_LED,PIN_WR_REG_LED);
}
void device::selectCounterTapeDigit(uint8_t value){
    sbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_O);
    sbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_1);
    sbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_2);
    switch (value)
    {
    case DIGIT_COUNTER_TAPE_0:
        cbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_O);
        break;
    case DIGIT_COUNTER_TAPE_1:
        cbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_1);
        break;
    case DIGIT_COUNTER_TAPE_2:
        cbi (PORT_SELECT_DIGIT_TAPE,PIN_COUNTER_DIGECT_2);
        break;
    }
}
void device::selectChannelIndicator (uint8_t value){
    switch (value)
    {
        case CHANNEL_LED_LK:
            sbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_LK);
            cbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_PK);
            break;
        case CHANNEL_LED_PK:
            sbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_PK);
            cbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_LK);
            break;    
        default:
            cbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_LK);
            cbi (PORT_SELECT_CHANNEL_INDICATOR,PIN_LED_PK);
            break;
    }
}

void device::factor_digits(int value)
{
    int digits_count = 0;
    if (!value) {
        resetSegments(digits_count);
        return;
    }
    while (value > 0) {
        // Копируем очередную цифру в массив
        DIGIT [digits_count++] = decode7segment(value % 10);
        // Переходим к следующей цифре
        value /= 10;
   }
    resetSegments(digits_count);
}
void device::resetSegments (uint8_t digits){
    for (; digits < 3; digits++){
        DIGIT [digits++] = SEGMENTS[0];
    }
}