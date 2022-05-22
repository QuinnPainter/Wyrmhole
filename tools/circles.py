#!/usr/bin/python3
#https://stackoverflow.com/questions/35801952/drawing-concentric-tiling-circles-with-even-diameter

import numpy as np
import matplotlib.pyplot as pp

fg = pp.figure()
ax = fg.add_subplot(111)

def point(x, y, c):
    xx = [x - 1/2, x + 1/2, x + 1/2, x - 1/2, x - 1/2 ]
    yy = [y - 1/2, y - 1/2, y + 1/2, y + 1/2, y - 1/2 ]
    ax.plot(xx, yy, 'k-')
    ax.fill_between(xx, yy, color=c, linewidth=0)

def half_integer_centered_circle(R, c):
    x = 1
    y = R
    while y >= x:
        point(x, y, c)
        point(x, - y + 1, c)
        point(- x + 1, y, c)
        point(- x + 1, - y + 1, c)
        point(y, x, c)
        point(y, - x + 1, c)
        point(- y + 1, x, c)
        point(- y + 1, - x + 1, c)
        def test(x, y):
            rSqr = x**2 + y**2
            return (R - 1/2)**2 < rSqr and rSqr < (R + 1/2)**2
        if test(x + 1, y):
            x += 1
        elif test(x, y - 1):
            y -= 1
        else:
            x += 1
            y -= 1

for i in range(5, 10):
    half_integer_centered_circle(2*i - 1, 'r')
    half_integer_centered_circle(2*i, 'b')

pp.axis('equal')
pp.show()
