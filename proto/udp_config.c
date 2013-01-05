#include "udp_config.h"

volatile unsigned char udp_identify_timer;

// obs�u� pakiet
void udp_config_handle_packet(unsigned char* data, unsigned int len) {

    // otrzymany pakiet
    udp_config_packet *packet = (udp_config_packet*) data;

    // ignoruj niepoprawne pakiety
    if (packet->start != UDP_CONFIG_MAGIC) {
        return;
    }

    // ignoruj pakiety z ustawionym adresem MAC innego urz�dzenia
    if (memcmp(packet->mac, nic_get_mac(), 6) != 0) {
        // MAC niezgodny, sprawd� czy nie ustawiono adresu broadcast MAC (ff:ff:ff:ff:ff:ff)
        for (i=0; i<6; i++) {
            if (packet->mac[i] != 0xFF) {
                return;
            }
        }
    }

    switch(packet->type) {

        // wyszukiwanie urz�dze�
        case UDP_CONFIG_TYPE_DISCOVERY:
            // wyszukiwanie okre�lonego typu urz�dze�
            if ( (packet->length == 1) && (packet->data[0] != UDP_CONFIG_DEVICE_TYPE) ) {
                // szukany inny typ urz�dzenia
                return;
            }

            // formuj pakiet
            packet->type = UDP_CONFIG_TYPE_MY_CONFIG;
            len = udp_config_fill(packet->data);
            
            break;

        // identyfikacja wybranego urz�dzenia
        case UDP_CONFIG_TYPE_IDENTIFY:
            // mrugaj naprzemiennie diodami RX/TX przez 3 sekundy
            udp_identify_timer = 30;

            packet->type = UDP_CONFIG_TYPE_IDENTIFY_OK;
            len = 0;
            break;

        // niepoprawny typ
        default:
            return;
    }

    // ode�lij pakiet
    struct uip_udp_conn* conn;
    conn = uip_udp_new(&uip_udp_conn->ripaddr, uip_udp_conn->rport);

    if (!conn) {
        return;
    }

    // wy�lij z portu, na kt�rym pakiet zosta� odebrany
    uip_udp_bind(conn, HTONS(UDP_CONFIG_PORT));

    // nag��wek    
    packet->start = UDP_CONFIG_MAGIC;
    packet->length = len;

    // nadawca
    memcpy(packet->mac, nic_get_mac(), 6);

    // wy�lij
    uip_udp_send(len + 10);

    // czekaj na wys�anie
    nic_wait_for_send();

    // zamknij po��czenia UDP (przychodz�cy broadcast i wychodz�cy unicast)
    uip_udp_remove(conn);
    uip_udp_remove(uip_udp_conn);
}

// wy�lij pakiet powitalny (inicjalizacja "po��czenia" UDP)
void udp_config_init_hello_packet() {

    // adres broadcast
    uip_ipaddr_t addr;
    uip_ipaddr(addr, 255,255,255,255);

    // utw�rz "po��czenie" UDP
    struct uip_udp_conn* conn;

    conn = uip_udp_new(&addr, HTONS(UDP_CONFIG_PORT));

    if (!conn) {
        return;
    }

    // wy�lij z portu konfiguracyjnego
    uip_udp_bind(conn, HTONS(UDP_CONFIG_HELLO_PORT));

    // @see http://www.embeddedrelated.com/groups/lpc2000/show/26150.php
}

// wy�lij pakiet powitalny (broadcast z aktualnymi ustawieniami)
void udp_config_send_hello_packet(unsigned char* data) {

    // pakiet
    udp_config_packet *packet = (udp_config_packet*) data;

    // nag��wek + dane
    packet->start  = UDP_CONFIG_MAGIC;
    packet->type   = UDP_CONFIG_TYPE_HELLO;
    memcpy(packet->mac, nic_get_mac(), 6);
    packet->length = udp_config_fill(packet->data);
   
    // wy�lij
    uip_udp_send(packet->length + 10);
    nic_wait_for_send();

    // zamknij aktualne "po��czenie" UDP
    uip_udp_remove(uip_udp_conn);
}

// wype�nij podan� pami�� danymi konfiguracyjnymi
unsigned int udp_config_fill(unsigned char* data) {

    // struktury danych konfiguracyjnych
    udp_config_info       *info   = (udp_config_info*) data;
    udp_config_info_extra *extra  = (udp_config_info_extra*) (info->extra);

    // konfiguracja
    struct rs_current_setup_t *rs_setup = rs_get_setup();
    config_t* conf = config_get();

    // informacje o sprz�cie
    info->type  = UDP_CONFIG_DEVICE_TYPE;

    // id i flagi informacyjne
    info->flags = server_state;

    // nazwa
    memcpy(info->name, conf->name, 16);

    // IP
    info->eth_hardware = UDP_CONFIG_ETH_HARDWARE;
    uip_gethostaddr( ((u16_t*) &(info->ip)) );
    uip_getdraddr( ((u16_t*) &(info->gate)) );
    uip_getnetmask( ((u16_t*) &(info->mask)) );

    // wersja oprogramowania (0.9)
    info->rev_major = 0;
    info->rev_minor = 9;
    
    //
    // extra
    //

    // tryb pracy
    extra->mode = conf->mode;
    memcpy(extra->server_ip, conf->server_ip, 4);

    // ustawienia COM
    extra->rs_baud = rs_setup->baud;
    extra->rs_data_bits = rs_setup->data_bits;
    extra->rs_parity_bits = rs_setup->parity_bits;
    extra->rs_stop_bits = rs_setup->stop_bits;

    // zwr�� rozmiar danych konfiguracyjnych
    return UDP_CONFIG_INFO_SIZE + UDP_CONFIG_INFO_EXTRA_SIZE;
}
