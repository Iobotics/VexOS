//
//  SerialPort.c
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/13/2012.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
//

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct SerialPort {
    // device header //
    unsigned char     deviceId;
    DeviceType        type;
    String            name;
    // device item fields //
    UARTPort          port;
    BaudRate          baudRate;
    bool              hasOptions;
    SerialOptions     options;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

SerialPort* SerialPort_new(String name, UARTPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    SerialPort* ret = malloc(sizeof(SerialPort));
    ret->type       = DeviceType_SerialPort;
    ret->name       = name;
    ret->port       = port;
    ret->baudRate   = 0;
    ret->hasOptions = false;
    memset(&ret->options, 0, sizeof(SerialOptions));
    Device_addUART(port, (Device*) ret);
    return ret;
}

void SerialPort_open(SerialPort* serial, BaudRate baudRate) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(baudRate != BaudRate_300   && baudRate != BaudRate_600 
            && baudRate != BaudRate_1200  && baudRate != BaudRate_2400 
            && baudRate != BaudRate_4800  && baudRate != BaudRate_9600
            && baudRate != BaudRate_14400 && baudRate != BaudRate_19200
            && baudRate != BaudRate_28800 && baudRate != BaudRate_38400
            && baudRate != BaudRate_57600 && baudRate != BaudRate_115200,
            VEXOS_ARGINVALID, "Invalid Baud rate: %d", baudRate);

    serial->baudRate = baudRate;
    OpenSerialPort(serial->port, serial->baudRate);
}

void SerialPort_openWithOptions(SerialPort* serial, BaudRate baudRate, SerialOptions opts) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(baudRate != BaudRate_300   && baudRate != BaudRate_600 
            && baudRate != BaudRate_1200  && baudRate != BaudRate_2400 
            && baudRate != BaudRate_4800  && baudRate != BaudRate_9600
            && baudRate != BaudRate_14400 && baudRate != BaudRate_19200
            && baudRate != BaudRate_28800 && baudRate != BaudRate_38400
            && baudRate != BaudRate_57600 && baudRate != BaudRate_115200,
            VEXOS_ARGINVALID, "Invalid Baud rate: %d", baudRate);

    serial->baudRate   = baudRate;
    serial->hasOptions = true;
    serial->options    = opts;
    OpenSerialPortEx(serial->port, serial->baudRate, opts.dataBits, opts.stopBits,
            opts.parity, opts.flowControl, opts.mode);

}

BaudRate SerialPort_getBaudRate(SerialPort* serial) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!serial->baudRate, VEXOS_OPINVALID, "Serial port is not open: %s", serial->name); 

    return serial->baudRate;
}

SerialOptions SerialPort_getOptions(SerialPort* serial) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!serial->hasOptions, VEXOS_OPINVALID, 
               "Serial port not opened with options: %s", serial->name); 

    return serial->options;
}

unsigned char SerialPort_getByteCount(SerialPort* serial) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!serial->baudRate, VEXOS_OPINVALID, "Serial port is not open: %s", serial->name); 

    return GetSerialPortByteCount(serial->port);
}

unsigned char SerialPort_readByte(SerialPort* serial) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!serial->baudRate, VEXOS_OPINVALID, "Serial port is not open: %s", serial->name); 

    return ReadSerialPort(serial->port);
}

void SerialPort_writeByte(SerialPort* serial, unsigned char byte) {
    ErrorIf(serial == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!serial->baudRate, VEXOS_OPINVALID, "Serial port is not open: %s", serial->name); 

    WriteSerialPort(serial->port, byte);
}

