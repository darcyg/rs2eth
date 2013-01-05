#include "config.h"

// wersja i struktura w pami�ci EEPROM
unsigned char config_ver __attribute__((section(".eeprom")));
config_t      config_eeprom __attribute__((section(".eeprom")));

// pobierz ustawienia z pami�ci EEPROM do kopii w pami�ci RAM
unsigned char config_read() {
    unsigned char ver;

    memset((void*) &config_ram, 0, sizeof(config_t));

    // por�wnaj wersj�
    eeprom_read_block((void*) &ver, &config_ver, 1);

    if (ver != CONFIG_VERSION) {
        // pobierz ustawienia domy�lne i zapisz je do EEPROMu
        config_set_default();
        config_save();
        return 0;
    }

    // pobierz ustawienia
    eeprom_read_block((void*) &config_ram, &config_eeprom, sizeof(config_t));

    return 1;
}

// zwr�� wska�nik do kopii ustawie� w pami�ci RAM
config_t* config_get() {
    return &config_ram;
}

// zapisz ustawienia do pami�ci EEPROM z kopii w pami�ci RAM
void config_save() {
    unsigned char ver = CONFIG_VERSION;

    // zapisz wersj� ustawie�
    eeprom_write_block((void*) &ver, &config_ver, 1);

    // zapisz ustawienia
    eeprom_write_block((void*) &config_ram, &config_eeprom, sizeof(config_t));
}

// ustaw domy�lne warto�ci
void config_set_default() {

    // IP
    uip_ipaddr(config_ram.ip,   192, 168, 1, 12);
    uip_ipaddr(config_ram.gate, 192, 168, 1, 1);
    uip_ipaddr(config_ram.mask, 255, 255, 255, 0);

    // przypisania port�w
    config_ram.telnet_port = 23;

    // nazwa
    strcpy_P(config_ram.name, PSTR("rs2eth"));

    // tryb pracy
    config_ram.mode = MODE_SERVER;
    uip_ipaddr(config_ram.server_ip, 192, 168, 1, 20);
    config_ram.server_port = 23;

    // RS: 115kbps 8N1
    config_ram.rs_baud = 115200UL;
    config_ram.rs_data_bits = 8;
    config_ram.rs_parity_bits = 0;
    config_ram.rs_stop_bits = 1;

}
