import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import numpy as np
# import streamlit as st # don't install as it breaks tensorflow
from matplotlib.ticker import (AutoMinorLocator, MultipleLocator)
import random

fig, ax = plt.subplots()

center = (0.5, 0.5)
radius = 0.4
circle = Circle(center, radius, edgecolor='blue', facecolor='none', linewidth=0.4)
ax.add_patch(circle)

ax.set_aspect('equal')

# Define the center of the circle (start point of vectors)
x_start = center[0]
y_start = center[1]

# Define a equal angles within a range
degrees = np.arange(0, 360, 6)
# print(degrees)
# angles = np.radians([0, 45, 90, 135, 180, 225, 270, 315])
angles = np.radians(degrees)

# Calculate the end points of the vectors on the circle's edge
x_end = x_start + radius * np.cos(angles)
y_end = y_start + radius * np.sin(angles)

# A simpler way is to plot lines directly for vectors from origin to a point
random.seed(0.13)
initial = None
prev = None
for i, angle in enumerate(angles):
    x_tip = center[0] + radius * np.cos(angle)
    y_tip = center[1] + radius * np.sin(angle)
    ax.plot([center[0], x_tip], [center[1], y_tip], color='red', linewidth=0.4, marker='', markersize=3)
    ax.annotate(i + 1, xy =(x_tip, y_tip), xytext =(x_tip, y_tip), fontsize=5)
    delta = random.uniform(0.13, 0.15)
    x_tip = center[0] + (radius - delta) * np.cos(angle)
    y_tip = center[1] + (radius - delta) * np.sin(angle)
    ax.plot(x_tip, y_tip, color='g', marker='o', markersize=1)
    if prev is not None:
        ax.plot([prev[0], x_tip], [prev[1], y_tip], color='g', linewidth=0.3)
    prev = (x_tip, y_tip)
    if i == 0:
        initial = prev

# draw the last segment
ax.plot([prev[0], initial[0]], [prev[1], initial[1]], color='g', linewidth=0.3)
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.xaxis.set_major_locator(MultipleLocator(0.1))
ax.yaxis.set_major_locator(MultipleLocator(0.1))
ax.grid(True)
plt.title("Circle with Multidimensions")
plt.show()