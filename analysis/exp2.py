import numpy as np
import matplotlib.pyplot as plt
import math


def plotResults(index):
    # n = len(uni_interval)
    #
    # ind = np.arange(n)  # the x locations for the groups
    # width = 0.35       # the width of the bars

    idx = [1.0*i/index[-1] for i in index]
    x = [i/100.0 for i in xrange(len(index))]

    for i in xrange(len(index)):
        if index[i] > 0:
            min_x = i/100.0
            break

    fig, ax = plt.subplots()
    plt.plot(x, idx)

    ax.fill_between(x, 0, idx, facecolor='blue')
    plt.xlim([min_x, 1])
    # rects1 = ax.bar(ind, uni_interval, width, color='r')
    #
    # rects2 = ax.bar(ind + width, nuni_interval, width, color='b')
    #
    # # add some text for labels, title and axes ticks
    ax.set_xlabel('Revenue Ratio', fontsize=18)
    ax.set_ylabel('Case Proportion', fontsize=18)
    # ax.set_title('Optimality Gap')
    # ax.set_xticks(ind + width)
    # ax.set_xticklabels([1,2,3,4,5])
    #
    # ax.legend((rects1[0], rects2[0]), ('Optimal Uniform', 'Non-uniform'), loc='best')


    plt.show()


def analysis():
    res = []
    with open('../results/exp2.txt') as f:
        for line in f.readlines():
            res.append([int(x) for x in line.strip().split()])

    index = [0 for i in xrange(101)]
    for opt, app in res:
        idx = int(app * 100.0 / opt)
        index[idx] += 1
    for i in xrange(100):
        index[i+1] += index[i]

    return index

if __name__ == '__main__':
    index = analysis()
    plotResults(index)
    # print str(0.5) + str(0.5)