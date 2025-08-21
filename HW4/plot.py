import matplotlib.pyplot as plt

autocorrelationcoeff = [70, -2, -10, -2, -10, -2, -10, 14, 6, 2, 6, -2, 6, -2, -42, 6, 6, 6, 6, 6, 6, 14, -10, -2, -10, -2, -10, -2, 70, -2, -10, -2, -10, -2, -10, 14, 6, 2, 6, -2, 6, -2, -42, 6, 6, 6, 6, 6, 6, 14, -10, -2, -10, -2, -10, -2, 70, -2, -10, -2, -10, -2, -10, 14, 6, 2, 6, -2, 6, -2]

x = list(range(len(autocorrelationcoeff)))

plt.figure(figsize=(12, 5))
plt.stem(x, autocorrelationcoeff, use_line_collection=True)
plt.xlabel("Shift (rshift)")
plt.ylabel("Autocorrelation Coefficient")
plt.title("Autocorrelation vs. Shift")
plt.grid(True)
plt.show()
