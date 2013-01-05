#ifndef _UDP_CONFIG_H
#define _UDP_CONFIG_H

#include "../rs_eth.h"
#include <string.h>

// obs�u� pakiet
void udp_config_handle_packet(unsigned char*, unsigned int);

// wy�lij pakiet powitalny (inicjalizacja "po��czenia" UDP)
void udp_config_init_hello_packet();

// wy�lij pakiet powitalny (broadcast z aktualnymi ustawieniami)
void udp_config_send_hello_packet(unsigned char*);

// wype�nij podan� pami�� danymi konfiguracyjnymi
unsigned int udp_config_fill(unsigned char*);

//
// definicja og�lnego pakietu
//
typedef struct {
    // dwa bajty startu zgodne z UDP_CONFIG_MAGIC
    unsigned int  start;

    // typ pakietu
    unsigned char type;

    // adres MAC nadawcy (konwertera) lub celu (broadcast UDP do konkretnego konwertera)
    unsigned char mac[6];

    // d�ugo�� danych w tablicy data
    unsigned char length;

    // dane dodatkowe w pakiecie
    unsigned char data[];
} udp_config_packet;

//
// pakiet z informacj� o urz�dzeniu
//
typedef struct {

    // typ urz�dzenia
    unsigned char type;

    // nazwa (przypisana przez u�ytkownika)
    unsigned char name[16];

    // flagi
    unsigned char flags;

    // dane o warstwie sieciowej
    unsigned char eth_hardware;
    unsigned char ip[4];
    unsigned char gate[4];
    unsigned char mask[4];
    
    // oprogramowanie
    unsigned char rev_major;
    unsigned char rev_minor;

    // za struktur� mog� pojawi� si� dodatkowe dane
    // ...
    unsigned char extra[];
} udp_config_info;

//
// pole extra
//
typedef struct {
    // tryb pracy
    unsigned char mode;
    unsigned int server_ip[2];
    unsigned int server_port;

    // ustawienia COM
    unsigned long rs_baud;
    unsigned char rs_data_bits;
    unsigned char rs_parity_bits;
    unsigned char rs_stop_bits;
} udp_config_info_extra;

// rozmiary struktur konfiguracyjnych
#define UDP_CONFIG_INFO_SIZE        sizeof(udp_config_info)
#define UDP_CONFIG_INFO_EXTRA_SIZE  sizeof(udp_config_info_extra)

// port UDP
#define UDP_CONFIG_PORT             1638
#define UDP_CONFIG_HELLO_PORT       1639

// znacznik startu
#define UDP_CONFIG_MAGIC            0xFAFA

// typy pakiet�w
// < przychodz�ce       IN      -       0x01    0x03    0x20
// > wychodz�ce         OUT     0x02    0x10    0x04    0x21
#define UDP_CONFIG_TYPE_DISCOVERY       0x01    // < detekcja urz�dze� przez program zarz�dzaj�cy
#define UDP_CONFIG_TYPE_HELLO           0x02    // > pakiet wysy�any przez urz�dzenie po pod��czeniu do sieci
#define UDP_CONFIG_TYPE_IDENTIFY        0x03    // < program zarz�dzaj�cy prosi o identyfikacj� urz�dzenia - zamrugaj diodami statusowymi
#define UDP_CONFIG_TYPE_IDENTIFY_OK     0x04    // > odpowied� na ��danie identyfikacji urz�dzenia
#define UDP_CONFIG_TYPE_MY_CONFIG       0x10    // > odpowied� na detekcj� urz�dze�
#define UDP_CONFIG_TYPE_SET_CONFIG      0x20    // < ��danie zmiany konfiguracji
#define UDP_CONFIG_TYPE_SET_CONFIG_OK   0x21    // > informacja o powodzeniu zmiany konfiguracji

// typ aktualnego urz�dzenia
#define UDP_CONFIG_DEVICE_TYPE      0x01 // przej�ci�wka RS232-Ethernet

// typ sterownika sieci
#define UDP_CONFIG_ETH_HARDWARE     0x01 // ENC28

extern volatile unsigned char udp_identify_timer;

#endif
