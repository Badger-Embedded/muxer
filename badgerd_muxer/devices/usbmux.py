import logging
from pyftdi.ftdi import Ftdi
from pyftdi.eeprom import FtdiEeprom
from pyftdi.gpio import GpioAsyncController
from pyftdi.usbtools import UsbDeviceDescriptor
from enum import Enum


class USBMuxMode(Enum):
    DUT_TO_DEVICE = 0b0011
    HOST_TO_DEVICE = 0b0010
    HOST_TO_DUT = 0b0000
    NO_CONNECTION = 0b0001
    UNKNOWN = 0b1111


class USBMux(object):
    def __init__(self, serial: str, vid: int = 0x0403, pid: int = 0x6015):
        """
        URL scheme: ftdi://[vendor[:product[:index|:serial]]]/interface
        """
        self._vid = vid
        self._pid = pid
        self._serial = serial
        self._ftdi: Ftdi = Ftdi()
        self._ftdi.open_from_url(f"ftdi://{vid}:{pid}:{serial}/1", reset=False)
        self._eeprom = FtdiEeprom()
        self._eeprom.connect(self._ftdi)
        self._mode = USBMuxMode.UNKNOWN
        self._id_pin = 0b0000
        pins = self.read_pins()
        self._id_pin = pins & 0b00000100
        pins = pins & 0b00000011
        logging.debug("Detecting the current mode of the USBMux device...")
        for mode in USBMuxMode:
            if mode.value == pins:
                self._mode = mode
                break
        else:
            self._mode = USBMuxMode.UNKNOWN
        logging.debug(
            f"Detected mode: {self._mode} id_pin: {'HIGH' if self._id_pin else 'LOW'}"
        )

    @classmethod
    def list(self, all: bool = False):
        f = Ftdi()
        descriptor: UsbDeviceDescriptor
        interface: int
        devices = f.list_devices()
        logging.debug(f"Found Device Count: {len(devices)}")
        logging.debug(f"Devices List Raw: {devices}")
        if all:
            return devices

        filtered = []
        for descriptor, interface in devices:
            if "bdgrd" in descriptor.sn:
                filtered.append((descriptor, interface))
        return filtered

    def _switch_to_cbus_mode(self):
        self._ftdi.set_bitmode(0b1111, Ftdi.BitMode.CBUS)

    def read_pins(self) -> int:
        # self.dump_config()
        pins = 0
        # self._ftdi.set_bitmode(0x03, Ftdi.BitMode.CBUS)
        pins = self._ftdi.read_pins()
        logging.debug(f"Value of the pins: 0b{pins:04b}")
        # self._ftdi.set_cbus_direction(0b1111, 0b0011)
        # self._ftdi.set_cbus_gpio(USBMuxMode.NO_CONNECTION.value)
        # pins = self._ftdi.read_pins()
        # logging.debug(f"Value of the pins: 0b{pins:04b}")
        # self._ftdi.set_bitmode(0xFF, Ftdi.BitMode.CBUS)
        return pins

    def get_status(self):
        return (self._mode, True if self._id_pin > 0 else False)

    def dump_config(self):
        print("cbus_pins", self._eeprom.cbus_pins)
        logging.debug(self._eeprom.dump_config())

    def configure(self, new_serial: str):
        self._eeprom.set_manufacturer_name("Badgerd Tech.")
        self._eeprom.set_product_name("Badgerd USBMux")
        self._eeprom.set_serial_number(new_serial)

        self._eeprom.set_property("cbus_func_0", "GPIO")
        self._eeprom.set_property("cbus_func_1", "GPIO")
        self._eeprom.set_property("cbus_func_2", "GPIO")
        self._eeprom.set_property("cbus_func_3", "GPIO")
        # Pull request created https://github.com/eblot/pyftdi/pull/364
        # eeprom.set_property("cbus_func_0", eeprom.CBUSX.GPIO)
        # eeprom.set_property("cbus_func_1", eeprom.CBUSX.GPIO)
        # eeprom.set_property("cbus_func_2", eeprom.CBUSX.GPIO)
        # eeprom.set_property("cbus_func_3", eeprom.CBUSX.GPIO)

        self._eeprom.commit(dry_run=False)
        self._eeprom.reset_device()

    def set_mode(self, mode: USBMuxMode):
        """
        CBUS0    CBUS1
        1        0       No Connection
        0        0       DUT to Host
        1        1       DUT to Device
        0        1       Device to Host

        CBUS2 is DUT ID
        """
        logging.debug(f"Setting connection mode to {mode}")
        match mode:
            case USBMuxMode.DUT_TO_DEVICE:
                pass
            case USBMuxMode.HOST_TO_DEVICE:
                pass
            case USBMuxMode.HOST_TO_DUT:
                pass
            case USBMuxMode.NO_CONNECTION:
                pass
            case _:
                raise ValueError(f"Unknown mode: {mode}")
        self._mode = mode
        new_pins = self._mode.value
        if self._id_pin > 0:
            new_pins = new_pins | self._id_pin
        else:
            new_pins = new_pins & ~self._id_pin

        logging.debug(f"Updating pin values to: 0b{new_pins:04b}")
        self._ftdi.set_bitmode((0b11110000 | new_pins), Ftdi.BitMode.CBUS)

    def set_id_pin(self, value):
        """
        CBUS2 is DUT ID
        """
        logging.debug(f"Setting ID pin in DUT port to: {'HIGH' if value else 'LOW'}")
        logging.debug(f"Current mode of the usbmux device: {self._mode}")
        # self._ftdi.set_cbus_direction(0b0100, 0b0100)
        new_pins = self._mode.value
        if value:
            # CBUS3 *CBUS2* CBUS1 CBUS0
            new_pins = new_pins | 0b0100
            self._id_pin = 0b0100
        else:
            new_pins = new_pins & 0b1011
            self._id_pin = 0b0000

        logging.debug(f"Updating pin values to: 0b{new_pins:04b}")
        self._ftdi.set_bitmode((0b11110000 | new_pins), Ftdi.BitMode.CBUS)
