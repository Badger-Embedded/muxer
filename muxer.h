#pragma once
#include <string>

#define MUX_NO_CONNECTION   0xF1
#define MUX_DUT_TO_HOST     0xF0
#define MUX_DUT_TO_DEVICE   0xF3
#define MUX_DEVICE_TO_HOST  0xF2

#define BIT_DUT_ID          (0x1u << 2)

enum class CLICommand {
  UNKNOWN = -1,
  NONE,
  LIST,
  SET_USB_ID_PIN,
  SHOW_SERIAL,
  SET_SERIAL,
  INIT,
  DUT,
  DEVICE,
  DUT_TO_DEVICE,
  STATUS
};

enum class CLIOptions {
  DEVICE_ID = 0,
  DEVICE_SERIAL,
  DEVICE_TYPE,
  VENDOR,
  PRODUCT,
  SET_SERIAL,
  USB_ID_PIN,
  CLI_OPTIONS_SIZE
};

struct CLIOption
{
  int argn = -1;
  char* args = NULL;
};
