import pandas as pd
from sklearn.ensemble import IsolationForest

# Load ML dataset
df = pd.read_csv("ml_dataset.csv")

# Helper function
def detect_anomaly(data):
    model = IsolationForest(
        n_estimators=100,
        contamination=0.05,
        random_state=42
    )
    return model.fit_predict(data)

# Temperature anomaly
df["temp_anomaly"] = detect_anomaly(df[["temperature", "evap_temp", "superheat"]])
df["temp_anomaly"] = df["temp_anomaly"].apply(lambda x: "Anomaly" if x == -1 else "Normal")

# Power anomaly
df["power_anomaly"] = detect_anomaly(df[["power_watts", "power_per_rpm"]])
df["power_anomaly"] = df["power_anomaly"].apply(lambda x: "Anomaly" if x == -1 else "Normal")

# RPM anomaly
df["rpm_anomaly"] = detect_anomaly(df[["fan_rpm", "compressor_rpm"]])
df["rpm_anomaly"] = df["rpm_anomaly"].apply(lambda x: "Anomaly" if x == -1 else "Normal")

# Vibration anomaly
df["vibration_anomaly"] = detect_anomaly(df[["vibration"]])
df["vibration_anomaly"] = df["vibration_anomaly"].apply(lambda x: "Anomaly" if x == -1 else "Normal")

# Save output
df.to_csv("ml_dataset_component_anomalies.csv", index=False)

print("✅ Component-level anomalies generated")
