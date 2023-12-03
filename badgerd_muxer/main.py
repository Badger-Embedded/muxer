import logging
import click
from typing import List, Tuple
from pyftdi.usbtools import UsbDeviceDescriptor
from badgerd_muxer.devices.usbmux import USBMux, USBMuxMode


@click.group()
@click.option("-d", "--debug", is_flag=True)
@click.version_option()
def cli(debug):
    if debug:
        logging.basicConfig(level=logging.DEBUG)
        logging.getLogger("pyftdi").setLevel(logging.DEBUG)


@cli.group()
def usbmux():
    pass


@usbmux.command("list")
@click.option("-a", "--all", is_flag=True, default=False, help="Lists all FTDI devices")
def usbmux_list(all):
    devices: List[Tuple[UsbDeviceDescriptor, int]] = USBMux.list(all)

    for descriptor, interface in devices:
        click.echo(
            f"Interface: {interface}, Description: {descriptor.description}, Vid: 0x{descriptor.vid:04x}, Pid: 0x{descriptor.pid:04x}, Serial: {descriptor.sn}"
        )


@usbmux.command("configure")
@click.option("--vid", default=0x0403, type=int)
@click.option("--pid", default=0x6015, type=int)
@click.option("-s", "--serial-number", "old_serial", required=True, type=str)
@click.argument("new_serial", type=str)
def usbmux_configure(vid: int, pid: int, old_serial: str, new_serial: str) -> None:
    device = USBMux(old_serial, vid, pid)
    device.configure(new_serial)


@usbmux.command("status")
@click.option("--vid", default=0x0403, type=int)
@click.option("--pid", default=0x6015, type=int)
@click.argument("serial_number", type=str)
def usbmux_status(vid, pid, serial_number):
    device = USBMux(serial=serial_number, vid=vid, pid=pid)
    mode: USBMuxMode
    id_pin: bool
    mode, id_pin = device.get_status()
    click.echo(
        f"[{1 if id_pin else 0}:{mode.value}] USB-ID State: {'HIGH' if id_pin else 'LOW'} and Mode: {mode}"
    )


@usbmux.group("set", help="SERIAL_NUMBER: Ex. bdgrd_usbmux_101")
@click.option("--vid", default=0x0403, type=int)
@click.option("--pid", default=0x6015, type=int)
@click.argument("serial_number", type=str)
@click.pass_context
def usbmux_set(ctx: click.Context, serial_number: str, vid: int, pid: int):
    ctx.ensure_object(dict)

    ctx.obj["serial_number"] = serial_number
    ctx.obj["vid"] = vid
    ctx.obj["pid"] = pid


@usbmux_set.command("mode")
@click.option(
    "-c",
    "--connect",
    type=click.Choice(
        ["dut", "device", "dut-device", "no-connect"], case_sensitive=False
    ),
    help="""
    dut: connects HOST and DUT port of the USBMUX together\n
    device: connects HOST and DEVICE port of the USBMUX together\n
    dut-device: connects DEVICE and DUT port of the USBMUX together
    """,
)
@click.pass_context
def usbmux_set_mode(ctx: click.Context, connect: str):
    ctx.ensure_object(dict)
    device = USBMux(
        serial=ctx.obj["serial_number"], vid=ctx.obj["vid"], pid=ctx.obj["pid"]
    )
    match connect:
        case "dut":
            device.set_mode(USBMuxMode.HOST_TO_DUT)
        case "device":
            device.set_mode(USBMuxMode.HOST_TO_DEVICE)
        case "dut-device":
            device.set_mode(USBMuxMode.DUT_TO_DEVICE)
        case "no-connect":
            device.set_mode(USBMuxMode.NO_CONNECTION)


@usbmux_set.command("id_pin")
@click.option("--high/--low", "value", default=False, show_default=True)
@click.pass_context
def usbmux_set_id_pin(ctx, value):
    ctx.ensure_object(dict)
    device = USBMux(
        serial=ctx.obj["serial_number"], vid=ctx.obj["vid"], pid=ctx.obj["pid"]
    )
    device.set_id_pin(value)
