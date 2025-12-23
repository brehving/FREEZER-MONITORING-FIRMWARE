import pandas as pd
from sklearn.ensemble import IsolationForest

# Load dataset
df = pd.read_csv("ml_dataset.csv")

# Features for anomaly detection
features = [
    "temperature",
    "evap_temp",
    "superheat",
    "power_watts",
    "fan_rpm",
    "compressor_rpm",
    "vibration",
    "power_per_rpm",
    "fan_efficiency"
]

X = df[features]

# Train Isolation Forest
model = IsolationForest(
    n_estimators=100,
    contamination=0.05,
    random_state=42
)

df["anomaly"] = model.fit_predict(X)
df["anomaly_score"] = model.decision_function(X)

df["anomaly_label"] = df["anomaly"].apply(
    lambda x: "Anomaly" if x == -1 else "Normal"
)

# Save output
df.to_csv("ml_dataset_with_anomalies.csv", index=False)

print("✅ Phase 2 completed: anomalies detected")
