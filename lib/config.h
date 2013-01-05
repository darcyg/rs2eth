#ifndef _CONFIG_H
#define _CONFIG_H

#include "../rs_eth.h"

#define CONFIG_VERSION   0x02

// struktura ustawie� w pami�ci EEPROM uC
typedef struct {

    // ustawienia IP
    unsigned int ip[2];
    unsigned int gate[2];
    unsigned int mask[2];
    unsigned char use_dhcp;

    // przypisania port�w
    unsigned int telnet_port;

    // przypisana przez u�ytkownika nazwa urz�dzenia
    char name[16];

    // tryb pracy
    //  0 - serwer
    //  1 - klient (auto-��czenie si� z podanym hostem)
    unsigned char mode; 
    unsigned int server_ip[2];
    unsigned int server_port;
    
    // ustawienia COM
    unsigned long rs_baud;
    unsigned char rs_data_bits;
    unsigned char rs_parity_bits;
    unsigned char rs_stop_bits;

    // dane logowania
    unsigned char login[9];
    unsigned char pass[9];
} config_t;

// wersja i struktura w pami�ci EEPROM
extern unsigned char config_ver __attribute__((section(".eeprom")));
extern config_t config_eeprom __attribute__((section(".eeprom")));

// struktura w pami�ci RAM
config_t config_ram;

// odczyt / pobranie / zapis konfiguracji
unsigned char config_read();
config_t*     config_get();
void          config_save();

// ustaw domy�lne warto�ci
void config_set_default();

#endif
