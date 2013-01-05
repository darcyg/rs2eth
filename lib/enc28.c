#include "enc28.h"

void enc28_init()
{
    // SS dla ENC jako wyj�cie...
    DDR(ENC28_SS_PORT) |= (1 << ENC28_SS_PIN);

    // ... i w stan wysoki
    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN);
   
    // jeszcze nie wybrano banku
    enc28_selected_bank = 0xff;

    // resetuj ENC28J60
    enc28_soft_reset();
}


void enc28_net_init(unsigned char *my_mac)
{
    enc28_select_bank(ENC28_ECON1);

    //
    // Bank 0
    //

	// inicjalizacja bufora odbiorczego / nadawczego
    
    // poczatek bufora odbiorczego
	enc28_next_packet_ptr = ENC28_RXSTART_INIT;
    
	enc28_write_reg(ENC28_ERXSTL, ENC28_RXSTART_INIT & 0xFF);
	enc28_write_reg(ENC28_ERXSTH, ENC28_RXSTART_INIT >> 8);

	// ustaw wskazniki bufora odbiorczego
	enc28_write_reg(ENC28_ERXRDPTL, ENC28_RXSTART_INIT & 0xFF);
	enc28_write_reg(ENC28_ERXRDPTH, ENC28_RXSTART_INIT >> 8);
	
    // koniec bufora odbiorczego
	enc28_write_reg(ENC28_ERXNDL, ENC28_RXSTOP_INIT & 0xFF);
	enc28_write_reg(ENC28_ERXNDH, ENC28_RXSTOP_INIT >> 8);
	
    // poczatek bufora nadawczego
	enc28_write_reg(ENC28_ETXSTL, ENC28_TXSTART_INIT & 0xFF);
	enc28_write_reg(ENC28_ETXSTH, ENC28_TXSTART_INIT >> 8);
	
    // koniec bufora nadawczego
	enc28_write_reg(ENC28_ETXNDL, ENC28_TXSTOP_INIT & 0xFF);
	enc28_write_reg(ENC28_ETXNDH, ENC28_TXSTOP_INIT >> 8);


    // 
	// Bank 1
    //

    // Przepuszczamy tylko broadcast ARP, inne tylko unicast (nasz mac)
    //
    // Filtr:
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum=f7f9
    // binarnie:11 0000 0011 1111
    // hex 303F->EPMM0=0x3f,EPMM1=0x30
	enc28_write_reg(ENC28_ERXFCON, (ENC28_ERXFCON_UCEN|ENC28_ERXFCON_CRCEN|ENC28_ERXFCON_PMEN));
	enc28_write_reg(ENC28_EPMM0, 0x3f);
	enc28_write_reg(ENC28_EPMM1, 0x30);
	enc28_write_reg(ENC28_EPMCSL, 0xf9);
	enc28_write_reg(ENC28_EPMCSH, 0xf7);


    //
    // Bank 2
    //

	// inicjalizuj warstwe odbiorcza ENC
	enc28_write_reg(ENC28_MACON1, (ENC28_MACON1_MARXEN|ENC28_MACON1_TXPAUS|ENC28_MACON1_RXPAUS));
	
    // reset jednostki MAC
    enc28_reg_clear_bit(ENC28_MACON2, ENC28_MACON2_MARST);

	// automatyczne wypelnienie wysy�anego pakietu do 60 bajtow + generowanie CRC
	enc28_reg_set_bit(ENC28_MACON3, (ENC28_MACON3_PADCFG0|ENC28_MACON3_TXCRCEN|ENC28_MACON3_FRMLNEN));
	
    //
    // full-duplex
    //
    enc28_write_phy(ENC28_PHCON1, ENC28_PHCON1_PDPXMD);

    // czy full-duplex dostepny?
    if (enc28_read_phy(ENC28_PHCON1) & ENC28_PHCON1_PDPXMD) {
        enc28_reg_set_bit(ENC28_MACON3, ENC28_MACON3_FULDPX);
        enc28_write_reg(ENC28_MABBIPG, 0x12);

        //my_net_config.using_fullduplex = 1;
    }
    else {
        enc28_reg_clear_bit(ENC28_MACON3, ENC28_MACON3_FULDPX);
        enc28_write_reg(ENC28_MABBIPG, 0x15);

        //my_net_config.using_fullduplex = 0;
    }

    // odstepy miedzy ramkami w buforach
	enc28_write_reg(ENC28_MAIPGL, 0x12);
	enc28_write_reg(ENC28_MAIPGH, 0x0C);

	// ustaw maksymalny rozmiar ramki, ktora przyjmiemy
	enc28_write_reg(ENC28_MAMXFLL, ENC28_MAX_FRAMELEN & 0xFF);	
	enc28_write_reg(ENC28_MAMXFLH, ENC28_MAX_FRAMELEN >> 8);


    //
	// Bank 3
    //

    // zapisz adres MAC
    enc28_write_reg(ENC28_MAADR5, my_mac[0]);
    enc28_write_reg(ENC28_MAADR4, my_mac[1]);
    enc28_write_reg(ENC28_MAADR3, my_mac[2]);
    enc28_write_reg(ENC28_MAADR2, my_mac[3]);
    enc28_write_reg(ENC28_MAADR1, my_mac[4]);
    enc28_write_reg(ENC28_MAADR0, my_mac[5]);

    // bez powrotu wyslanych ramek (no-loopback)
	enc28_write_phy(ENC28_PHCON2, ENC28_PHCON2_HDLDIS);

    // wybierz bank 0
	enc28_select_bank(ENC28_ECON1);

	// wlacz przerwania (p. 65)
	enc28_write_reg(ENC28_EIE, ENC28_EIE_INTIE|ENC28_EIE_PKTIE);
	
    // wlacz odbieranie pakietow
	enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_RXEN);

    // auto-inkrementacja
    enc28_reg_set_bit(ENC28_ECON2, ENC28_ECON2_AUTOINC);

    // Inicjalizacja diod LED w gniazdku RJ45 (p. 11)
    // LEDA - zolta      LEDB - zielona

    //enc28_setup_led(0b0111, 0b0100); // TX-RX / link
    enc28_setup_led(0b0001, 0b1100); // TX / link-RX
}


unsigned char enc28_read_opcode(unsigned char op, unsigned char address)
{
    spi_select(ENC28_SS_PORT, ENC28_SS_PIN); //_delay_us(15);
    
    spi_write((op) | ((address) & ENC28_MASK_ADDRESS));

    // dla rejestrow MAC i MII czekamy nieco dluzej na wynik
    if((address) & 0x80)
        spi_read();

    unsigned char tmp = spi_read();

    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN); //_delay_us(15);

    return tmp;
}

void enc28_write_opcode(unsigned char op, unsigned char address, unsigned char data)
{
    spi_select(ENC28_SS_PORT, ENC28_SS_PIN); //_delay_us(15);

    spi_write((op) | ((address) & ENC28_MASK_ADDRESS));
    spi_write(data);

    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN); //_delay_us(15);

    return;
}


void enc28_select_bank(unsigned char address)
{
    // ustaw bank, tylko jesli potrzeba
    if( ((address) & (ENC28_MASK_BANK)) != enc28_selected_bank) {
            // ustaw bank
            enc28_write_opcode(ENC28_OPCODE_BFC, ENC28_ECON1, (ENC28_ECON1_BSEL1|ENC28_ECON1_BSEL0));
            enc28_write_opcode(ENC28_OPCODE_BFS, ENC28_ECON1, (address & (ENC28_MASK_BANK))>>5);
            enc28_selected_bank = (address) & (ENC28_MASK_BANK);
    }
}

unsigned char enc28_read_reg(unsigned char address)
{
    // ustaw bank
    enc28_select_bank(address);
    // odczytaj z rejestru
    return enc28_read_opcode(ENC28_OPCODE_RCR, address);
}

void enc28_write_reg(unsigned char address, unsigned char data)
{
    // ustaw bank
    enc28_select_bank(address);
    // zapisz do rejestru
    enc28_write_opcode(ENC28_OPCODE_WCR, address, data); 
}

void enc28_read_buffer(unsigned char* data, unsigned int len)
{
    spi_select(ENC28_SS_PORT, ENC28_SS_PIN);

    // odczyt z pamieci
    spi_write(ENC28_OPCODE_RBM);

    while(len) {
        len--;
        // odczytaj dane
        *data = spi_read();
        data++;
    }

    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN);
}

void enc28_write_buffer(unsigned char* data, unsigned int len)
{
    spi_select(ENC28_SS_PORT, ENC28_SS_PIN);
    
    // zapis do pamieci
    spi_write(ENC28_OPCODE_WBM);
    
    while(len) {
        len--;
        // zapisz dane
        spi_write(*data);
        data++;
    }
    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN);
}

uint16_t enc28_read_phy(unsigned char address)
{
    uint16_t value = 0x0000;

    // ustaw adres czytanego rejestru PHY
    enc28_write_reg(ENC28_MIREGADR, address);

    // ustaw bit MICMD.MIIRD
    enc28_write_reg(ENC28_MICMD, ENC28_MICMD_MIIRD);

    // czekaj na zakonczenie operacji
    while(enc28_read_reg(ENC28_MISTAT) & ENC28_MISTAT_BUSY);
        
    // wyzeruj bit MICMD.MIIRD
    enc28_write_reg(ENC28_MICMD, 0x00);

    value  = enc28_read_reg(ENC28_MIRDL);
    value |= ((uint16_t) enc28_read_reg(ENC28_MIRDH)) << 8;

    return value;
}

void enc28_write_phy(unsigned char address, uint16_t data)
{
    // ustaw adres zapisywanego rejestru PHY
    enc28_write_reg(ENC28_MIREGADR, address);
    
    // zapisz dane do wybranego rejestru
    enc28_write_reg(ENC28_MIWRL, data);
    enc28_write_reg(ENC28_MIWRH, data>>8);
    
    // czekaj na zakonczenie operacji
    while(enc28_read_reg(ENC28_MISTAT) & ENC28_MISTAT_BUSY);
}

void enc28_packet_send(unsigned char* packet, unsigned int len)
{
    // czekaj na zako�czenie aktualnej operacji wysy�ania
    while (enc28_read_reg(ENC28_ECON1) & ENC28_ECON1_TXRTS);
 
    // zapisz pakiet do bufora nadawczego
	enc28_write_reg(ENC28_EWRPTL, ENC28_TXSTART_INIT & 0xFF);
	enc28_write_reg(ENC28_EWRPTH, ENC28_TXSTART_INIT >> 8);
	
    // koniec bufora nadawczego wraz z koncem pakietu
	enc28_write_reg(ENC28_ETXNDL, (ENC28_TXSTART_INIT+len) & 0xFF);
	enc28_write_reg(ENC28_ETXNDH, (ENC28_TXSTART_INIT+len) >> 8);

	// rozpocznij zapis do bufora ENC28
	enc28_write_opcode(ENC28_OPCODE_WBM, 0, 0x00);

	// zapisz pakiet do bufora nadawczego
	enc28_write_buffer(packet, len);

    // errata: reset logiki transmisji danych
	enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_TXRST);
	enc28_reg_clear_bit(ENC28_ECON1, ENC28_ECON1_TXRST);

	// wyslij pakiet po sieci
    enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_TXRTS);

#if ENC28_DEBUG
    rs_send('s'); rs_dump(packet, len);
#endif
}

void enc28_packet_send_double(unsigned char* packet1, unsigned int len1, unsigned char* packet2, unsigned int len2) {

    // czekaj na zako�czenie aktualnej operacji wysy�ania
	while (enc28_read_reg(ENC28_ECON1) & ENC28_ECON1_TXRTS);

    // errata: Transmit Logic reset
	enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_TXRST);
	enc28_reg_clear_bit(ENC28_ECON1, ENC28_ECON1_TXRST);
 
    // zapisz pakiet do bufora nadawczego
	enc28_write_reg(ENC28_EWRPTL, ENC28_TXSTART_INIT & 0xFF);
	enc28_write_reg(ENC28_EWRPTH, ENC28_TXSTART_INIT >> 8);
	
    // koniec bufora nadawczego wraz z koncem pakietu
	enc28_write_reg(ENC28_ETXNDL, (ENC28_TXSTART_INIT+len1+len2) & 0xFF);
	enc28_write_reg(ENC28_ETXNDH, (ENC28_TXSTART_INIT+len1+len2) >> 8);

	// bajt kontroli
	enc28_write_opcode(ENC28_OPCODE_WBM, 0, 0x00);

	// zapisz pakiet do bufora nadawczego
	enc28_write_buffer(packet1, len1);

    // zapisz te� drugi pakiet
    if (len2 > 0) {
        enc28_write_buffer(packet2, len2);
    }

    // errata: reset logiki transmisji danych
	enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_TXRST);
	enc28_reg_clear_bit(ENC28_ECON1, ENC28_ECON1_TXRST);

	// wyslij pakiet po sieci
    enc28_reg_set_bit(ENC28_ECON1, ENC28_ECON1_TXRTS);

#if ENC28_DEBUG
    rs_send('s'); rs_dump(packet1, len1);  rs_dump(packet2, len2);
#endif
}

unsigned int enc28_packet_recv(unsigned char* packet, unsigned int maxlen)
{
    uint16_t rxstat;
	uint16_t len;

    // czekaj na zako�czenie aktualnej operacji wysy�ania
	while (enc28_read_reg(ENC28_ECON1) & ENC28_ECON1_TXRTS);

	// ustaw bufor odczytu na poczatek pakieru w buforze
	enc28_write_reg(ENC28_ERDPTL, (enc28_next_packet_ptr));
	enc28_write_reg(ENC28_ERDPTH, (enc28_next_packet_ptr)>>8);

	// pobierz wska�nik do poczatku kolejnego pakietu
	enc28_next_packet_ptr  = enc28_read_opcode(ENC28_OPCODE_RBM, 0);
	enc28_next_packet_ptr |= enc28_read_opcode(ENC28_OPCODE_RBM, 0)<<8;

	// odczytaj rozmiar odebranego pakietu
	len  = enc28_read_opcode(ENC28_OPCODE_RBM, 0);
	len |= enc28_read_opcode(ENC28_OPCODE_RBM, 0)<<8;

    len-=4; // usun CRC

	// odczytaj status odebranego pakietu
	rxstat  = enc28_read_opcode(ENC28_OPCODE_RBM, 0);
	rxstat |= enc28_read_opcode(ENC28_OPCODE_RBM, 0)<<8;

	// ignoruj za du�e pakiety
    if (len>maxlen-1) {
        return 0;
    }

    // sprawd� CRC i inne bledy transmisji
    if ( (rxstat & 0x80) == 0 ){
        len = 0;
    } else {
        // skopiuj pakiet z pamieci ENC28 do pamieci uC
        enc28_read_buffer(packet, len);
    }

	// przesun wskaznik w buforze odbiorczym na nastepny pakiet
    // zwalniamy tym samym pamiec zajeta przez wlasnie odebrany pakiet
	enc28_write_reg(ENC28_ERXRDPTL, (enc28_next_packet_ptr));
	enc28_write_reg(ENC28_ERXRDPTH, (enc28_next_packet_ptr)>>8);

	// zmniejsz licznik odebranych pakietow o 1
    // zwalniamy tym samym flage przerwania od odebranego pakietu
	enc28_write_opcode(ENC28_OPCODE_BFS, ENC28_ECON2, ENC28_ECON2_PKTDEC);

#if ENC28_DEBUG
    rs_send('r'); rs_dump(packet, len);
#endif

    return len; 
}


unsigned char enc28_count_packets() {
    return enc28_read_reg(ENC28_EPKTCNT);
}


void enc28_soft_reset()
{
    spi_select(ENC28_SS_PORT, ENC28_SS_PIN); //_delay_us(15);
    
    // software-reset
    spi_write(ENC28_OPCODE_SRC);

    // czekaj na gotowo�� kontrolera
    while ( !(enc28_read_reg(ENC28_ESTAT) & ENC28_ESTAT_CLKRDY) );

    spi_unselect(ENC28_SS_PORT, ENC28_SS_PIN);// _delay_us(15);
    
    return;
}


unsigned char enc28_read_rev_id()
{
    // odczytaj tylko m�odsze 5 bitow
    return (enc28_read_reg(ENC28_EREVID) & 0x1F);
}

unsigned char enc28_is_link_up()
{
    return (enc28_read_phy(ENC28_PHSTAT1) & ENC28_PHSTAT1_LLSTAT) == ENC28_PHSTAT1_LLSTAT;
}

unsigned char enc28_is_jabbering() {
    return (enc28_read_phy(ENC28_PHSTAT1) & ENC28_PHSTAT1_JBSTAT) == ENC28_PHSTAT1_JBSTAT;
}

unsigned char enc28_is_full_duplex() {
    return (enc28_read_phy(ENC28_PHSTAT2) & ENC28_PHSTAT2_DPXSTAT) == ENC28_PHSTAT2_DPXSTAT;
}

unsigned char enc28_is_correct_polarity() {
    return (enc28_read_phy(ENC28_PHSTAT2) & ENC28_PHSTAT2_PLRITY) != ENC28_PHSTAT2_PLRITY;
}

void enc28_dump()
{
/*
    rs_newline();
    rs_text_P(PSTR("enc28j60 rev. B")); rs_send('0' +  enc28_read_rev_id()); rs_newline();

    rs_newline();
    rs_text_P(PSTR("Registers:")); rs_newline();
    rs_text_P(PSTR(" EREVID: ")); rs_hex(enc28_read_reg(ENC28_EREVID)); rs_newline();
    rs_text_P(PSTR(" EIE:    ")); rs_hex(enc28_read_reg(ENC28_EIE)); rs_newline();
    rs_text_P(PSTR(" EIR:    ")); rs_hex(enc28_read_reg(ENC28_EIR)); rs_newline();
    rs_text_P(PSTR(" ESTAT:  ")); rs_hex(enc28_read_reg(ENC28_ESTAT)); rs_newline();
    rs_text_P(PSTR(" ECON1:  ")); rs_hex(enc28_read_reg(ENC28_ECON1)); rs_newline();
    rs_text_P(PSTR(" ECON2:  ")); rs_hex(enc28_read_reg(ENC28_ECON2)); rs_newline();
    rs_text_P(PSTR(" MISTAT: ")); rs_hex(enc28_read_reg(ENC28_MISTAT)); rs_newline();
    rs_text_P(PSTR(" MACON1: ")); rs_hex(enc28_read_reg(ENC28_MACON1)); rs_newline();
    rs_text_P(PSTR(" MACON2: ")); rs_hex(enc28_read_reg(ENC28_MACON2)); rs_newline();
    rs_text_P(PSTR(" MACON3: ")); rs_hex(enc28_read_reg(ENC28_MACON3)); rs_newline();
    rs_text_P(PSTR(" MACON4: ")); rs_hex(enc28_read_reg(ENC28_MACON4)); rs_newline();

    rs_newline();
    rs_text_P(PSTR("PHY registers:")); rs_newline();
    rs_text_P(PSTR(" PHSTAT1:")); rs_hex(enc28_read_phy(ENC28_PHSTAT1) >> 8); rs_hex(enc28_read_phy(ENC28_PHSTAT1) & 0xff); rs_newline();
    rs_text_P(PSTR(" PHSTAT2:")); rs_hex(enc28_read_phy(ENC28_PHSTAT2) >> 8); rs_hex(enc28_read_phy(ENC28_PHSTAT2) & 0xff); rs_newline();
    rs_text_P(PSTR(" PHID1:  ")); rs_hex(enc28_read_phy(ENC28_PHHID1) >> 8);  rs_hex(enc28_read_phy(ENC28_PHHID1) & 0xff); rs_newline();
    rs_text_P(PSTR(" PHID2:  ")); rs_hex(enc28_read_phy(ENC28_PHHID2) >> 8);  rs_hex(enc28_read_phy(ENC28_PHHID2) & 0xff); rs_newline();
    rs_text_P(PSTR(" PHCON1: ")); rs_hex(enc28_read_phy(ENC28_PHCON1) >> 8);   rs_hex(enc28_read_phy(ENC28_PHCON1) & 0xff); rs_newline();
    rs_text_P(PSTR(" PHCON2: ")); rs_hex(enc28_read_phy(ENC28_PHCON2) >> 8);   rs_hex(enc28_read_phy(ENC28_PHCON2) & 0xff); rs_newline();

    rs_newline();
    rs_text_P(PSTR("Buffer:")); rs_newline();
    rs_text_P(PSTR(" RX start:")); rs_hex(enc28_read_reg(ENC28_ERXSTH));rs_hex(enc28_read_reg(ENC28_ERXSTL)); rs_newline();
    rs_text_P(PSTR(" RX end:  ")); rs_hex(enc28_read_reg(ENC28_ERXNDH));rs_hex(enc28_read_reg(ENC28_ERXNDL)); rs_newline();
    rs_text_P(PSTR(" TX start:")); rs_hex(enc28_read_reg(ENC28_ETXSTH));rs_hex(enc28_read_reg(ENC28_ETXSTL)); rs_newline();
    rs_text_P(PSTR(" TX end:  ")); rs_hex(enc28_read_reg(ENC28_ETXNDH));rs_hex(enc28_read_reg(ENC28_ETXNDL)); rs_newline();
    **/
}
