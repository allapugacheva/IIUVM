#include "stdafx.h"
#include <conio.h>
#include <stdio.h>
#include "hexioctrl.h"

const int DR[2] = {0x1F0, 0x170};
const int DH[2] = {0x1F6, 0x176};
const int CR[2] = {0x1F7, 0x177};

unsigned short data[256];

bool waitReady(int channelNum);
bool getDeviceInfo(int devNum, int channelNum);
void showInfo();

int main() {

	ALLOW_IO_OPERATIONS;
	for( int channel = 0; channel <= 1; channel++ )
		for( int device = 0; device <= 1; device++ )		
			if(getDeviceInfo(device, channel))
				showInfo();
	system("pause");
	return 0;
}

bool waitReady(int channel) {

	for (int i = 0; i < 1000; i++) {

		unsigned char state = _inp(CR[channel]);
		if (state & (1 << 6)) 
			return true; 
	}
	return false;
}

bool getDeviceInfo(int device, int channel) {

	while (_inp(CR[channel]) & (1 << 7));	

	_outp(DH[channel], (device << 4) + (7 << 5));
	if (!waitReady(channel))	
		return false;	

	_outp(CR[channel], 0xEC); 
	while (_inp(CR[channel]) & (1 << 7));

	if (!(_inp(CR[channel]) & (1 << 3))) {
		
		while (_inp(CR[channel]) & (1 << 7));	

		_outp(DH[channel], (device << 4) + (7 << 5));
		if (!waitReady(channel))	
			return false;

		_outp(CR[channel], 0xA1);
		while (_inp(CR[channel]) & (1 << 7));

		if (!(_inp(CR[channel]) & (1 << 3)))
			return false;
	}

	for( int i = 0; i < 256; i++ )
		data[i] = _inpw(DR[channel]);
	
	return true;
}

void showInfo() {

	printf("%-15s", "Model: ");
	for(int i = 27; i <= 46; i++)
		printf("%c%c", data[i] >> 8, data[i] & 0xFF );
	printf("\n");

	printf("%-15s", "Serial number: ");
	for( int i = 10; i <= 19; i++ )
		printf("%c%c", data[i] >> 8, data[i] & 0xFF );
	printf("\n");

	printf("%-15s", "Firmware: ");
	for( int i = 23; i <= 26; i++ )
		printf("%c%c", data[i] >> 8, data[i] & 0xFF );
	printf("\n");

	if (data[83] & 1 << 9)
		printf("%-15s%llu bytes\n", "Size: ", (((long long)data[103] << 48) | ((long long)data[102] << 32) | ((long long)data[101] << 16) | ((long long)data[100])) * 512);
	else
		printf("%-15s%llu bytes\n", "Size: ", (((long long)data[61] << 16) | ((long long)data[60])) * 512);

	printf("%-15s", "Interface: ");
	if(data[0] & (1 << 15))	
		printf("ATAPI\n");
	else 
		printf("ATA\n");
		
	printf("%-15s%s", "Multiword DMA: ", "modes ");
	for(int i = 0; i <= 2; i++)
		if(data[63] & (1 << i))
			printf("%d ", i);
	printf("\n");

	printf("%-15s%s", "Ultra DMA: ", "modes ");
	for(int i = 0; i <= 6; i++)
		if (data[88] & (1 << i))
			printf("%d ", i); 
	printf("\n");

	printf("%-15s%s", "PIO: ", "modes ");
	for(int i = 0; i <= 1; i++)
		if (data[64] & (1 << i))
			printf("%d ", i + 3);
	printf("\n");
}