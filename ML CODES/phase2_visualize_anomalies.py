import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv("ml_dataset_component_anomalies.csv")
df["ts"] = pd.to_datetime(df["ts"])

# Helper plotting function
def plot_anomaly(time, normal_y, anomaly_y, ylabel, title):
    plt.figure()
    plt.plot(time, normal_y, label="Normal")
    plt.scatter(time, anomaly_y, label="Anomaly")
    plt.xlabel("Time")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.legend()
    plt.show()

# -----------------------------
# TEMPERATURE
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["temp_anomaly"] == "Normal"]["temperature"],
    df[df["temp_anomaly"] == "Anomaly"]["temperature"],
    "Temperature (°C)",
    "Temperature Anomaly Detection"
)

# -----------------------------
# SUPERHEAT
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["temp_anomaly"] == "Normal"]["superheat"],
    df[df["temp_anomaly"] == "Anomaly"]["superheat"],
    "Superheat",
    "Superheat Anomaly Detection"
)

# -----------------------------
# POWER
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["power_anomaly"] == "Normal"]["power_watts"],
    df[df["power_anomaly"] == "Anomaly"]["power_watts"],
    "Power (Watts)",
    "Power Anomaly Detection"
)

# -----------------------------
# FAN RPM
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["rpm_anomaly"] == "Normal"]["fan_rpm"],
    df[df["rpm_anomaly"] == "Anomaly"]["fan_rpm"],
    "Fan RPM",
    "Fan RPM Anomaly Detection"
)

# -----------------------------
# COMPRESSOR RPM
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["rpm_anomaly"] == "Normal"]["compressor_rpm"],
    df[df["rpm_anomaly"] == "Anomaly"]["compressor_rpm"],
    "Compressor RPM",
    "Compressor RPM Anomaly Detection"
)

# -----------------------------
# VIBRATION
# -----------------------------
plot_anomaly(
    df["ts"],
    df[df["vibration_anomaly"] == "Normal"]["vibration"],
    df[df["vibration_anomaly"] == "Anomaly"]["vibration"],
    "Vibration",
    "Vibration Anomaly Detection"
)
