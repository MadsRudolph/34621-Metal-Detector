/**
 * power_test.c
 * Power consumption measurement with button-controlled test modes
 * Target: Arduino Mega 2560
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <math.h>

typedef enum {
    MODE_IDLE=0, MODE_TX_ONLY, MODE_TX_SAMPLE, MODE_TX_OLED, MODE_FULL, MODE_COUNT
} test_mode_t;

static const char* mode_names[] = {"IDLE","TX_ONLY","TX_SAMPLE","TX_OLED","FULL"};
volatile test_mode_t current_mode = MODE_FULL;
volatile uint8_t mode_changed = 1;

#define SAMPLE_BUFFER_SIZE 64
#define DISPLAY_UPDATE_MS 83
#define IIR_ALPHA 0.1f
#define OLED_ADDR 0x78

static volatile uint16_t buffer_a[SAMPLE_BUFFER_SIZE];
static volatile uint16_t buffer_b[SAMPLE_BUFFER_SIZE];
static volatile uint16_t *active_buffer = buffer_a;
static volatile uint16_t *ready_buffer = buffer_b;
static volatile uint8_t sample_index = 0;
static volatile uint8_t buffer_ready = 0;
static volatile float magnitude = 0.0f;
static volatile float phase_deg = 0.0f;
static volatile float filtered_magnitude = 0.0f;
static volatile float filtered_phase = 0.0f;
static volatile uint32_t millis_counter = 0;
static volatile uint32_t last_display_update = 0;

void power_save_init(void) {
    PRR0 |= (1<<PRADC)|(1<<PRUSART1)|(1<<PRUSART2)|(1<<PRUSART3);
    PRR1 |= (1<<PRTIM4)|(1<<PRTIM5);
}

void timer0_init(void) { TCCR0A=(1<<WGM01); TCCR0B=(1<<CS01)|(1<<CS00); OCR0A=249; TIMSK0=(1<<OCIE0A); }
ISR(TIMER0_COMPA_vect) { millis_counter++; }
uint32_t millis(void) { uint32_t ms; cli(); ms=millis_counter; sei(); return ms; }

void button_init(void) { DDRE&=~(1<<PE4); PORTE|=(1<<PE4); EICRB|=(1<<ISC41); EIMSK|=(1<<INT4); }
ISR(INT4_vect) {
    static uint32_t lp=0;
    if(millis_counter-lp>200) { current_mode=(current_mode+1)%MODE_COUNT; mode_changed=1; lp=millis_counter; }
}

void tx_init(void) { DDRB|=(1<<PB5); TCCR1A=(1<<COM1A0); TCCR1B=(1<<WGM12)|(1<<CS10); OCR1A=3999; }
void tx_start(void) { TCCR1A=(1<<COM1A0); TCCR1B=(1<<WGM12)|(1<<CS10); }
void tx_stop(void) { TCCR1A=0; TCCR1B=0; PORTB&=~(1<<PB5); }

void sampling_timer_init(void) { TCCR3A=0; TCCR3B=(1<<WGM32)|(1<<CS30); OCR3A=1999; TIMSK3=(1<<OCIE3A); }
void sampling_start(void) { TCCR3B=(1<<WGM32)|(1<<CS30); TIMSK3=(1<<OCIE3A); }
void sampling_stop(void) { TCCR3B=0; TIMSK3=0; }

ISR(TIMER3_COMPA_vect) {
    volatile uint8_t d=SPDR; d=SPDR; d=SPDR; (void)d;
    static uint8_t ph=0; const int16_t st[4]={2048,3048,2048,1048};
    active_buffer[sample_index++]=st[ph]; ph=(ph+1)&3;
    if(sample_index>=SAMPLE_BUFFER_SIZE) {
        sample_index=0; volatile uint16_t*t=active_buffer; active_buffer=ready_buffer; ready_buffer=t; buffer_ready=1;
    }
}

void dft_calculate(const volatile uint16_t*s,float*m,float*p) {
    int32_t r=0,i=0;
    for(uint8_t j=0;j<SAMPLE_BUFFER_SIZE;j+=4) { r+=s[j];r-=s[j+2];i+=s[j+1];i-=s[j+3]; }
    float rf=(float)r,imf=(float)i;
    *m=sqrtf(rf*rf+imf*imf); *p=atan2f(-imf,rf)*(180.0f/M_PI);
    if(*p<0)*p+=360.0f; *m=(*m/(SAMPLE_BUFFER_SIZE*4095.0f))*100.0f;
}
float iir_filter(float n,float o,float a){return a*n+(1.0f-a)*o;}

void i2c_init(void){TWSR=0;TWBR=12;TWCR=(1<<TWEN);}
void i2c_start(uint8_t a){TWCR=(1<<TWSTA)|(1<<TWEN)|(1<<TWINT);while(!(TWCR&(1<<TWINT)));TWDR=a;TWCR=(1<<TWEN)|(1<<TWINT);while(!(TWCR&(1<<TWINT)));}
void i2c_write(uint8_t d){TWDR=d;TWCR=(1<<TWEN)|(1<<TWINT);while(!(TWCR&(1<<TWINT)));}
void i2c_stop(void){TWCR=(1<<TWSTO)|(1<<TWEN)|(1<<TWINT);}

void oled_command(uint8_t c){i2c_start(OLED_ADDR);i2c_write(0);i2c_write(c);i2c_stop();}
void oled_on(void){oled_command(0xAF);}
void oled_off(void){oled_command(0xAE);}
void oled_init(void){_delay_ms(100);oled_command(0xAE);oled_command(0xD5);oled_command(0x80);oled_command(0xA8);oled_command(0x3F);oled_command(0xD3);oled_command(0);oled_command(0x40);oled_command(0x8D);oled_command(0x14);oled_command(0x20);oled_command(0);oled_command(0xA1);oled_command(0xC8);oled_command(0xDA);oled_command(0x12);oled_command(0x81);oled_command(0xCF);oled_command(0xD9);oled_command(0xF1);oled_command(0xDB);oled_command(0x40);oled_command(0xA4);oled_command(0xA6);oled_command(0xAF);}
void oled_set_position(uint8_t p,uint8_t c){oled_command(0xB0+p);oled_command(c&0xF);oled_command(0x10+((c>>4)&0xF));}
void oled_clear(void){for(uint8_t p=0;p<8;p++){oled_set_position(p,0);i2c_start(OLED_ADDR);i2c_write(0x40);for(uint8_t c=0;c<128;c++)i2c_write(0);i2c_stop();}}
void oled_update_display(float m,float p){for(uint8_t pg=0;pg<2;pg++){oled_set_position(pg,0);i2c_start(OLED_ADDR);i2c_write(0x40);uint8_t f=(uint8_t)(m*1.28f);for(uint8_t c=0;c<128;c++)i2c_write(c<f?0xFF:0);i2c_stop();}}

void spi_init(void){DDRB|=(1<<PB2)|(1<<PB1)|(1<<PB0);SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0);}

void uart_init(void){UBRR0H=0;UBRR0L=103;UCSR0B=(1<<TXEN0);UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);}
void uart_putchar(char c){while(!(UCSR0A&(1<<UDRE0)));UDR0=c;}
void uart_puts(const char*s){while(*s){if(*s==10)uart_putchar(13);uart_putchar(*s++);}}
void uart_print_int(int v){char b[12];char*p=b+11;*p=0;if(v==0)*--p=48;else{uint8_t n=0;if(v<0){n=1;v=-v;}while(v>0){*--p=48+(v%10);v/=10;}if(n)*--p=45;}uart_puts(p);}

void apply_mode(test_mode_t m){tx_stop();sampling_stop();oled_off();switch(m){case MODE_FULL:case MODE_TX_OLED:oled_on();oled_clear();case MODE_TX_SAMPLE:sampling_start();case MODE_TX_ONLY:tx_start();break;default:break;}}

void print_mode_status(test_mode_t m){
    uart_puts("\n[MODE ");uart_print_int(m);uart_puts("] ");uart_puts(mode_names[m]);
    uart_puts(": TX=");uart_puts(m>=MODE_TX_ONLY?"ON":"OFF");
    uart_puts(" SAMPLE=");uart_puts(m>=MODE_TX_SAMPLE?"ON":"OFF");
    uart_puts(" OLED=");uart_puts(m>=MODE_TX_OLED?"ON":"OFF");
    uart_puts(" DFT=");uart_puts(m>=MODE_FULL?"ON":"OFF");
    uart_puts("\n>> Measure current now\n\n");
}

int main(void){
    uart_init();uart_puts("\n\n=== Power Test ===\nPress button (Pin 2)\n\n");
    power_save_init();uart_puts("[OK] Power save\n");
    spi_init();uart_puts("[OK] SPI\n");
    i2c_init();uart_puts("[OK] I2C\n");
    oled_init();uart_puts("[OK] OLED\n");
    timer0_init();uart_puts("[OK] Timer0\n");
    button_init();uart_puts("[OK] Button\n");
    tx_init();sampling_timer_init();
    sei();apply_mode(current_mode);print_mode_status(current_mode);
    uint32_t lp=0,lc=0,ls=millis();
    while(1){
        if(mode_changed){mode_changed=0;apply_mode(current_mode);print_mode_status(current_mode);lc=0;ls=millis();}
        if(current_mode==MODE_FULL&&buffer_ready){buffer_ready=0;float nm,np;dft_calculate(ready_buffer,&nm,&np);filtered_magnitude=iir_filter(nm,filtered_magnitude,IIR_ALPHA);filtered_phase=iir_filter(np,filtered_phase,IIR_ALPHA);magnitude=filtered_magnitude;phase_deg=filtered_phase;lc++;}
        uint32_t now=millis();
        if(current_mode>=MODE_TX_OLED&&now-last_display_update>=DISPLAY_UPDATE_MS){last_display_update=now;oled_update_display(magnitude,phase_deg);}
        if(now-lp>=5000){uart_puts("Mode: ");uart_puts(mode_names[current_mode]);if(current_mode==MODE_FULL){uart_puts(" DFT:");uart_print_int((int)(lc*1000/(now-ls+1)));uart_puts("Hz");}uart_puts("\n");lp=now;lc=0;ls=now;}
    }
    return 0;
}
