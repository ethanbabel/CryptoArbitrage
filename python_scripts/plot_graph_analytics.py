#!/usr/bin/env python3
import sys
import pandas as pd
import matplotlib.pyplot as plt

def plot_cycle_size_distribution(csv_file, output_file):
    # Read the CSV file into a DataFrame
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Error reading {csv_file}: {e}")
        sys.exit(1)
    
    # Ensure the columns are numeric
    df['CycleSize'] = pd.to_numeric(df['CycleSize'], errors='coerce')
    df['CycleCount'] = pd.to_numeric(df['CycleCount'], errors='coerce')

    # Sort the DataFrame by cycle size (ascending)
    df_sorted = df.sort_values(by="CycleSize")

    # Create the bar plot
    plt.figure(figsize=(10, 6))
    plt.bar(df_sorted['CycleSize'].astype(int).astype(str), df_sorted['CycleCount'], color='skyblue')
    plt.xlabel("Cycle Size")
    plt.ylabel("Count")
    plt.title("Cycle Size Distribution")
    plt.tight_layout()
    plt.savefig(output_file)
    plt.close()
    print(f"Cycle size distribution plot saved to {output_file}")

def plot_token_cycle_frequency(csv_file, output_file, top_n=20):
    # Read the CSV file into a DataFrame
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Error reading {csv_file}: {e}")
        sys.exit(1)

    # Ensure frequency is numeric
    df['Frequency'] = pd.to_numeric(df['Frequency'], errors='coerce')

    # Sort by frequency in descending order
    df_sorted = df.sort_values(by="Frequency", ascending=False)

    # Optionally, take only the top N tokens for clarity
    if top_n is not None:
        df_sorted = df_sorted.head(top_n)

    # Create the bar plot
    plt.figure(figsize=(12, 8))
    plt.bar(df_sorted['Token'], df_sorted['Frequency'], color='salmon')
    plt.xlabel("Token")
    plt.ylabel("Cycle Frequency")
    plt.title(f"Token Cycle Frequency (Top {top_n} tokens)")
    plt.xticks(rotation=45, ha="right")
    plt.tight_layout()
    plt.savefig(output_file)
    plt.close()
    print(f"Token cycle frequency plot saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python plot_cycle_stats.py <cycle_size_count.csv> <token_cycle_frequency.csv>")
        sys.exit(1)

    cycle_csv = sys.argv[1]
    token_csv = sys.argv[2]

    plot_cycle_size_distribution(cycle_csv, "cycle_size_distribution.png")
    plot_token_cycle_frequency(token_csv, "token_cycle_frequency.png", top_n=20)