#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <libusb-1.0/libusb.h>

static const int INTERFACE_NUMBER = 0;
static const unsigned int TIMEOUT_MS = 5 * 1000;

void delay (float seconds)
  {
    struct timespec requested;
    struct timespec remaining;

    requested.tv_sec  = (int) seconds;
    requested.tv_nsec = (seconds - (int) seconds) * 1E9;
    nanosleep (&requested, &remaining);

    return;
  }

int interact (struct libusb_device_handle *handle, unsigned char *command, size_t length)
  {
    static const int INTERRUPT_IN_ENDPOINT = 0x81;
    static const int INTERRUPT_OUT_ENDPOINT = 0x01;

    int result;
    int count;
    int i = 0;

    unsigned char output[8];

    result = libusb_interrupt_transfer (handle,
        INTERRUPT_OUT_ENDPOINT,
        command,
        length,
        &count,
        TIMEOUT_MS);
    if (result < 0)
      {
        std::cout << "error - failed to send command to the device" << std::endl;
      }

    std::cout << "send:";
    for (i = 0; i < count - 1; i++)
      {
        std::cout << " " << (unsigned int) command[i];
      }
    std::cout << std::endl;

    result = libusb_interrupt_transfer (handle,
        INTERRUPT_IN_ENDPOINT,
        output,
        sizeof (output),
        &count,
        TIMEOUT_MS);
    if (result < 0)
      {
        std::cout << "error - failed to receive data from the device" << std::endl;
      }

    std::cout << "recv:";
    for (i = 0; i < count; i++)
      {
        std::cout << " " << (unsigned int) output[i];
      }
    std::cout << std::endl;

    return (0);
  }

int main (void)
  {
	static const int VENDOR_ID = 0xa0a0;
	static const int PRODUCT_ID = 0x000a;

    libusb_context *ctx = NULL;
	int result;

	result = libusb_init (&ctx);
    if (result < 0)
      {
        std::cout << "error - failed to initialize libusb" << std::endl;
        return (result);
      }

	int device_ready = 0;
	struct libusb_device_handle *handle = libusb_open_device_with_vid_pid (NULL, VENDOR_ID, PRODUCT_ID);
    if (handle != NULL)
      {
        libusb_detach_kernel_driver (handle, INTERFACE_NUMBER);
          {
            result = libusb_claim_interface(handle, INTERFACE_NUMBER);
            if (result >= 0)
              {
                device_ready = 1;
              }
            else
              {
                std::cout << "error - libusb_claim_interface returned " << result << std::endl;
              }
          }
      }
    else
      {
        std::cout << "error - failed to open the device" << std::endl;
        return (1);
      }


    if (device_ready)
      {
        //libusb_set_debug (ctx, 255);
 
        unsigned char command_00[] =        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_01_on_0[] =   {0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};
        unsigned char command_01_on_1[] =   {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_01_on_2[] =   {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_01_on_3[] =   {0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_01_on_4[] =   {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        unsigned char command_01_off[] =    {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_02[] =        {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned char command_03[] =        {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        interact (handle, command_00, sizeof (command_00));
        interact (handle, command_03, sizeof (command_00));

        for (int loop = 0; loop < 32; loop++)
          {
            interact (handle, command_01_on_1, sizeof (command_01_on_1));
            interact (handle, command_02, sizeof (command_02));
            delay (0.05);

            interact (handle, command_01_on_2, sizeof (command_01_on_2));
            interact (handle, command_02, sizeof (command_02));
            delay (0.05);

            interact (handle, command_01_on_3, sizeof (command_01_on_3));
            interact (handle, command_02, sizeof (command_02));
            delay (0.05);

            interact (handle, command_01_on_4, sizeof (command_01_on_4));
            interact (handle, command_02, sizeof (command_02));
            delay (0.05);
          }

        interact (handle, command_01_on_0, sizeof (command_01_on_0));
        interact (handle, command_02, sizeof (command_02));

        interact (handle, command_03, sizeof (command_00));

        //libusb_set_debug (ctx, 3);

        libusb_release_interface (handle, 0);
        libusb_attach_kernel_driver (handle, INTERFACE_NUMBER);
      }


    libusb_close (handle);
    libusb_exit (NULL);

    return (0);
  }
