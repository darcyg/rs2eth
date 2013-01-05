#include "vcom.h"

// aktualny stan po��czenia
unsigned char vcom_state = VCOM_STATE_NOT_CONNECTED;

// ramka startowa: 01 02 0B 00 E1 00 08 00 00 01 FF 
void vcom_handle_packet(unsigned char* data, unsigned int len) {

    //rs_send('>'); rs_dump(data, len);

    // sprawd� bajty start / stop
    if ( (data[0] != VCOM_SOF) || (data[len-1] != VCOM_EOF) ) {
        return vcom_send_error(SERV_ERROR_UNKNOWN_FATAL);
    }

    // sprawd� d�ugo�� pakietu
    if ( (data[2] != len) || (len < VCOM_MINIMAL_FRAMELENGTH) || (len > VCOM_MAX_PAYLOAD_LENGTH + 4) ) {
        return vcom_send_error(SERV_ERROR_INVALID_FRAME_LENGTH_RECEIVED);
    }

    // detekcja typu otrzymanej ramki
    switch(data[1]) {

        // ��danie otwarcia portu COM + dane konfiguracyjne
        case VCOM_OPEN_FRAME:
            if (vcom_state == VCOM_STATE_NOT_CONNECTED) {
                // pobierz dane konfiguracyjne i skonfiguruj port COM
                vcom_setup_com(data+3);

                // jeste�my po��czeni
                vcom_state = VCOM_STATE_CONNECTED;

                // ode�lij potwiedzenie po��czenia z portem COM
                return vcom_send_packet(VCOM_ISOPEN_FRAME, 0, 0);
            }
            else {
                // ��danie otwarcia otwartego portu
                return vcom_send_error(SERV_ERROR_WRONG_STATE);
            }
            break;

        // dane do wys�ania na port
        case VCOM_DATA_FRAME:
            if (vcom_state == VCOM_STATE_CONNECTED) {
                LED_ON(LED_TX);

                // wy�lij dane na COM
                rs_send(data[3]);
                return;
            }
            else {
                return vcom_send_error(SERV_ERROR_WRONG_STATE);
            }
            break;

        // zamkni�cie portu COM
        case VCOM_CLOSE_FRAME:
            if (vcom_state == VCOM_STATE_CONNECTED) {
                vcom_state = VCOM_STATE_NOT_CONNECTED;

                // ode�lij potwiedzenie po��czenia z portem COM
                return vcom_send_packet(VCOM_ISCLOSED_FRAME, 0, 0);
            }
            else {
                // ��danie zamkni�cie zamkni�tego portu
                return vcom_send_error(SERV_ERROR_WRONG_STATE);
            }
            break;
    }

    // z�y typ ramki
    return vcom_send_error(SERV_ERROR_INVALID_FRAMETYPE_RECEIVED);
}


// wy�lij pakiet z informacj� o b��dzie
void vcom_send_error(unsigned char err) {
    return vcom_send_packet(VCOM_ERROR_FRAME, &err , 1);
}

// wy�lij pakiet o podanym typie z podanymi danymi
void vcom_send_packet(unsigned char type, unsigned char* data, unsigned char len) {

    unsigned char *appdata = (unsigned char*) uip_appdata;

    // nag��wek ramki
    appdata[0] = VCOM_SOF;
    appdata[1] = type;
    appdata[2] = len + 4;

    // dodaj dane z tablicy data
    for (unsigned char n=0; n<len; n++) {
        appdata[n+3] = data[n];
    }

    // zamknij ramk�
    appdata[3+len] = VCOM_EOF;

    //rs_send('<'); rs_dump(uip_appdata, len+4);

    // wy�lij
    uip_send(uip_appdata, len + 4);
}

// konfiguruj port COM wg podanych danych z ramki vCom
// 0B 00 E1 00 08 00 00 01
void vcom_setup_com(unsigned char* data) {
    unsigned long baud;

    //rs_dump(data, 7);

    // pobierz pr�dko�� (3 bajty) 
    // 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200
    baud = data[0];

    baud <<= 8;
    baud |= data[1];

    baud <<= 8;
    baud |= data[2];

    //rs_long(baud); rs_newline();

    // bity danych (5, 6, 7, 8)
    // data[3]

    // bity stopu (0:1, 1:1.5, 2:2)
    // data[4]

    // bity kontroli parzysto�ci (0: none, 1:odd, 2:even, 3:mark, 4:space)
    // data[5]

    // ignoruj DSR (0/1)
    // data[6]

    // dostosuj warto�ci do zgodnych z RFC 2217
    switch(data[4]) {
        case 0:
            data[4] = 1; break;
        case 1:
            data[4] = 3; break;
        //case 2: // bez zmian
    }

    data[5]++;

    rs_setup(baud, data[3] /* data */, data[5] /* parity */, data[4] /* stop */);
}

// okresowo sprawdzaj czy nie nadesz�y nowe dane na porcie COM
void vcom_periodic() {

    unsigned char* buf = uip_appdata;
    unsigned int len = 0;

    // wy�lij dane z bufora odbiorczego
    if (!com_buf_is_empty(&com_buffer)) {
        while ( !com_buf_is_empty(&com_buffer) && (len < 1450) ) {
            buf[len++] = com_buf_get(&com_buffer);
        }

        vcom_send_packet(VCOM_DATA_FRAME, buf, len);
    }
}
