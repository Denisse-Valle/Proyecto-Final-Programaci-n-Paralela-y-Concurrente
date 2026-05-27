import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/threads_benchmark.csv")

# Grafica tiempo
plt.figure(figsize=(8,5))
plt.plot(df["threads"], df["time"], marker='o')
plt.xlabel("Threads")
plt.ylabel("Tiempo (s)")
plt.title("Tiempo vs Threads")
plt.grid(True)
plt.savefig("images/time_vs_threads.png")

# Grafica speedup
plt.figure(figsize=(8,5))
plt.plot(df["threads"], df["speedup"], marker='o')
plt.xlabel("Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Threads")
plt.grid(True)
plt.savefig("images/speedup_vs_threads.png")

print("Graficas generadas.")