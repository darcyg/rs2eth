#include "spi.h"

void spi_master_init(void)
{
	// ustaw kierunki dla pinow SPI
	DDR(SPI_PORT) |= (1<<SPI_SCK_PIN) | (1<<SPI_MOSI_PIN) | (1<<SPI_SS_PIN);   // wyj�cia (MOSI,SCK,SS)

    // zerowanie rejestr�w	
    SPCR = SPSR = 0;

    // taktowanie SPI na f_osc/2
    spi_full_speed();

    // w��cz SPI jako uk�ad master
    SPCR |= (1<<SPE)|(1<<MSTR);
}

void spi_slave_init(void)
{
	// ustaw kierunki dla pinow SPI
	DDR(SPI_PORT) |=  (1<<SPI_MISO_PIN); //wyj�ciem MISO
	
    SPCR = (1<<SPE);  // w��cz SPI jako uk�ad slave
}

unsigned char spi_read(void)
{
    SPDR = 0xff; // dummy

	// oczekiwanie na zakonczenie transmisji
	while(!(SPSR&=~(1<<SPIF)));

    //rs_send('<'); rs_hex(SPDR); rs_newline();

    // odczytaj stan rejestru
    return SPDR;
}

void spi_write(unsigned char data)
{
	SPDR = data;
    
	// oczekiwanie na zakonczenie transmisji
	while(!(SPSR&=~(1<<SPIF)));

    data = SPDR; // dummy
}
