#pragma once

#define IOCTL_PCI_READ_CONFIG CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
 
typedef struct _PCI_DEVICE_INFO {
    unsigned short vendorId;
    unsigned short deviceId;
} PCI_DEVICE_INFO;

typedef struct _PCI_DEVICES {
    PCI_DEVICE_INFO devices[256];
    unsigned int count;
} PCI_DEVICES;
