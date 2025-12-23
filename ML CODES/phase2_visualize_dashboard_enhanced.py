import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv("ml_dataset_component_anomalies.csv")
df["ts"] = pd.to_datetime(df["ts"])

WINDOW = 5

def plot_component(ax, value_col, anomaly_col, ylabel, title):
    normal = df[df[anomaly_col] == "Normal"]
    anomaly = df[df[anomaly_col] == "Anomaly"]

    trend = df[value_col].rolling(WINDOW).mean()

    ax.plot(normal["ts"], normal[value_col], label="Normal")
    ax.scatter(anomaly["ts"], anomaly[value_col], s=80, label="Anomaly")
    ax.plot(df["ts"], trend, linestyle="--", label="Trend")

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.legend()

# Create 3x2 dashboard
fig, axs = plt.subplots(3, 2, figsize=(14, 12))
fig.suptitle("Predictive Maintenance – Full Anomaly Dashboard", fontsize=14)

plot_component(axs[0, 0], "temperature", "temp_anomaly", "Temperature (°C)", "Temperature")
plot_component(axs[0, 1], "superheat", "temp_anomaly", "Superheat", "Superheat")

plot_component(axs[1, 0], "power_watts", "power_anomaly", "Power (W)", "Power")
plot_component(axs[1, 1], "fan_rpm", "rpm_anomaly", "Fan RPM", "Fan RPM")

plot_component(axs[2, 0], "compressor_rpm", "rpm_anomaly", "Compressor RPM", "Compressor RPM")
plot_component(axs[2, 1], "vibration", "vibration_anomaly", "Vibration", "Vibration")

plt.tight_layout()
plt.show()
