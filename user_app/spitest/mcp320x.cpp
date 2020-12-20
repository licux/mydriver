#include "mcp320x.h"

MCP320X::MCP320X(int bus, int cs){
    try{
        this->spi = new SPI(bus, cs, SPI_CPOL | SPI_CPHA);
    }catch(SPIException e){
        throw e;
    }
    this->ref = 3.30; // 3.3V
}

void MCP320X::setReference(double r){
    this->ref = r;
}

double MCP320X::getValue(int channel){
    try{
        return this->getValue(channel, MCP320X_SINGLE);
    }catch(SPIException e){
        throw e;
    }
}

double MCP320X::getValue(int channel, int s_d){
    uint8_t c;
    uint8_t *rxbuf;
    int d = 0;

    // Initialize packet
    this->packet[0] = this->packet[1] = this->packet[2] = 0;

    // set start bit
    this->packet[0] |= HIGH << 2;

    // Single-ended, differential
    c = s_d & 0x01;
    this->packet[0] |= c << 1;

    // set Analogue channel
    c = channel & 0x07;

    // the LSB of 1st bit is MSB of channel (=D2)
    this->packet[0] |= (c & 0x04) >> 2;
    // the MSB of 2nd bit is LSB of channle (=D1, D0)
    this->packet[1] |= (c & 0x03) << 6;

    try{
        rxbuf = this->spi->transfer(MCP320X_PACKET_SIZE, this->packet);
    }catch(SPIException e){
        throw e;
    }

    // Read value is the last 4 bit of 2nd bit and 3rd bit
    d = (rxbuf[1] & 0x0F) << 8;
    d |= rxbuf[2];
    delete(rxbuf);

    return (double)((d * ref) / MCP320X_RESOLUTION);
}

MCP320X::~MCP320X(void){
    delete(spi);
}