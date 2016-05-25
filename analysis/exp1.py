import numpy as np
import matplotlib.pyplot as plt
import math


def plotResults(uni_interval, nuni_interval):
    n = len(uni_interval)

    ind = np.arange(n)  # the x locations for the groups
    width = 0.35       # the width of the bars

    fig, ax = plt.subplots()
    rects1 = ax.bar(ind, uni_interval, width, color='r')

    rects2 = ax.bar(ind + width, nuni_interval, width, color='b')

    # add some text for labels, title and axes ticks
    ax.set_xlabel('Revenue Ratio', fontsize=18)
    ax.set_ylabel('Case Proportion', fontsize=18)
    # ax.set_title('Optimality Gap')
    ax.set_xticks(ind + width)
    ax.set_xticklabels(('(0,0.75]', '(0.75,0.8]', '(0.8,0.85]',
                        '(0.85,0.9]', '(0.9,0.95]', '(0.95,1]'))

    ax.legend((rects1[0], rects2[0]), ('Optimal Uniform', 'Non-uniform'), loc='best')


    plt.show()


def analysis():
    res = []
    with open('op.txt') as f:
        for line in f.readlines():
            res.append([int(x) for x in line.strip().split()])

    s_uni = []
    s_nuni = []
    uni_interval = [0,0,0,0,0,0]
    nuni_interval = [0,0,0,0,0,0]
    for uni, nuni, op in res:
        s_uni.append(uni*1.0/op)
        s_nuni.append(nuni*1.0/op)
        uni_idx = int(uni * 20.0 / op - 14)
        nuni_idx = int(nuni * 20.0 / op - 14)
        uni_interval[uni_idx] += 1
        nuni_interval[nuni_idx] += 1

    uni_interval = [1.0*x / np.sum(uni_interval) for x in uni_interval]
    nuni_interval = [1.0*x / np.sum(nuni_interval) for x in nuni_interval]
    print np.min(s_uni), np.max(s_uni), np.min(s_nuni), np.max(s_nuni)
    print np.average(s_uni), np.average(s_nuni)
    return uni_interval, nuni_interval

if __name__ == '__main__':
    uni_interval, nuni_interval = analysis()
    plotResults(uni_interval, nuni_interval)
    # print 1.0/(math.log(200,2)+1)