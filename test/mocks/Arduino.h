// Minimal Arduino.h mock for host-side unit testing of coap-simple.
// Provides just enough of the Arduino core API (String, IPAddress, millis,
// and the standard C helpers the library relies on) to compile and run the
// library off-device.
#ifndef __ARDUINO_MOCK_H__
#define __ARDUINO_MOCK_H__

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>

// ---------------------------------------------------------------------------
// millis() — backed by a test-controllable clock.
// ---------------------------------------------------------------------------
extern unsigned long __mock_millis_value;
inline unsigned long millis() { return __mock_millis_value; }
inline void setMillis(unsigned long v) { __mock_millis_value = v; }

// ---------------------------------------------------------------------------
// String — thin wrapper around std::string covering the methods coap-simple
// uses (construction, assignment, +=, equals, length).
// ---------------------------------------------------------------------------
class String
{
public:
    std::string s;

    String() : s() {}
    String(const char *c) : s(c ? c : "") {}
    String(const String &o) : s(o.s) {}

    String &operator=(const char *c)
    {
        s = c ? c : "";
        return *this;
    }
    String &operator=(const String &o)
    {
        s = o.s;
        return *this;
    }

    String &operator+=(const char *c)
    {
        if (c)
            s += c;
        return *this;
    }
    String &operator+=(const String &o)
    {
        s += o.s;
        return *this;
    }

    bool equals(const String &o) const { return s == o.s; }
    bool equals(const char *c) const { return s == (c ? c : ""); }

    size_t length() const { return s.length(); }
    const char *c_str() const { return s.c_str(); }
};

// ---------------------------------------------------------------------------
// IPAddress — 4-octet IPv4 address with the [] and == operators the library
// depends on.
// ---------------------------------------------------------------------------
class IPAddress
{
public:
    uint8_t bytes[4];

    IPAddress() { bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0; }
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        bytes[0] = a;
        bytes[1] = b;
        bytes[2] = c;
        bytes[3] = d;
    }

    uint8_t operator[](int index) const { return bytes[index]; }
    uint8_t &operator[](int index) { return bytes[index]; }

    bool operator==(const IPAddress &o) const
    {
        return bytes[0] == o.bytes[0] && bytes[1] == o.bytes[1] &&
               bytes[2] == o.bytes[2] && bytes[3] == o.bytes[3];
    }
    bool operator!=(const IPAddress &o) const { return !(*this == o); }
};

#endif
