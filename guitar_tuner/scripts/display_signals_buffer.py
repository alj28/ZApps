
import argparse
import pandas as pd
import matplotlib.pyplot as plt

def import_data(file_name):
    df = pd.read_csv(file_name)
    return df["val"]

def plot_signal(samples):
    plt.plot(samples)
    plt.show()

if "__main__" == __name__:
    parser = argparse.ArgumentParser(prog='dump signals buffer')
    parser.add_argument('file')
    args = parser.parse_args()

    samples = import_data(args.file)
    plot_signal(samples)
    