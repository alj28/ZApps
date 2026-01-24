
import swd
import argparse
import subprocess

SIGNAL_BUFFER_VAR_NAME = "audio_samples"
ELF_NAME = "./../build/zephyr/zephyr.elf"

SIGNAL_BUFFER_SRC_FILE = "../src/signal_buffer.c"

def get_buffer_location():
    response = subprocess.run(["nm", ELF_NAME], capture_output=True).stdout
    lines = [l.decode("utf-8") for l in response.splitlines()]
    for l in lines:
        tokens = l.split(' ')
        name = tokens[-1].strip()
        if SIGNAL_BUFFER_VAR_NAME == name:
            address = int(f"0x{tokens[0].strip()}", 16)
            return address
    return -1
        
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

dev = swd.Swd()

def read_buffer(buffer_location, buffer_size):
    current_address = buffer_location
    address_inc = 4
    output = []
    for i in range(int(buffer_size)):
        val = dev.get_mem32(current_address)
        val &= 0xFFFFFFFF
        val = val if val < 0x80000000 else (val - 0x100000000)
        output += [val]
        current_address += address_inc
    return output


if "__main__" == __name__:
    parser = argparse.ArgumentParser(prog='dump signals buffer')
    parser.add_argument('output')
    args = parser.parse_args()

    print(args.output)


    print(f"SWD: {dev.get_version().str}")
    print(f"Voltage: {dev.get_target_voltage()}")
    print(f"MCU ID: 0x{dev.get_idcode():08x}")

    buffer_size = get_buffer_size()
    assert -1 != buffer_size

    buffer_location = get_buffer_location()
    assert -1 != buffer_location
    print(f"Buffer at location 0x{buffer_location:08x}, size {buffer_size}")

    print("Start reading memory")
    buffer = read_buffer(buffer_location, buffer_size)
    
    print("Writing to file.")
    with open(args.output, 'w') as f:
        for s in buffer:
            f.write(f'{s}, ')




