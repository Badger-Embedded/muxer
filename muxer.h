#pragma once
#include <string>

enum class CLICommand {
  UNKNOWN = -1,
  NONE,
  LIST,
  SET_USB_ID_PIN,
  SHOW_SERIAL,
  SET_SERIAL,
  INIT,
  DUT,
  TS,
  DUT_TO_TS,
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
