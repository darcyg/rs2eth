#ifndef _RS_H_
#define	_RS_H_

#include "../rs_eth.h"

#include <avr/pgmspace.h>

struct rs_current_setup_t {
    unsigned long baud;
    unsigned char data_bits;
    unsigned char parity_bits;
    unsigned char stop_bits;
};

// struktura z aktualn� konfiguracj� portu
struct rs_current_setup_t rs_current_setup;

#define RS_STATS    1

// okreslenie wartosci wpisu do UBBR
//#define USART_BAUDRATE(br) ((F_CPU/1000*125/br)-1)    // u2x=1
#define USART_BAUDRATE(br) ((F_CPU/1000*64/br)-1)  // u2x=0

// inicjalizacja UART z okreslona pr�dkoscia
void rs_init(unsigned long);

// ustawienia portu COM (warto�ci wg RFC 2217)
void rs_setup(unsigned long, unsigned char, unsigned char, unsigned char);

// pobierz ustawienia portu COM (warto�ci wg RFC 2217)
struct rs_current_setup_t *rs_get_setup();

// czy�ci bufor odbiorczy
void rs_flush();

// w��cza obs�ug� przerwania na przychodz�ce dane z USART
void rs_int_on_recv();

// funkcja obs�uguj�ca przychodz�ce dane z przerwania
void rs_on_recv(unsigned char);

// odbiera dane z bufora (procedura blokujaca!)
unsigned char rs_recv();

// wysy�a dane do bufora (procedura blokujaca!)
void rs_send(unsigned char);

// sprawdza czy w buforze znajduje sie nieodczytane odebrane dane (USART Receive Complete)
unsigned char rs_has_recv();

// sprawdza czy dane w buforze zosta�y wys�ane (USART Transmit Complete)
unsigned char rs_has_send();

// wysy�a �a�cuch tekstowy (procedura blokuj�ca!)
void rs_text(char[]);

// wysy�a �a�cuch tekstowy mieszcz�cy si� w pami�ci programu
void rs_text_P(PGM_P);

// zaczyna now� lini� (ci�g CRLF)
#define rs_newline()    rs_send(10);rs_send(13);

void rs_int(int);
void rs_long(unsigned long);
void rs_int2(unsigned char);

void rs_hex(unsigned char);

// zrzut pamieci spod podanego wskaznika
void rs_dump(unsigned char*, unsigned int);

// w��czenie nadajnika RS485
void rs_enable_transmit(unsigned char);

#ifdef RS_STATS
struct {
    unsigned long recv;
    unsigned long sent;
} rs_stats;
#endif

#endif
