#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>
#include <popt.h>
#include "muxer.h"

#define STRING_SIZE     128

#define DELAY_100MS     100000
#define FTDI_VENDOR     0x0403
#define FTDI_PRODUCT    0x6015

int closeDevice(struct ftdi_context* ftdi);
struct ftdi_context* openDevice(CLIOption* options);

CLICommand parseArguments(int argc, const char **argv, CLIOption* options) {
    int c;
    CLICommand cmd = CLICommand::UNKNOWN;
    poptContext optCon;
    struct poptOption optionsTable[] = {
            // Commands
            { "list", 'l', POPT_ARG_NONE, NULL, 'l', "lists all Badgerd USBMUX devices connected to PC", NULL },
            { "show-serial", 'o', POPT_ARG_NONE, NULL, 'o', "displays serial number of given device", NULL },
            { "set-serial", 'r', POPT_ARG_STRING, &options[static_cast<int>(CLIOptions::SET_SERIAL)].args, 'r', "writes serial number to given device", NULL },
            { "init", 'i', POPT_ARG_NONE, NULL, 'i', "initialize target board", NULL },
            { "dut", 'd', POPT_ARG_NONE, NULL, 'd', "connects USB to the target board", NULL },
            { "dut-device", 'u', POPT_ARG_NONE, NULL, 'u', "connects target board and host together", NULL },
            { "device", 't', POPT_ARG_NONE, NULL, 't', "connects host to the device", NULL },
            { "set-id-pin", 'b', POPT_ARG_INT, &options[static_cast<int>(CLIOptions::USB_ID_PIN)].argn, 'b', "writes USB ID pin of DUT port in USBMUX (input 0 or 1)", NULL },
            { "status", 's', POPT_ARG_NONE, NULL, 's', "show current status: DUT or DEVICE or NOCONNECTION or DUT_DEVICE", NULL },
            { "device-serial", 'e', POPT_ARG_STRING, &options[static_cast<int>(CLIOptions::DEVICE_SERIAL)].args, 'e',
                    "use device with given serial number", NULL },
            { "device-type", 'k', POPT_ARG_STRING, &options[static_cast<int>(CLIOptions::DEVICE_TYPE)].args, 'k',
                    "make the device of this type", NULL },
            { "vendor", 'x', POPT_ARG_INT, &options[static_cast<int>(CLIOptions::VENDOR)].argn, 'x', "use device with given vendor id", NULL },
            { "product", 'p', POPT_ARG_INT, &options[static_cast<int>(CLIOptions::PRODUCT)].argn, 'p', "use device with given product id", NULL },
            POPT_AUTOHELP
            { NULL, 0, 0, NULL, 0, NULL, NULL }
    };

    optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "command");
    if (argc < 2) {
        poptPrintUsage(optCon, stderr, 0);
        poptFreeContext(optCon);
        return CLICommand::NONE;
    }

    /* Now do options processing, get portname */
    while ((c = poptGetNextOpt(optCon)) >= 0) {
        switch (c) {
            case 'l':
                cmd = CLICommand::LIST;
                break;
            case 'o':
                cmd = CLICommand::SHOW_SERIAL;
                break;
            case 'r':
                cmd = CLICommand::SET_SERIAL;
                break;
            case 'i':
                cmd = CLICommand::INIT;
                break;
            case 'd':
                cmd = CLICommand::DUT;
                break;
            case 't':
                cmd = CLICommand::DEVICE;
                break;
            case 'u':
                cmd = CLICommand::DUT_TO_DEVICE;
                break;
            case 'b':
                cmd = CLICommand::SET_USB_ID_PIN;
                break;
            case 's':
                cmd = CLICommand::STATUS;
                break;
            default:
                cmd = CLICommand::UNKNOWN;
                break;
        }
    }

    if (c < -1) {
        fprintf(stderr, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));
        poptFreeContext(optCon);
        return CLICommand::UNKNOWN;
    }

    poptFreeContext(optCon);

    return cmd;
}

struct ftdi_context* openDevice(CLIOption* options)
{
    struct ftdi_context *ftdi = NULL;
    int ret = 0;

    if (options[static_cast<int>(CLIOptions::DEVICE_SERIAL)].args == NULL)
    {
        fprintf(stderr, "No serial number provided to open the device\n");
        return NULL;
    }

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return NULL;
    }

    ret = ftdi_usb_open_desc_index(ftdi,
            options[static_cast<int>(CLIOptions::VENDOR)].argn,
            options[static_cast<int>(CLIOptions::PRODUCT)].argn,
            NULL,
            options[static_cast<int>(CLIOptions::DEVICE_SERIAL)].args, 0);
    if (ret < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
    }

    ret = ftdi_read_eeprom(ftdi);
    if (ret < 0) {
        fprintf(stderr, "Unable to read ftdi eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
    }

    ret = ftdi_eeprom_decode(ftdi, 0);
    if (ret < 0) {
        fprintf(stderr, "Unable to decode ftdi eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
    }

    return ftdi;
}

int closeDevice(struct ftdi_context* ftdi)
{
    int ret = 0;
    if ((ret = ftdi_usb_close(ftdi)) < 0)
    {
        fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    ftdi_free(ftdi);

    return EXIT_SUCCESS;
}

int configureDevice(CLIOption* options)
{
    int ret = 0;
    struct ftdi_context *ftdi;

    ftdi = openDevice(options);

    ret = ftdi_eeprom_initdefaults(ftdi, "Badgerd Technologies", "USBMUX", options[static_cast<int>(CLIOptions::SET_SERIAL)].args);
    if (ret < 0) {
        fprintf(stderr, "Unable to init eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_0, CBUSH_IOMODE);
    if (ret < 0) {
        fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }
    ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_1, CBUSH_IOMODE);
    if (ret < 0) {
        fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }
    ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_2, CBUSH_IOMODE);
    if (ret < 0) {
        fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_3, CBUSH_IOMODE);
    if (ret < 0) {
        fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    ret = ftdi_eeprom_build(ftdi);
    if (ret < 0) {
        fprintf(stderr, "Unable to build eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    ret = ftdi_write_eeprom(ftdi);
    if (ret < 0) {
        fprintf(stderr, "Unable to write eeprom into device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    closeDevice(ftdi);
    return EXIT_SUCCESS;
}

int listDevices(CLIOption* options) {
    int fret = 0, i = 0;
    struct ftdi_context *ftdi = NULL;
    struct ftdi_device_list *devlist = NULL, *curdev = NULL;
    char manufacturer[STRING_SIZE + 1], description[STRING_SIZE + 1], serial[STRING_SIZE + 1];
    int retval = EXIT_SUCCESS;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    if ((fret = ftdi_usb_find_all(ftdi, &devlist,
                    options[static_cast<int>(CLIOptions::VENDOR)].argn,
                    options[static_cast<int>(CLIOptions::PRODUCT)].argn)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", fret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    printf("Number of FTDI devices found: %d\n", fret);

    for (curdev = devlist; curdev != NULL; i++)
    {
        if ((fret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, STRING_SIZE, description, STRING_SIZE,
                serial, STRING_SIZE)) < 0)
        {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", fret, ftdi_get_error_string(ftdi));
            retval = EXIT_FAILURE;
            break;
        }

        printf("Dev: %d, Manufacturer: %s, Serial: %s, Description: %s\n", i,
                manufacturer, serial, description);

        curdev = curdev->next;
    }

    ftdi_list_free(&devlist);
    ftdi_free(ftdi);

    return retval;
}

int switchConnection(CLICommand command, CLIOption* options)
{
    /**
     * CBUS0    CBUS1
     * 1        0       No Connection
     * 0        0       DUT to Host
     * 1        1       DUT to Device
     * 0        1       Device to Host
     *
     * CBUS2 is DUT ID
     * */
    struct ftdi_context *ftdi = NULL;
    int ret = EXIT_SUCCESS;
    int pinState = MUX_NO_CONNECTION;
    unsigned char pins = 0x0;

    ftdi = openDevice(options);
    if (ftdi == NULL)
    {
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    switch (command)
    {
    case CLICommand::DUT:
        pinState = MUX_DUT_TO_HOST;
        break;
    case CLICommand::DEVICE:
        pinState = MUX_DEVICE_TO_HOST;
        break;
    case CLICommand::DUT_TO_DEVICE:
        pinState = MUX_DUT_TO_DEVICE;
        break;
    default:
        fprintf(stderr, "Unkown switch command!\n");
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    if (ftdi_read_pins(ftdi, &pins) != 0) {
        fprintf(stderr, "Error reading pins state.\n");
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    printf("Pins: %x\n", pins);

    if ((pins & BIT_DUT_ID))
    {
        pinState |= BIT_DUT_ID;
    }
    else
    {
        pinState &= ~BIT_DUT_ID;
    }

    printf("Updated pins: %x\n", pinState);

    ftdi_set_bitmode(ftdi, pinState, BITMODE_CBUS);
    usleep(DELAY_100MS);
    closeDevice(ftdi);

    return ret;
}

int setDUTIDPin(CLIOption* options, bool value)
{
    /**
     * CBUS0    CBUS1
     * 1        0       No Connection
     * 0        0       DUT to Host
     * 1        1       DUT to Device
     * 0        1       Device to Host
     *
     * CBUS2 is DUT ID
     * */
    struct ftdi_context *ftdi = NULL;
    int ret = EXIT_SUCCESS;
    unsigned char pins = MUX_NO_CONNECTION;

    ftdi = openDevice(options);
    if (ftdi == NULL)
    {
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    if (ftdi_read_pins(ftdi, &pins) != 0) {
        fprintf(stderr, "Error reading pins state.\n");
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    printf("Pins: %x\n", pins);

    if (value)
    {
        pins |= BIT_DUT_ID;
    }
    else
    {
        pins &= ~BIT_DUT_ID;
    }

    printf("Updated pins: %x\n", pins);

    ftdi_set_bitmode(ftdi, pins, BITMODE_CBUS);
    usleep(DELAY_100MS);
    closeDevice(ftdi);

    return ret;
}

int printStatus(CLIOption* options)
{
    struct ftdi_context *ftdi = NULL;
    int ret = EXIT_SUCCESS;
    unsigned char pins = MUX_NO_CONNECTION;

    ftdi = openDevice(options);
    if (ftdi == NULL)
    {
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    if (ftdi_read_pins(ftdi, &pins) != 0) {
        fprintf(stderr, "Error reading pins state.\n");
        closeDevice(ftdi);
        return EXIT_FAILURE;
    }

    printf("DUT USB-ID State: %d and ", ( (pins & BIT_DUT_ID) && 1) );

    pins &= ~BIT_DUT_ID;

    if (pins == MUX_DUT_TO_DEVICE)
    {
        printf("DUT is connected to DEVICE [%d:%d]\n", ( (pins & BIT_DUT_ID) && 1), 0x1);
    }
    else if (pins == MUX_DUT_TO_HOST)
    {
        printf("DUT is connected to HOST [%d:%d]\n", ( (pins & BIT_DUT_ID) && 1), 0x2);
    }
    else if (pins == MUX_DEVICE_TO_HOST)
    {
        printf("DEVICE is connected to HOST [%d:%d]\n", ( (pins & BIT_DUT_ID) && 1), 0x3);
    }
    else if (pins == MUX_NO_CONNECTION)
    {
        printf("No connection between devices [%d:%d]\n", ( (pins & BIT_DUT_ID) && 1), 0x4);
    }
    else
    {
        printf("Pin state values are malformed [%d:%d]\n", ( (pins & BIT_DUT_ID) && 1), 0xff);
    }

    return ret;
}

int main(int argc, const char **argv)
{
    int ret;
    CLIOption options[static_cast<int>(CLIOptions::CLI_OPTIONS_SIZE)];

    struct ftdi_version_info version;
    unsigned char buf[1];
    unsigned char pins;
    unsigned char pinState = 0xF1;
    options[static_cast<int>(CLIOptions::VENDOR)].argn = FTDI_VENDOR;
    options[static_cast<int>(CLIOptions::PRODUCT)].argn = FTDI_PRODUCT;
    CLICommand command = parseArguments(argc, argv, options);

    if (command == CLICommand::UNKNOWN) {
        fprintf(stderr, "Unkown command!\n");
        return EXIT_FAILURE;
    }

    version = ftdi_get_library_version();
    printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
        version.version_str, version.major, version.minor, version.micro,
        version.snapshot_str);

    switch (command)
    {
    case CLICommand::LIST:
        return listDevices(options);
    case CLICommand::SET_SERIAL:
        return configureDevice(options);
    case CLICommand::SET_USB_ID_PIN:
        return setDUTIDPin(options, options[static_cast<int>(CLIOptions::USB_ID_PIN)].argn);
    case CLICommand::DUT:
    case CLICommand::DEVICE:
    case CLICommand::DUT_TO_DEVICE:
        return switchConnection(command, options);
    case CLICommand::STATUS:
        return printStatus(options);
    case CLICommand::UNKNOWN:
        fprintf(stderr, "Unknown command!\n");
        return EXIT_FAILURE;
    default:
        return EXIT_SUCCESS;
    }

    return ret;
}
