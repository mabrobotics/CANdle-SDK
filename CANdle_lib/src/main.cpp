#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "libusb.h"

#define VENDOR_ID 105
#define PRODUCT_ID 4096

#define ACM_CTRL_DTR 0x01
#define ACM_CTRL_RTS 0x02

static struct libusb_device_handle *devh = NULL;

static int ep_in_addr = 0x81;
static int ep_out_addr = 0x01;

std::mutex mtx;
const unsigned int size = 64;
uint8_t txbuf[size];
uint8_t rxbuf[size];

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

	while (1)
	{
		/* create tx data */
		for (uint32_t i = 0; i < size - 1; i++)
			localtx[i] = i % UINT8_MAX;

		std::unique_lock<std::mutex> lock(mtx);
		std::memcpy(txbuf, localtx, size - 1);
		std::memcpy(localrx, rxbuf, size - 1);
		lock.unlock();

		/* use rx data */
		uint32_t sum = 0.0;

		for (uint32_t i = 0; i < size - 1; i++)
			sum += localrx[i];
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
		lock.unlock();

		if (libusb_bulk_transfer(devh, ep_out_addr, tx, size - 1, &actual_length, 10) < 0)
		{
			fprintf(stderr, "Error while sending\n");
		}

		if (int ret = libusb_bulk_transfer(devh, ep_in_addr, rx, size, &receivedLen, 1000) < 0)
		{
			fprintf(stderr, "Error while receiving %d\n", ret);
		}
		else
		{
			std::unique_lock<std::mutex> lock(mtx);
			std::memcpy(rxbuf, rx, size - 1);
			lock.unlock();
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
