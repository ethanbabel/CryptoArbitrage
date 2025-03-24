# CryptoArbitrage

CryptoArbitrage is a project designed to detect circular arbitrage opportunities between different cryptocurrency tokens. The goal is to identify price inneficiencies in the market using Bellman Ford. 

## Features

- **Real-time Data Fetching**: Continuously uses dRPC eth_call() to get swap quotes from Uniswap V3 router.
- **Arbitrage Detection**: Identifies potential arbitrage opportunities by representing the token universe as a log transformed graph, then using Bellman Ford to detect negative weight cycles. Such cycles are then arbitrage opportunities.

## Future Additions

- **Automated Trading**: Executing trades automatically when profitable arbitrage opportunities are detected.
- **Gas Cost Calculation**: Account for gas costs when considering and arbitrage opportunity. 

