#ifndef __ANNOUNCE_H
#define __ANNOUNCE_H

#include "../rs_eth.h"

// port UDP pakiet�w MCHP discovery
#define ANNOUNCE_PORT   30303

// obs�u� pakiet MCHP discovery
void announce_handle_packet(unsigned char*, unsigned int);


#endif
