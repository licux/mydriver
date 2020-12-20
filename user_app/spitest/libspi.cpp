#include <stdio.h>
#include <sstream>
#include <exception>

#include "libspi.h"

#define SPIDEV_PREFIX "/dev/spidev"

SPI::SPI(int bus, int ce, uint8_t mode){
    std::stringstream devname;

    devname << SPIDEV_PREFIX << bus << "." << ce;

    this->fd = open(devname.str().c_str(), O_RDWR);
    if(this->fd < 0){
        throw SPIException(string("Cannot open spidev:") + devname.str());
    }

    if(ioctl(this->fd, SPI_IOC_WR_MODE, &mode) < 0){
        throw SPIException(string("SPI_IOC_WRMODE failed."));
    }

    this->bits_per_word = 8;
    this->cs_change = LOW;
    this->speed = 1000000;
    this->delay_usecs = 0;
}

void SPI::setSpeed(int s){
    this->speed = s;
}

void SPI::setCSActive(int a){
    this->cs_change = a;
}

void SPI::setBits(int b){
    this->bits_per_word = b;
}

void SPI::setDelay(int d){
    this->delay_usecs = d;
}

uint8_t *SPI::transfer(int len, uint8_t *txbuf){
    struct spi_ioc_transfer send;
    uint8_t *rxbuf = new uint8_t(len);

    send.tx_buf = (unsigned long)txbuf;
    send.rx_buf = (unsigned long)rxbuf;
    send.len = len;
    send.delay_usecs = this->delay_usecs;
    send.speed_hz = this->speed;
    send.bits_per_word = this->bits_per_word;
    send.cs_change = this->cs_change;

    int r = ioctl(this->fd, SPI_IOC_MESSAGE(1), &send);
    if(r < 1){
        delete(rxbuf);
        throw SPIException(string("SPI transfer error."));
    }
    return rxbuf;
}

SPI::~SPI(void){
    close(this->fd);
}