// Small CoAP encode/decode helpers used only by the tests, so assertions can
// be written against decoded packet fields instead of raw byte offsets.
#ifndef __COAP_CODEC_H__
#define __COAP_CODEC_H__

#include <cstdint>
#include <vector>
#include <string>

struct DecodedOption
{
    uint16_t number;
    std::vector<uint8_t> value;

    std::string str() const { return std::string(value.begin(), value.end()); }
};

struct DecodedPacket
{
    bool valid = false;
    uint8_t version = 0;
    uint8_t type = 0;
    uint8_t tokenlen = 0;
    uint8_t code = 0;
    uint16_t messageid = 0;
    std::vector<uint8_t> token;
    std::vector<DecodedOption> options;
    std::vector<uint8_t> payload;

    std::string payloadStr() const { return std::string(payload.begin(), payload.end()); }

    // Returns the first option with the given number, or nullptr.
    const DecodedOption *option(uint16_t number) const
    {
        for (const auto &o : options)
            if (o.number == number)
                return &o;
        return nullptr;
    }

    int optionCount(uint16_t number) const
    {
        int n = 0;
        for (const auto &o : options)
            if (o.number == number)
                n++;
        return n;
    }

    std::string optionString(uint16_t number) const
    {
        const DecodedOption *o = option(number);
        if (!o)
            return "";
        return std::string(o->value.begin(), o->value.end());
    }
};

// Decode a raw CoAP datagram. On any malformed input, valid stays false.
inline DecodedPacket coapDecode(const std::vector<uint8_t> &buf)
{
    DecodedPacket p;
    if (buf.size() < 4)
        return p;

    p.version = (buf[0] & 0xC0) >> 6;
    p.type = (buf[0] & 0x30) >> 4;
    p.tokenlen = buf[0] & 0x0F;
    p.code = buf[1];
    p.messageid = ((uint16_t)buf[2] << 8) | buf[3];

    size_t pos = 4;
    if (p.tokenlen > 8 || pos + p.tokenlen > buf.size())
        return p;
    for (uint8_t i = 0; i < p.tokenlen; i++)
        p.token.push_back(buf[pos++]);

    uint16_t running = 0;
    while (pos < buf.size() && buf[pos] != 0xFF)
    {
        uint8_t b = buf[pos++];
        uint16_t delta = (b & 0xF0) >> 4;
        uint16_t len = b & 0x0F;

        if (delta == 13)
        {
            if (pos >= buf.size())
                return p;
            delta = buf[pos++] + 13;
        }
        else if (delta == 14)
        {
            if (pos + 1 >= buf.size())
                return p;
            delta = ((buf[pos] << 8) | buf[pos + 1]) + 269;
            pos += 2;
        }

        if (len == 13)
        {
            if (pos >= buf.size())
                return p;
            len = buf[pos++] + 13;
        }
        else if (len == 14)
        {
            if (pos + 1 >= buf.size())
                return p;
            len = ((buf[pos] << 8) | buf[pos + 1]) + 269;
            pos += 2;
        }

        if (pos + len > buf.size())
            return p;

        DecodedOption opt;
        opt.number = running + delta;
        for (uint16_t i = 0; i < len; i++)
            opt.value.push_back(buf[pos++]);
        running = opt.number;
        p.options.push_back(opt);
    }

    if (pos < buf.size() && buf[pos] == 0xFF)
    {
        pos++;
        for (; pos < buf.size(); pos++)
            p.payload.push_back(buf[pos]);
    }

    p.valid = true;
    return p;
}

// Append a single option (with extended delta/length encoding) to a buffer.
// Used by tests to craft incoming datagrams for Coap::loop().
inline void coapAppendOption(std::vector<uint8_t> &buf, uint16_t number,
                             uint16_t &running, const std::vector<uint8_t> &value)
{
    uint16_t delta = number - running;
    uint16_t len = (uint16_t)value.size();

    uint8_t deltaNibble, lenNibble;
    std::vector<uint8_t> ext;

    if (delta < 13)
        deltaNibble = (uint8_t)delta;
    else if (delta < 269)
    {
        deltaNibble = 13;
        ext.push_back((uint8_t)(delta - 13));
    }
    else
    {
        deltaNibble = 14;
        ext.push_back((uint8_t)((delta - 269) >> 8));
        ext.push_back((uint8_t)((delta - 269) & 0xFF));
    }

    std::vector<uint8_t> lenExt;
    if (len < 13)
        lenNibble = (uint8_t)len;
    else if (len < 269)
    {
        lenNibble = 13;
        lenExt.push_back((uint8_t)(len - 13));
    }
    else
    {
        lenNibble = 14;
        lenExt.push_back((uint8_t)((len - 269) >> 8));
        lenExt.push_back((uint8_t)((len - 269) & 0xFF));
    }

    buf.push_back((deltaNibble << 4) | lenNibble);
    for (uint8_t b : ext)
        buf.push_back(b);
    for (uint8_t b : lenExt)
        buf.push_back(b);
    for (uint8_t b : value)
        buf.push_back(b);

    running = number;
}

// Build a minimal CoAP request datagram with the given URI path segments.
inline std::vector<uint8_t> coapBuildRequest(uint8_t type, uint8_t code,
                                             uint16_t messageid,
                                             const std::vector<std::string> &pathSegments,
                                             const std::vector<uint8_t> &token = {},
                                             const std::string &payload = "",
                                             bool observe = false,
                                             uint8_t observeValue = 0)
{
    std::vector<uint8_t> buf;
    buf.push_back((0x01 << 6) | ((type & 0x03) << 4) | (token.size() & 0x0F));
    buf.push_back(code);
    buf.push_back((messageid >> 8) & 0xFF);
    buf.push_back(messageid & 0xFF);
    for (uint8_t b : token)
        buf.push_back(b);

    uint16_t running = 0;
    if (observe)
    {
        std::vector<uint8_t> ov;
        if (observeValue != 0)
            ov.push_back(observeValue);
        coapAppendOption(buf, 6 /* COAP_OBSERVE */, running, ov);
    }
    for (const auto &seg : pathSegments)
    {
        std::vector<uint8_t> v(seg.begin(), seg.end());
        coapAppendOption(buf, 11 /* COAP_URI_PATH */, running, v);
    }

    if (!payload.empty())
    {
        buf.push_back(0xFF);
        for (char c : payload)
            buf.push_back((uint8_t)c);
    }
    return buf;
}

// Build a CoAP ACK datagram (the kind a server sends back to a client request),
// used to drive the client-side response callback path through Coap::loop().
inline std::vector<uint8_t> coapBuildAck(uint8_t code, uint16_t messageid,
                                         const std::string &payload = "",
                                         const std::vector<uint8_t> &token = {})
{
    std::vector<uint8_t> buf;
    buf.push_back((0x01 << 6) | ((2 /* COAP_ACK */ & 0x03) << 4) | (token.size() & 0x0F));
    buf.push_back(code);
    buf.push_back((messageid >> 8) & 0xFF);
    buf.push_back(messageid & 0xFF);
    for (uint8_t b : token)
        buf.push_back(b);
    if (!payload.empty())
    {
        buf.push_back(0xFF);
        for (char c : payload)
            buf.push_back((uint8_t)c);
    }
    return buf;
}

#endif
