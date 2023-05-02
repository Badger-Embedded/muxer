/* simple.c

   Simple libftdi usage example

   This program is distributed under the GPL, version 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>
#include <popt.h>

#define DELAY_100MS     100000

int parseArguments(int argc, const char **argv) {
    int c;
    char *serial = NULL;

    poptContext optCon;
    struct poptOption optionsTable[] = {
            // Commands
            { "list", 'l', POPT_ARG_NONE, NULL, 'l', "lists all Badgerd USBMUX devices connected to PC", NULL },
            { "info", 'i', POPT_ARG_NONE, NULL, 'i', "displays info about device", "serial number" },
            { "show-serial", 'o', POPT_ARG_NONE, NULL, 'o', "displays serial number of given device", NULL },
            { "set-serial", 'r', POPT_ARG_STRING, &serial, 'r', "writes serial number to given device", NULL },
            { "init", 't', POPT_ARG_NONE, NULL, 't', "initialize target board", NULL },
            { "dut", 'd', POPT_ARG_NONE, NULL, 'd', "connects USB to the target board", NULL },
            { "ts", 's', POPT_ARG_NONE, NULL, 's', "connects USB to the test server", NULL },
            { "status", 'u', POPT_ARG_NONE, NULL, 'u', "show current status: DUT or TS or NOINIT", NULL },
            // { "dyper1", 'y', POPT_ARG_STRING, &options[CCO_DyPer].args, 'y', "Connect or disconnect terminals of 1st dynamic jumper; STRING = \"on\" or \"off\"", NULL },
            // { "dyper2", 'z', POPT_ARG_STRING, &options[CCO_DyPer].args, 'z', "Connect or disconnect terminals of 2nd dynamic jumper; STRING = \"on\" or \"off\"", NULL },
            // // Options
            // { "tick-time", 'm', POPT_ARG_INT, &options[CCO_TickTime].argn, 'm', "set time delay for 'tick' command",
            //         NULL },
            // { "device-id", 'v', POPT_ARG_INT, &options[CCO_DeviceId].argn, 'v', "use device with given id", NULL },
            // { "device-serial", 'e', POPT_ARG_STRING, &options[CCO_DeviceSerial].args, 'e',
            //         "use device with given serial number", NULL },
            // { "device-type", 'k', POPT_ARG_STRING, &options[CCO_DeviceType].args, 'k',
            //         "make the device of this type", NULL },
            // { "vendor", 'x', POPT_ARG_INT, &options[CCO_Vendor].argn, 'x', "use device with given vendor id", NULL },
            // { "product", 'a', POPT_ARG_INT, &options[CCO_Product].argn, 'a', "use device with given product id", NULL },
            // { "invert", 'n', POPT_ARG_NONE, NULL, 'n', "invert bits for --pins command", NULL },
            POPT_AUTOHELP
            { NULL, 0, 0, NULL, 0, NULL, NULL }
    };

    optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "command");
    if (argc < 2) {
        poptPrintUsage(optCon, stderr, 0);
        poptFreeContext(optCon);
        return EXIT_SUCCESS;
    }
    /* Now do options processing, get portname */
    // while ((c = poptGetNextOpt(optCon)) >= 0) {
    //     switch (c) {
    //         case 'l':
    //             *cmd = CCC_List;
    //             break;
    //         case 'i':
    //             *cmd = CCC_Info;
    //             break;
    //         case 'o':
    //             *cmd = CCC_ShowSerial;
    //             break;
    //         case 'r':
    //             *cmd = CCC_SetSerial;
    //             break;
    //         case 't':
    //             *cmd = CCC_Init;
    //             break;
    //         case 'd':
    //             *cmd = CCC_DUT;
    //             break;
    //         case 's':
    //             *cmd = CCC_TS;
    //             break;
    //         case 'p':
    //             *cmd = CCC_Pins;
    //             break;
    //         case 'c':
    //             *cmd = CCC_Tick;
    //             break;
    //         case 'u':
    //             *cmd = CCC_Status;
    //             break;
    //         case 'y':
    //             *cmd = CCC_DyPer1;
    //             break;
    //         case 'z':
    //             *cmd = CCC_DyPer2;
    //             break;
    //         case 'n':
    //             options[CCO_BitsInvert].argn = 1;
    //             break;
    //     }
    // }

    // if (serial)
    //     snprintf(args, argsLen, "%s", serial);
    // free(serial);

    // if (c < -1) {
    //     fprintf(stderr, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));
    //     poptFreeContext(optCon);
    //     return EXIT_FAILURE;
    // }

    poptFreeContext(optCon);

    return EXIT_SUCCESS;
}


int main(int argc, const char **argv)
{
    int ret;
    struct ftdi_context *ftdi;
    struct ftdi_version_info version;
    unsigned char buf[1];
    unsigned char pins;
    unsigned char pinState = 0xF1;

    if (parseArguments(argc, argv) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

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
