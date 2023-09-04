#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

#include "libusb.h"

#define VENDOR_ID 105
#define PRODUCT_ID 4096

#define ACM_CTRL_DTR 0x01
#define ACM_CTRL_RTS 0x02

static struct libusb_device_handle *devh = NULL;

static int ep_in_addr = 0x81;
static int ep_out_addr = 0x01;

std::mutex mtx;
const unsigned int size = 1023;
uint8_t txbuf[size];
uint8_t rxbuf[size];

std::atomic<bool> sent;

typedef union CANFrame
{
	struct
	{
		uint32_t canId;
		uint8_t length;
		uint8_t payload[64];
	} s;
	uint8_t data[sizeof(s)];
} CANFrame;

void Delay(double seconds)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);

	double interval = static_cast<double>(frequency.QuadPart) * seconds;

	LARGE_INTEGER current;
	do
	{
		QueryPerformanceCounter(&current);
	} while (static_cast<double>(current.QuadPart - start.QuadPart) < interval);
}

void writeData()
{
	std::cout << "CREATED TX" << std::endl;
	uint8_t localrx[size];
	uint8_t localtx[size];

	uint32_t every = 0;

	CANFrame canFrame;
	canFrame.s.canId = 0x664;
	canFrame.s.length = 8;
	uint8_t payload[]{0x40, 0xC5, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};
	memcpy(canFrame.s.payload, payload, canFrame.s.length);

	while (1)
	{
		// std::this_thread::sleep_for(std::chrono::milliseconds(10));
		Delay(0.002);
		/* create tx data */
		if (every % 10 == 0)
		{
			every = 0;
			*(CANFrame *)&localtx[0] = canFrame;

			std::unique_lock<std::mutex> lock(mtx);
			std::memcpy(txbuf, localtx, size - 1);
			lock.unlock();
			sent.store(false);
		}

		std::unique_lock<std::mutex> lock(mtx);
		std::memcpy(localrx, rxbuf, size - 1);
		memset(rxbuf, 0, sizeof(rxbuf));
		lock.unlock();

		every++;

		/* use rx data */
		uint32_t k = 0;
		while (k < sizeof(localrx) && localrx[k] != 0x00)
		{
			for (int i = 0; i < 10; i++)
				std::cout << std::hex << " 0x" << (int)localrx[k + i] << " ";
			std::cout << "\r\n";
			localrx[k] = 0;
			k += 69;
		}

		if (k)
			std::cout << "------------------" << std::endl;
	}
}

void readData()
{
	std::cout << "CREATED RX" << std::endl;
	uint8_t rx[size];
	uint8_t tx[size];

	while (1)
	{
		int receivedLen = 0;
		int actual_length = 0;

		std::unique_lock<std::mutex> lock(mtx);
		std::memcpy(tx, txbuf, size - 1);
		memset(txbuf, 0, sizeof(txbuf));
		lock.unlock();

		if (int ret = libusb_bulk_transfer(devh, ep_out_addr, tx, size - 1, &actual_length, 10) < 0)
			fprintf(stderr, "Error while sending %d \n", ret);
		else
			sent.store(true);

		if (int ret = libusb_bulk_transfer(devh, ep_in_addr, rx, size, &receivedLen, 1000) < 0)
			fprintf(stderr, "Error while receiving %d  transferred %d\n", ret, receivedLen);
		else
		{
			std::unique_lock<std::mutex> lock(mtx);
			std::memcpy(rxbuf, rx, receivedLen);
			lock.unlock();
			memset(rx, 0, sizeof(rx));
		}
	}
}

int main(int argc, char **argv)
{
	int rc;
	rc = libusb_init(NULL);
	if (rc < 0)
	{
		fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(rc));
		exit(1);
	}

	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if (!devh)
	{
		fprintf(stderr, "Error finding USB device\n");
		return -1;
	}

	for (int if_num = 0; if_num < 2; if_num++)
	{
		if (libusb_kernel_driver_active(devh, if_num))
			libusb_detach_kernel_driver(devh, if_num);

		rc = libusb_claim_interface(devh, if_num);

		if (rc < 0)
			fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(rc));
	}

	std::thread TX(writeData);
	std::thread RX(readData);

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	TX.join();
	RX.join();

	libusb_release_interface(devh, 0);
	if (devh)
		libusb_close(devh);
	libusb_exit(NULL);
	return rc;
}
