import matplotlib.pyplot as plt
import pandas as pd

def plot(data, name, file):
    df = pd.DataFrame(data, columns=["Circuit", "Framework", "Cycles/sample"])
    pivot_df = df.pivot(index="Circuit", columns="Framework", values="Cycles/sample")

    # Narrower figure width
    fig, ax = plt.subplots(figsize=(8, 6))

    pivot_df.plot(kind="bar", ax=ax)

    ax.set_yscale("log", base=2)
    ax.set_ylabel("Cycles / Sample")
    ax.set_title(name)

    # Make x-axis more compact
    ax.margins(x=0.02)
    ax.set_xticklabels(ax.get_xticklabels(), rotation=0, ha="center")

    ax.legend(title="Framework")
    plt.tight_layout()
    plt.savefig(file)

data_m1 = [
    ["RC Lowpass", "RT-WDF", 72.07],
    ["RC Lowpass", "wdmodels", 19.04],
    ["RC Lowpass", "chowdsp_wdf", 22.09],
    ["RC Lowpass", "wdf_compiler", 14.94],

    ["Pre-Amp EQ", "RT-WDF", 346.18],
    ["Pre-Amp EQ", "wdmodels", 73.30],
    ["Pre-Amp EQ", "chowdsp_wdf", 66.33],
    ["Pre-Amp EQ", "wdf_compiler", 58.02],

    ["Diode Clipper", "wdmodels", 374.53],
    ["Diode Clipper", "chowdsp_wdf", 131.78],
    ["Diode Clipper", "wdf_compiler", 112.79],

    ["Baxandall EQ", "RT-WDF", 588.16],
    ["Baxandall EQ", "wdmodels", 59.23],
    ["Baxandall EQ", "chowdsp_wdf", 65.55],
    ["Baxandall EQ", "wdf_compiler", 36.44],
]
data_zen4 = [
    ["RC Lowpass", "RT-WDF", 99.85],
    ["RC Lowpass", "wdmodels", 15.03],
    ["RC Lowpass", "chowdsp_wdf", 31.73],
    ["RC Lowpass", "wdf_compiler", 19.04],

    ["Pre-Amp EQ", "RT-WDF", 505.42],
    ["Pre-Amp EQ", "wdmodels", 80.29],
    ["Pre-Amp EQ", "chowdsp_wdf", 95.77],
    ["Pre-Amp EQ", "wdf_compiler", 70.58],

    ["Diode Clipper", "wdmodels", 389.10],
    ["Diode Clipper", "chowdsp_wdf", 141.86],
    ["Diode Clipper", "wdf_compiler", 113.12],

    ["Baxandall EQ", "RT-WDF", 805.60],
    ["Baxandall EQ", "wdmodels", 72.95],
    ["Baxandall EQ", "chowdsp_wdf", 112.25],
    ["Baxandall EQ", "wdf_compiler", 43.60],
]

plot(data_m1, "WDF Framework Performance Comparison (Apple M1)", "chart_m1.png")
plot(data_zen4, "WDF Framework Performance Comparison (AMD Zen 4)", "chart_zen4.png")
