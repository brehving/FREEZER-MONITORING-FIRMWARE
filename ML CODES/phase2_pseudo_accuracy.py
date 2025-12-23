import pandas as pd

# Load anomaly dataset
df = pd.read_csv("ml_dataset_with_anomalies.csv")

# Define threshold (bottom 5% anomaly scores)
threshold = df["anomaly_score"].quantile(0.05)

# Pseudo ground truth from scores
df["pseudo_label"] = df["anomaly_score"] < threshold   # True = anomaly

# Model prediction
df["model_label"] = df["anomaly_label"] == "Anomaly"   # True = anomaly

# Pseudo accuracy (consistency)
accuracy = (df["pseudo_label"] == df["model_label"]).mean() * 100

print(f"Isolation Forest Pseudo Accuracy: {accuracy:.2f}%")
