# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt

def parse_text_file(filename):
    d = {}
    d1 = {}
    # Open the file in read mode
    with open(filename, 'r') as file:
        
        lines = file.readlines()
        
        # Process each line
        for line in lines:
            if line.strip() == '':
                continue
            
            fields = line.split(',')
            
            key = fields[0].strip()
            interruptions = fields[4].strip()
            values = [field.strip() for field in fields[1:]]
            if key in d:
                d[key].append(values)
            else:
                d[key] = [values]
            if interruptions in d1:
                d1[interruptions].append(values)
            else:
                d1[interruptions] = [values]
    return (d,d1)

def compute_mean(data_dict):
    mean_dict = {}
    
    for key, values in data_dict.items():
        # Compute the mean for the desired fields
        cycles = [float(value[1]) for value in values]  
        times = [float(value[2]) for value in values]  
        mean_cycles = sum(cycles) / len(cycles)
        mean_time = sum(times) / len(times)
        mean_dict[key] = (mean_cycles,mean_time)
    
    return dict(sorted(mean_dict.items(), key=lambda x: int(x[0])))

def plot_graph(data_dict, keyname):
    keys = []
    values1 = []
    values2 = []

    for key, value in data_dict.items():
        keys.append(key)
        values1.append(float(value[0]))
        values2.append(float(value[1])/10)
    
    keys = list(map(int,keys))

    fig, ax1 = plt.subplots()

    ax1.plot(keys, values1, 'b-', label='Cycles')
    ax1.set_xlabel(keyname)
    ax1.set_ylabel('Processing cycles', color='b')
    ax1.tick_params('y', colors='b')
    ax1.set_ylim(0, max(values1)*1.2)

    ax2 = ax1.twinx()
    ax2.plot(keys, values2, 'r-', label='Time')
    ax2.set_ylabel('Time (ms)', color='r')
    ax2.tick_params('y', colors='r')

    plt.title('Processing cycles and computation time of signature based on ' + keyname)
    ax2.legend(loc='upper left', bbox_to_anchor=(0.4,1))
    ax1.legend(loc='upper left', bbox_to_anchor=(0.4,0.9))
    plt.show()




filename = input("Enter the filename: ")
filename = "C:/Users/Ridwane/Documents/Thèse/Resultats/"+filename
dicts = parse_text_file(filename)
d_interval = compute_mean(dicts[0])
print (d_interval)
d_interrupts = compute_mean(dicts[1])
print(d_interrupts)
plot_graph(d_interval, "Alloted time")
plot_graph(d_interrupts, "Number of interruptions")

