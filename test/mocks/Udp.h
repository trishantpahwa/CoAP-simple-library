// Minimal Udp.h mock for host-side unit testing of coap-simple.
// Declares the abstract UDP interface coap-simple talks to. The concrete
// test double lives in test/FakeUDP.h.
#ifndef __UDP_MOCK_H__
#define __UDP_MOCK_H__

#include "Arduino.h"

class UDP
{
public:
    virtual ~UDP() {}

    virtual uint8_t begin(uint16_t port) = 0;
    virtual int beginPacket(IPAddress ip, uint16_t port) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    virtual int endPacket() = 0;
    virtual int parsePacket() = 0;
    virtual int read(unsigned char *buffer, size_t len) = 0;
    virtual IPAddress remoteIP() = 0;
    virtual uint16_t remotePort() = 0;
};

#endif
