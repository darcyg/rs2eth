#ifndef _HTTP_PAGES_H
#define _HTTP_PAGES_H

#include "../../rs_eth.h"

// strona g��wna
unsigned int http_page_info(unsigned char*);

// ustawienia portu COM
unsigned int http_page_com_setup(unsigned char*);

// zapisz ustawienia portu COM
unsigned int http_post_com_setup(unsigned char*, unsigned char*);

// konfiguracja sieciowa
unsigned int http_page_ip_setup(unsigned char*);

// zapisz konfiguracj� sieciow�
unsigned int http_post_ip_setup(unsigned char*, unsigned char*);

// tryb pracy
unsigned int http_page_mode(unsigned char*);

// zapisz konfiguracj� trybu pracy
unsigned int http_post_mode(unsigned char*, unsigned char*);

// statystyki
unsigned int http_page_stats(unsigned char*);

// administracja
unsigned int http_page_admin(unsigned char*);

// zmiana nazwy urz�dzenia
unsigned int http_post_admin_name(unsigned char*, unsigned char*);

// zmiana numeru port�w
unsigned int http_post_admin_ports(unsigned char*, unsigned char*);

// zmiana loginu/has�a do panelu WWW
unsigned int http_post_admin_pass(unsigned char*, unsigned char*);

// uruchom ponownie
unsigned int http_page_reset(unsigned char*);

// wykonanie ponownego uruchomienia
unsigned int http_post_reset(unsigned char*);

// wykonanie powrotu do domy�lnych ustawie�
unsigned int http_post_default(unsigned char*);

// dokumentacja
unsigned int http_page_manual(unsigned char*);

// poka� temperatur� z czujnika DS18B20
unsigned int http_show_temperature(unsigned char*, unsigned char jsonify);

#endif
