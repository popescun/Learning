import numpy as np
import matplotlib.pyplot as plt
plt.style.use('../deeplearning.mplstyle')

# x_train is the input variable (size in 1000 square feet)
# y_train is the target (price in 1000's of dollars)
x_train = np.array([1.0, 2.0])
y_train = np.array([300.0, 500.0])
print(f'x_train: {x_train}')
print(f'y_train: {y_train}')

# m is the number of training examples
print(f'x_train.shape: {x_train.shape[0]}')
m = x_train.shape[0]
print(f'Number of training example is: {m}')

m = len(x_train)
print(f'Number of training example is: {m}')

i = 0 # Change this to 1 to see (x^1, y^1)

x_i = x_train[i]
y_i = y_train[i]
print(f'(x_{i}, y_{i}) = ({x_i}, {y_i})')

# plot the data points
plt.scatter(x_train, y_train, marker='x', c='r')
# set the  title
plt.title('Housing Prices')
# set the y-axis label
plt.ylabel('Price (in 1000\'s of dollars)')
# set the x-axis label
plt.xlabel('Size (in 1000 sqft)')
# plt.show()

w = 100
b = 100
print(f'w: {w}')
print(f'b: {b}')

def compute_model_output(x: np.array, w: float, b: float):
    """
    Computes the prediction of a linear model
    Args:
        x (ndarray (m,)): Data, m examples
        w,b (scalar)   : model parameters
    Returns
        f_wb (ndarray (m,)): model prediction
    """
    m = x.shape[0]
    f_wb = np.zeros(m)
    for i in range(m):
        f_wb[i] = w * x[i] + b
    return f_wb

tmp_f_wb = compute_model_output(x_train, w, b,)

# plot our model prediction
#plt.plot(x_train, tmp_f_wb, c='b', label='Our Prediction')

# plot the data points
plt.scatter(x_train, y_train, marker='x', c='r', label='Our Prediction')
# set the  title
plt.title('Housing Prices')
# set the y-axis label
plt.ylabel('Price (in 1000\'s of dollars)')
# set the x-axis label
plt.xlabel('Size (in 1000 sqft)')
plt.legend()
# plt.show()

# prediction line fits the data for:
w = 200
b = 100

tmp_f_wb = compute_model_output(x_train, w, b,)
plt.plot(x_train, tmp_f_wb, c='b', label='Our Prediction')
plt.show()

x_i = 1.2
cost_1200sqft =  w * x_i + b
print(f'cost_1200sqft: ${cost_1200sqft:.0f} thousand dollars')