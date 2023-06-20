# -*- coding: utf-8 -*-
"""
Created on Tue Jun 20 08:37:20 2023

@author: Ridwane
"""

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
from create_plot import *


def get_filenames():
    a = 1
    filename = "blabla"
    filenames = []
    while (a<10):
        filename = (input("Filename: "))
        if (filename==''): break
        a+=1
        filenames.append("C:/Users/Ridwane/Documents/Thèse/Resultats/"+filename)
    return(filenames)

def parse_files(filenames):
    filenames_dicts = []
    for filename in filenames:
        filenames_dicts.append(parse_text_file(filename)[0])
    return filenames_dicts


def plot_comp(data_dicts, keyname, filenames):
    max_value = 0
    min_value = 1e100
    fig, ax1 = plt.subplots(figsize=(10, 7), dpi=120)

    for i, data_dict in enumerate(data_dicts):
        keys = []
        values = []

        for key, value in data_dict.items():
            keys.append(key)
            values.append(float(value[0]))

        keys = list(map(int, keys))

        ax1.plot(keys, values, label=(filenames[i].split('/')[-1].split('.')[0]))
        ax1.scatter(keys, values)
        if (max(values)>max_value): max_value = max(values)
        if (min(values)<min_value): min_value = min(values)


    ax1.set_xlabel(keyname[1]+" (ms)")
    ax1.set_ylabel('Processing cycles')
    ax1.set_yscale('log')
    ax1.set_xscale('log')
    ax1.yaxis.set_major_locator(ticker.LogLocator(numticks=10))
    ax1.yaxis.set_major_formatter(ticker.LogFormatter())
    print(max_value, min_value)
    exp_max = np.ceil(np.log10(max_value))
    exp_min = np.floor(np.log10(min_value))
    y_max = np.power(10, exp_max)
    y_min = np.power(10, exp_min)
    ax1.set_ylim(y_min, y_max)
    ax1.set_xlim(left=0.9, right=220)
    ax1.set_xticks(keys)
    ax1.set_xticklabels(keys, rotation=45, ha='right')


    plt.title(keyname[0]+'processing cycles of signature based on ' + keyname[1])
    plt.tight_layout()
    plt.legend()
    plt.show()

def plot_comp_max(filenames_dicts, filenames):
    d_intervals = []
    for data_dict in filenames_dicts:
        d_intervals.append(compute_max(data_dict))
    plot_comp(d_intervals, ["Maximum ","Alloted Time"], filenames)
    
def plot_comp_mean(filenames_dicts, filenames):
    d_intervals = []
    for data_dict in filenames_dicts:
        d_intervals.append(compute_mean(data_dict))
    plot_comp(d_intervals, ["Mean ", "Alloted Time"], filenames)

def main():
    filenames = get_filenames()
    filenames = ["C:/Users/Ridwane/Documents/Thèse/Resultats/"+filename for filename in ("Dilithium2signO0.csv", "Dilithium3signO0.csv", "Dilithium2signOfast.csv", "Falcon512signO0.csv", "Falcon512signOfast.csv")]
    filenames_dicts = parse_files(filenames)
    plot_comp_max(filenames_dicts, filenames)
    plot_comp_mean(filenames_dicts, filenames)


main()
'''    
Dilithium2signO0.csv
Dilithium3signO0.csv
Falcon512signO0.csv'''
    
