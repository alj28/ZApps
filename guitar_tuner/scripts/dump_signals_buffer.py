import swd
import argparse
import subprocess
import pandas as pd

SIGNAL_BUFFER_VAR_NAME = "audio_samples"
SIGNAL_BUFFER_VAR_INDEX_NAME = "audio_samples_indx"
ELF_NAME = "./../build/zephyr/zephyr.elf"

SIGNAL_BUFFER_SRC_FILE = "../src/signal_buffer.c"

dev = swd.Swd()
cm = swd.CortexM(dev)

def get_var_value_by_address(address):
    return dev.get_mem32(address)

def get_var_address(var_name):
    response = subprocess.run(["nm", ELF_NAME], capture_output=True).stdout
    lines = [l.decode("utf-8") for l in response.splitlines()]
    for l in lines:
        tokens = l.split(' ')
        name = tokens[-1].strip()
        if var_name == name:
            address = int(f"0x{tokens[0].strip()}", 16)
            return address
    return -1

def get_var_value_by_name(var_name):
    address = get_var_address(var_name)
    return get_var_value_by_address(address)

def get_buffer_size():
    with open(SIGNAL_BUFFER_SRC_FILE, 'r') as f:
        lines = f.readlines()

    for l in lines:
        tokens = l.split()
        if len(tokens) < 3:
            continue
        if tokens[0].strip() != "#define":
            continue
        if tokens[1].strip() != "AUDIO_SAMPLES_SIZE":
            continue
        return int(tokens[2].strip(), 10)

    return -1

def generate_samples_address_vector():
    buffer_address = get_var_address(SIGNAL_BUFFER_VAR_NAME)
    buffer_index = get_var_value_by_name(SIGNAL_BUFFER_VAR_INDEX_NAME)
    buffer_size = get_buffer_size()

    indexes = [(buffer_index + i)%buffer_size for i in range(buffer_size)]

    return [buffer_address + i*2 for i in indexes]

def read_buffer(addresses):
    output = []
    for a in addresses:
        byte_list = [b for b in dev.read_mem(a, 2)]
        v = byte_list[0]
        v += (byte_list[1] << 8)
        v = v if v < 0x8000 else (v - 0x10000)
        output += [v]
    return output

def store_buffer_to_csv(samples, file_name):
    d = {
        "val" : samples
    }
    df = pd.DataFrame(d)
    df.to_csv(file_name)


if '__main__' == __name__:
    parser = argparse.ArgumentParser(prog='dump signals buffer')
    parser.add_argument('output')
    args = parser.parse_args()

    print(args.output)

    print(f"SWD: {dev.get_version().str}")
    print(f"Voltage: {dev.get_target_voltage()}")
    print(f"MCU ID: 0x{dev.get_idcode():08x}")

    print("Start reading memory")
    cm.halt()
    addresses = generate_samples_address_vector()
    data = read_buffer(addresses)
    cm.run()

    print("Writing to file.")
    store_buffer_to_csv(data, args.output)


