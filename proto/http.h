#ifndef _HTTP_H
#define _HTTP_H

#include "../rs_eth.h"
#include "http/data.h"
#include "http/pages.h"

// obs�u� przychodz�ce ��dania do serwera
void http_handle_packet(unsigned char*, unsigned int);

// serwuj tre�� dynamiczn�
unsigned int http_serve_dynamic(unsigned char*, unsigned char);

// generuj menu
unsigned int http_serve_menu(unsigned char*, unsigned char);

// serwuj tre�� dynamiczn� generowan� jako odpowiedzi na ��dania POST
unsigned int http_serve_post(unsigned char*, unsigned char*, unsigned char*);

// serwuj tre�� statyczn�
unsigned int http_serve_static(unsigned char*, unsigned char*);

// sprawd� dane autoryzacyjne u�ytkownika
unsigned char http_authenticate_user(unsigned char*);

// wykonuj czynno�ci okresowe: zamykanie otwartych po��cze�, "dosy�anie" reszty danych (strony dynamiczne)
void http_periodic();

// kopiuje podany �a�cuch z pami�ci FLASH do pami�ci RAM oraz zwraca d�ugo�� kopiowanego �a�cucha
unsigned int http_copy_from_pstr(unsigned char*, PGM_VOID_P);

// kopiuje podany obszar z pami�ci FLASH do pami�ci RAM oraz zwraca d�ugo�� kopiowanego �a�cucha
unsigned int http_copy_from_flash(unsigned char*, PGM_VOID_P, unsigned int);

// wstawia podan� warto�� numeryczn� jako �a�cuch do pami�ci RAM oraz zwraca d�ugo�� wstawionego �a�cucha
unsigned int http_copy_from_val(unsigned char*, unsigned long);

// wstawia podany adres IP jako �a�cuch do pami�ci RAM oraz zwraca d�ugo�� wstawionego �a�cucha
unsigned int http_copy_from_ip(unsigned char*, uip_ipaddr_t*);

// kopiuje warto�� podanego parametru zapytania do bufora oraz zwraca jego d�ugo��
unsigned char http_get_query_param(unsigned char*, PGM_VOID_P, unsigned char*);

// parsuj podany �a�cuch z adresem IP
unsigned char http_parse_ip(unsigned char*, uip_ipaddr_t*);

// port serwera HTTP
#define HTTP_PORT 80

// typ stron dynamicznych (zak�adki menu)
#define HTTP_PAGE_INFO      1
#define HTTP_PAGE_COM_SETUP 2
#define HTTP_PAGE_IP_SETUP  3
#define HTTP_PAGE_MODE      4
#define HTTP_PAGE_STATS     5
#define HTTP_PAGE_ADMIN     6
#define HTTP_PAGE_RESET     7
#define HTTP_PAGE_MANUAL    8

// parsowanie zapyta�
#define HTTP_QUERY_IS(txt)      strcmp_P((char*)query, PSTR(txt)) == 0
#define HTTP_QUERY_BEGINS(txt)  strstr_P((char*)query, PSTR(txt)) == (char*) query

// nag��wki odpowiedzi serwera
extern const char HTTP_SERVER[] PROGMEM;
extern const char HTTP_AUTHENTICATE[] PROGMEM;

extern const char HTTP_CACHE[] PROGMEM;
extern const char HTTP_DONT_CACHE[] PROGMEM;

extern const char HTTP_200_BODY[] PROGMEM;
extern const char HTTP_200_HTML[] PROGMEM;
extern const char HTTP_200_PNG[] PROGMEM;
extern const char HTTP_200_GIF[] PROGMEM;
extern const char HTTP_200_CSS[] PROGMEM;
extern const char HTTP_200_JS[] PROGMEM;
extern const char HTTP_200_TEXT[] PROGMEM;

extern const char HTTP_301_REDIRECT[] PROGMEM;

extern const char HTTP_401_HEADER[] PROGMEM;
extern const char HTTP_401_BODY[] PROGMEM;
extern const char HTTP_404_HEADER[] PROGMEM;
extern const char HTTP_404_BODY[] PROGMEM;

extern const char HTTP_AUTH_STRING[] PROGMEM;

#endif
