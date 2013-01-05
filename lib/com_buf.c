#include "com_buf.h"

volatile com_buf_t com_buffer;

#ifdef RS_485
volatile unsigned char rs_rx_tx_timer;
#endif

// inicjalizacja bufora
void com_buf_init(volatile com_buf_t* buf) {
    buf->read_ptr = 0;
    buf->write_ptr = 0;
    buf->flags = 0;
    memset((void*) (buf->data), 0, COM_BUF_SIZE);
}

// bufor przepe�niony?
unsigned char com_buf_is_full(volatile com_buf_t* buf) {
    unsigned int tmp_ptr = buf->write_ptr;

    COM_BUF_PTR_INC(tmp_ptr);
    COM_BUF_PTR_INC(tmp_ptr);

    // wska�nik zapisu powinien by� "za" wska�nikiem odczytu
    return (tmp_ptr == buf->read_ptr) ? 1 : 0;
}

// bufor pusty?
unsigned char com_buf_is_empty(volatile com_buf_t* buf) {
    return (buf->write_ptr == buf->read_ptr) ? 1 : 0;
}

// zapisz do bufora
unsigned char com_buf_put(volatile com_buf_t* buf, unsigned char data) {

    #ifdef RS_485
        rs_rx_tx_timer = 0;
    #endif

    if (com_buf_is_full(buf)) {
        return 0;
    }
    
    // zapisz
    buf->data[buf->write_ptr] = data;

    // przesu� wska�nik
    COM_BUF_PTR_INC(buf->write_ptr);

    return 1;
}

// odczytaj z bufora
unsigned char com_buf_get(volatile com_buf_t* buf) {

    if (com_buf_is_empty(buf)) {
        return 0;
    }

    // odczytaj
    unsigned char data = buf->data[buf->read_ptr];

    // przesu� wska�nik
    COM_BUF_PTR_INC(buf->read_ptr);

    return data;
}

#ifdef RS_485
    // oblicz okres mi�dzy ramkami wg aktualnej pr�dko�ci transmisji (ms)
    unsigned char com_buf_get_frame_delay() {
        // pobierz aktualn� pr�dko�� RS'a
        struct rs_current_setup_t* rs_setup = rs_get_setup();

        switch(rs_setup->baud) {
            case 1200L:
                return 30; // ~30ms

            case 2400L:
                return 15;  // ~15ms

            case 4800L:
                return 7;  // ~7ms

            case 9600L:
                return 4;  // ~3,5ms

            case 14400L:
                return 2;   // ~2ms

            // >= 19200 bps
            default:
                return 2; // 1.750 ms
        }
    
    }
#endif
