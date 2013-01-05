#include "announce.h"

// obs�u� pakiet
void announce_handle_packet(unsigned char* data, unsigned int len) {
    // wype�nij pakiet (nazwa urz�dzenia, adres MAC, sygnaturka)
    len = 0;

    // skonfigurowana nazwa urz�dzenia
    config_t* config_get();
    
    strcpy(config_ram.name, (char*)data+len);
    len += strlen((char*)data);

    // �amanie wiersza
    memcpy_P(PSTR("\r\n"), data+len, 2);
    len += 2;

    // adres MAC (w formacie "human-readable")
    unsigned char *mac = nic_get_mac();
    for (unsigned char i=0; i<6; i++) {
        data[len++] = dec2hex(mac[i] >> 4);
        data[len++] = dec2hex(mac[i] & 0x0f);

        if (i<5)
            data[len++] = '-';
    }

    // �amanie wiersza
    memcpy_P(PSTR("\r\n"), data+len, 2);
    len += 2;
    
    // sygnaturka
    strcpy_P(RS2ETH_SIGNATURE, (char*)data+len);
    len += strlen_P(RS2ETH_SIGNATURE);

    rs_dump(data, len);

    // utw�rz "po��czenie" UDP (odpowied�)
    struct uip_udp_conn* conn;
    conn = uip_udp_new(&uip_udp_conn->ripaddr, uip_udp_conn->rport);

    if (!conn) {
        return;
    }

    // wy�lij z portu konfiguracyjnego
    uip_udp_bind(conn, HTONS(ANNOUNCE_PORT));

    // wy�lij
    uip_udp_send(len + 10);

    // czekaj na wys�anie
    nic_wait_for_send();

    // zamknij po��czenia UDP (przychodz�cy broadcast i wychodz�cy unicast)
    uip_udp_remove(conn);
    uip_udp_remove(uip_udp_conn);
}
