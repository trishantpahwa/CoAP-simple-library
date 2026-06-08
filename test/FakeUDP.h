// Test double for the Arduino UDP interface.
//
// FakeUDP records every datagram the library "sends" (so tests can inspect the
// exact bytes on the wire) and lets tests queue up datagrams to be "received"
// by Coap::loop().
#ifndef __FAKE_UDP_H__
#define __FAKE_UDP_H__

#include "Udp.h"
#include <vector>

struct SentDatagram
{
    IPAddress ip;
    uint16_t port;
    std::vector<uint8_t> data;
};

struct IncomingDatagram
{
    std::vector<uint8_t> data;
    IPAddress ip;
    uint16_t port;
};

class FakeUDP : public UDP
{
public:
    // Captured outgoing datagrams (one entry per endPacket()).
    std::vector<SentDatagram> sent;

    // Queue of datagrams that loop() will read via parsePacket()/read().
    std::vector<IncomingDatagram> incoming;

    uint16_t begin_port = 0;
    bool begin_called = false;

    // -- Outgoing path -----------------------------------------------------
    uint8_t begin(uint16_t port) override
    {
        begin_called = true;
        begin_port = port;
        return 1;
    }

    int beginPacket(IPAddress ip, uint16_t port) override
    {
        _pending_ip = ip;
        _pending_port = port;
        _pending_data.clear();
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        for (size_t i = 0; i < size; i++)
            _pending_data.push_back(buffer[i]);
        return size;
    }

    int endPacket() override
    {
        SentDatagram d;
        d.ip = _pending_ip;
        d.port = _pending_port;
        d.data = _pending_data;
        sent.push_back(d);
        return 1;
    }

    // -- Incoming path -----------------------------------------------------
    void pushIncoming(const std::vector<uint8_t> &data, IPAddress ip, uint16_t port)
    {
        IncomingDatagram d;
        d.data = data;
        d.ip = ip;
        d.port = port;
        incoming.push_back(d);
    }

    int parsePacket() override
    {
        if (_in_idx < incoming.size())
        {
            _current = incoming[_in_idx++];
            return (int)_current.data.size();
        }
        return 0;
    }

    int read(unsigned char *buffer, size_t len) override
    {
        size_t n = len < _current.data.size() ? len : _current.data.size();
        memcpy(buffer, _current.data.data(), n);
        return (int)n;
    }

    IPAddress remoteIP() override { return _current.ip; }
    uint16_t remotePort() override { return _current.port; }

    void reset()
    {
        sent.clear();
        incoming.clear();
        _in_idx = 0;
    }

private:
    IPAddress _pending_ip;
    uint16_t _pending_port = 0;
    std::vector<uint8_t> _pending_data;

    size_t _in_idx = 0;
    IncomingDatagram _current;
};

#endif
