import csv
import requests
import subprocess
import time
from dotenv import load_dotenv
import os
from itertools import combinations

load_dotenv()

# 1inch API key
ONE_INCH_API_KEY = os.getenv("ONEINCH_API_KEY")

# 1inch API URL
ONE_INCH_URL = "https://api.1inch.dev/token/v1.2/1/search"

# Uniswap V3 Factory contract address
UNISWAP_V3_FACTORY = "0x1F98431c8aD98523631AE4a59f267346ea31F984"

# Cast RPC URL
RPC_URL = "https://eth.drpc.org"

# Uniswap V3 fee tiers (0.01%, 0.05%, 0.3%, 1%)
FEE_TIERS = [100, 500, 3000, 10000]

# Headers for 1inch API request
HEADERS = {
    "Authorization": "Bearer " + ONE_INCH_API_KEY,
    "X-API-Key": ONE_INCH_API_KEY
}

# Load token symbols from tokens.txt
with open("tokens.txt", "r") as file:
    tokens = [line.strip() for line in file.readlines()]

print(f"✅ Loaded {len(tokens)} token symbols. Fetching token data...")

# Data storage
token_data = []
token_addresses = {}
fee_tier_data = []

# Get token addresses and decimals
for token_symbol in tokens:
    try:
        params = {"query": token_symbol, "limit": 1}
        response = requests.get(ONE_INCH_URL, headers=HEADERS, params=params)
        time.sleep(1)  # Respect API rate limits

        if response.status_code != 200:
            print(f"⚠️ Request failed for {token_symbol} | Status Code: {response.status_code}")
            continue
        
        data = response.json()
        if not data:
            print(f"❌ No address found for {token_symbol}")
            continue
        
        token_address = data[0]["address"]
        decimals = data[0]["decimals"]
        token_addresses[token_symbol] = token_address  # Store address for later use
        token_data.append((token_symbol, token_address, decimals))

        print(f"🔹 {token_symbol} → Address: {token_address}, Decimals: {decimals}")

    except Exception as e:
        print(f"⚠️ Error processing {token_symbol}: {e}")

# Compute fee tiers and get for every possible token pair
token_pairs = [(a, b) for a in tokens for b in tokens if a != b]  # Generate all token pairs

print(f"🔄 Checking fee tiers for {len(token_pairs)} token pairs...")

for base_token, quote_token in token_pairs:
    try:
        base_address = token_addresses.get(base_token)
        quote_address = token_addresses.get(quote_token)

        if not base_address or not quote_address:
            print(f"⚠️ Skipping {base_token} ↔ {quote_token} due to missing data")
            continue

        selected_fee_tier = None

        for fee in FEE_TIERS:
            command = [
                "cast", "call", UNISWAP_V3_FACTORY,
                "getPool(address,address,uint24)(address)",
                base_address, quote_address, str(fee),
                "--rpc-url", RPC_URL
            ]
            result = subprocess.run(command, capture_output=True, text=True)
            pool_address = result.stdout.strip()

            if pool_address and pool_address.lower() != "0x0000000000000000000000000000000000000000":
                print(f"✅ {base_token} ↔ {quote_token} has Uniswap V3 pool with {fee / 10000:.2%} fee (Pool: {pool_address})")
                selected_fee_tier = fee
                break  # Use the **lowest** valid fee tier

        if selected_fee_tier:
            fee_tier_data.append((base_token, quote_token, selected_fee_tier))

        else:
            print(f"❌ No Uniswap V3 pool found for {base_token} ↔ {quote_token}")

    except Exception as e:
        print(f"⚠️ Error checking {base_token} ↔ {quote_token}: {e}")

# Save results to token_info.csv
with open("token_info.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["symbol", "address", "decimals"])
    writer.writerows(token_data)

# Save fee tiers to token_fee_tiers.csv
with open("token_fee_tiers.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["base_currency", "quote_currency", "fee_tier"])
    writer.writerows(fee_tier_data)

print(f"✅ Updated token_info.csv ({len(token_data)} tokens) and token_fee_tiers.csv ({len(fee_tier_data)} fee tiers).")