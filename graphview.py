import argparse
import subprocess
import numpy as np
import pwlf
from matplotlib import pyplot as plt

parser = argparse.ArgumentParser(description='uperf parameters')

parser.add_argument('-exec', type=str)
parser.add_argument('-case', type=str)
parser.add_argument('-start', type=str)
parser.add_argument('-end', type=str)
parser.add_argument('-step', type=str)
parser.add_argument('-delay', type=str)
parser.add_argument('-prologue', type=str)
parser.add_argument('-epilogue', type=str)
parser.add_argument('-loop', type=str)
parser.add_argument('-inst_num', type=str)
parser.add_argument('-thrput_inst', type=str)
parser.add_argument('-thrput_fill', type=str)
parser.add_argument('-affinity', type=str)
parser.add_argument('-f', type=str)

args = parser.parse_args()
cmd = [args.exec]
if args.case is not None:
    cmd.extend(['-case', args.case])
if args.start is not None:
    cmd.extend(['-start', args.start])
if args.end is not None:
    cmd.extend(['-end', args.end])
if args.step is not None:
    cmd.extend(['-step', args.step])
if args.delay is not None:
    cmd.extend(['-delay', args.delay])
if args.prologue is not None:
    cmd.extend(['-prologue', args.prologue])
if args.epilogue is not None:
    cmd.extend(['-epilogue', args.epilogue])
if args.loop is not None:
    cmd.extend(['-loop', args.loop])
if args.inst_num is not None:
    cmd.extend(['-inst_num', args.inst_num])
if args.thrput_inst is not None:
    cmd.extend(['-thrput_inst', args.thrput_inst])
if args.thrput_fill is not None:
    cmd.extend(['-thrput_fill', args.thrput_fill])
if args.affinity is not None:
    cmd.extend(['-affinity', args.affinity])
if args.f is not None:
    cmd.extend(['-f', args.f])

p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
x = []
y = []
starttest = False
list_of_strings = [x.decode('utf-8').rstrip('\n')
                   for x in iter(p.stdout.readlines())]
for line in list_of_strings:
    if line.find('test start:') == 0:
        starttest = True
    elif starttest == True:
        point = line.strip().split(' ')
        x.append(int(point[0]))
        y.append(float(point[1]))
    else:
        print(line)

x_matrix = np.vstack([x, np.ones(len(x))]).T
lr = np.linalg.lstsq(x_matrix, y, rcond=None)
print("IPC if no delay:" + str(1/lr[0][0]))

my_pwlf = pwlf.PiecewiseLinFit(x, y)
res = my_pwlf.fit(3)
print(res)
y_fit = my_pwlf.predict(x)

plt.title(cmd)
plt.plot(x, y)
plt.plot(x, y_fit)
plt.show()
