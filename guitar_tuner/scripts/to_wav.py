
import wave
import struct
import argparse
import pandas as pd

def import_data(file_name):
    df = pd.read_csv(file_name)
    return df["val"]

def export_to_wav(samples, file_name):
    FRAME_RATE = 4e3
    BIT_DEPTH = 16
    with wave.open(file_name, mode="wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(int(BIT_DEPTH/8))
        wav_file.setframerate(FRAME_RATE)
        frame_bytes = b''.join(struct.pack('<h', v) for v in samples) 
        wav_file.writeframes(frame_bytes)

if "__main__" == __name__:
    parser = argparse.ArgumentParser(prog='dump signals buffer')
    parser.add_argument('input')
    parser.add_argument('output')
    args = parser.parse_args()

    samples = import_data(args.input)
    export_to_wav(samples, args.output)
