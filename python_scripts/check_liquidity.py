import os
import json
import time
import pandas as pd
from dotenv import load_dotenv
from web3 import Web3

# Load environment variables 
load_dotenv()
RPC_URL = os.getenv("RPC_URL")  # Alchemy URL
web3 = Web3(Web3.HTTPProvider(RPC_URL))

# Uniswap V3 Factory and Pool ABI (trimmed to relevant methods)
UNISWAP_V3_FACTORY = "0x1F98431c8aD98523631AE4a59f267346ea31F984"
UNISWAP_V3_POOL_ABI = '[{"constant":true,"inputs":[],"name":"liquidity","outputs":[{"name":"","type":"uint128"}],"stateMutability":"view","type":"function"}]'
UNISWAP_V3_FACTORY_ABI = '[{"constant":true,"inputs":[{"name":"tokenA","type":"address"},{"name":"tokenB","type":"address"},{"name":"fee","type":"uint24"}],"name":"getPool","outputs":[{"name":"pool","type":"address"}],"stateMutability":"view","type":"function"}]'

# Load token pairs and fee tiers from CSV
fee_tiers_df = pd.read_csv("token_fee_tiers.csv")

# Load token info from CSV
token_info_df = pd.read_csv("token_info.csv", index_col="symbol")

# Store problematic pairs
failed_pairs = []

# Load Uniswap Factory contract
factory_contract = web3.eth.contract(address=UNISWAP_V3_FACTORY, abi=json.loads(UNISWAP_V3_FACTORY_ABI))

# Iterate over each token pair
for index, row in fee_tiers_df.iterrows():
    base_symbol, quote_symbol, fee_tier = row["base_currency"], row["quote_currency"], int(row["fee_tier"])

    try:
        print(f"🔍 Checking {base_symbol} ↔ {quote_symbol} at {fee_tier / 1e6:.2%} fee tier...")

        # Get token addresses from token_info.csv, convert token addresses to checksum format
        base_address = Web3.to_checksum_address(token_info_df.loc[base_symbol, "address"])
        quote_address = Web3.to_checksum_address(token_info_df.loc[quote_symbol, "address"])

        # Call Uniswap V3 factory to get pool address
        pool_address = factory_contract.functions.getPool(base_address, quote_address, fee_tier).call()

        if pool_address == "0x0000000000000000000000000000000000000000":
            print(f"❌ No pool found for {base_symbol} ↔ {quote_symbol}.")
            failed_pairs.append((base_symbol, quote_symbol, fee_tier))
            continue

        # Load Uniswap Pool Contract
        pool_contract = web3.eth.contract(address=pool_address, abi=json.loads(UNISWAP_V3_POOL_ABI))

        # Get liquidity
        liquidity = pool_contract.functions.liquidity().call()

        if liquidity == 0:
            print(f"⚠️ Pool exists but has **zero liquidity** for {base_symbol} ↔ {quote_symbol}.")
            failed_pairs.append((base_symbol, quote_symbol, fee_tier))
        else:
            print(f"✅ Pool exists with liquidity: {liquidity}")

        time.sleep(1)  # Avoid rate limiting

    except Exception as e:
        print(f"⚠️ Error checking {base_symbol} ↔ {quote_symbol}: {e}")
        failed_pairs.append((base_symbol, quote_symbol, fee_tier))

# Save failed pairs to CSV
if failed_pairs:
    failed_df = pd.DataFrame(failed_pairs, columns=["base currency", "quote currency", "fee tier"])
    failed_df.to_csv("failed_pairs.csv", index=False)
    print("\n🚨 Some pairs failed liquidity checks. See 'failed_pairs.csv'.")
    token_fee_tiers_df = pd.read_csv("token_fee_tiers.csv")
    token_fee_tiers_df = token_fee_tiers_df[~token_fee_tiers_df.apply(tuple, 1).isin(failed_pairs)]
    token_fee_tiers_df.to_csv("token_fee_tiers.csv", index=False)
else:
    print("\n✅ All token pairs have liquidity!")