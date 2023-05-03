#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>
#include <popt.h>
#include "muxer.h"

#define DELAY_100MS     100000

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
            { "ts", 't', POPT_ARG_NONE, NULL, 't', "connects USB to the test server", NULL },
            { "set-id-pin", 'b', POPT_ARG_INT, &options[static_cast<int>(CLIOptions::USB_ID_PIN)].argn, 'b', "writes USB ID pin of DUT port in USBMUX", NULL },
            { "status", 's', POPT_ARG_NONE, NULL, 's', "show current status: DUT or TS or NOINIT", NULL },
            { "device-id", 'v', POPT_ARG_INT, &options[static_cast<int>(CLIOptions::DEVICE_ID)].argn, 'v', "use device with given id", NULL },
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
                cmd = CLICommand::TS;
                break;
            case 'u':
                cmd = CLICommand::DUT_TO_TS;
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


int main(int argc, const char **argv)
{
    int ret;
    CLIOption options[static_cast<int>(CLIOptions::CLI_OPTIONS_SIZE)];

    struct ftdi_context *ftdi;
    struct ftdi_version_info version;
    unsigned char buf[1];
    unsigned char pins;
    unsigned char pinState = 0xF1;
    CLICommand command = parseArguments(argc, argv, options);

    if (command == CLICommand::UNKNOWN) {
        fprintf(stderr, "Unkown command!\n");
        return EXIT_FAILURE;
    }

    switch (command)
    {
    case CLICommand::LIST:
        // TODO: Call list devices
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::SET_SERIAL:
        // TODO: set serial id of the device using options
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::SET_USB_ID_PIN:
        // TODO: set usb id of dut port using options
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::INIT:
        // TODO: init the device via description
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::SHOW_SERIAL:
        // TODO:
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::DUT:
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::TS:
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::DUT_TO_TS:
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    case CLICommand::STATUS:
        printf("Not implemented yet!");
        return EXIT_FAILURE;
    default:
        fprintf(stderr, "Unknown command!\n");
        return EXIT_FAILURE;
    }

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    version = ftdi_get_library_version();
    printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
        version.version_str, version.major, version.minor, version.micro,
        version.snapshot_str);

    if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6015)) < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    ret = ftdi_read_eeprom(ftdi);
    if (ret < 0) {
        fprintf(stderr, "Unable to read ftdi eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        goto error;
    }

    ret = ftdi_eeprom_decode(ftdi, 0);
    if (ret < 0) {
        fprintf(stderr, "Unable to decode ftdi eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        goto error;
    }

    // ret = ftdi_eeprom_initdefaults(ftdi, "Badgerd Technologies", "USBMUX", "bdgrd_usbmux_001");
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to init eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }

    // ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_0, CBUSH_IOMODE);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }
    // ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_1, CBUSH_IOMODE);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }
    // ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_2, CBUSH_IOMODE);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }

    // ret = ftdi_set_eeprom_value(ftdi, CBUS_FUNCTION_3, CBUSH_IOMODE);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to set eeprom strings: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }

    // ret = ftdi_eeprom_build(ftdi);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to build eeprom: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }

    // ret = ftdi_write_eeprom(ftdi);
    // if (ret < 0) {
    //     fprintf(stderr, "Unable to write eeprom into device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    //     goto error;
    // }

    /**
     * CBUS0    CBUS1
     * 1        0       No Connection
     * 0        0       DUT to Host
     * 1        1       DUT to Device
     * 0        1       Device to Host
     *
     * CBUS2 is DUT ID
     * */



    ftdi_set_bitmode(ftdi, pinState, BITMODE_CBUS);
    usleep(DELAY_100MS);

    if (ftdi_read_pins(ftdi, &pins) != 0) {
        fprintf(stderr, "Error reading pins state.\n");
        ret = EXIT_FAILURE;
        goto error;
    }

    printf("Pins: %x\n", pins);


    error:

    if ((ret = ftdi_usb_close(ftdi)) < 0)
    {
        fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    ftdi_free(ftdi);

    return EXIT_SUCCESS;
}
