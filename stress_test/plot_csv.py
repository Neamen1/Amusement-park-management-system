
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Read the CSV file
file_path = r"G:\OneDrive\disk save\uni\7sem onedrive\pis + kurs\main fold\stress_test\perf_data train-neg.csv"  # Replace with the path to your CSV file
data = pd.read_csv(file_path)

# Set seaborn style for better visuals
sns.set_theme(style="whitegrid")

# Plot 1: Time taken for 'addFiles' vs num_files and num_threads
plt.figure(figsize=(12, 6))
sns.lineplot(
    data=data, 
    x="num_files", 
    y="time_taken_addFiles_ms", 
    hue="num_threads", 
    marker="o", 
    palette="tab10"
)
plt.title("Time Taken for Adding Files")
plt.xlabel("Number of Files")
plt.ylabel("Time Taken (ms)")
plt.legend(title="Number of Threads")
plt.grid(True)
plt.tight_layout()
plt.show()

# Plot 2: Time taken for 'buildIndex' vs num_files and num_threads
plt.figure(figsize=(12, 6))
sns.lineplot(
    data=data, 
    x="num_files", 
    y="time_taken_buildIndex_ms", 
    hue="num_threads", 
    marker="o", 
    palette="tab10"
)
plt.title("Time Taken for Building Index")
plt.xlabel("Number of Files")
plt.ylabel("Time Taken (ms)")
plt.legend(title="Number of Threads")
plt.grid(True)
plt.tight_layout()
plt.show()
