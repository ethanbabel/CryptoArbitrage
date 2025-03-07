# CryptoArbitrage

CryptoArbitrage is a project designed to detect circular arbitrage opportunities between different cryptocurrency tokens. The goal is to identify price inneficiencies in the market using Bellman Ford. 

## Features

- **Real-time Data Fetching**: Continuously fetches swap quote data for cryptocurrency tokens.
- **Arbitrage Detection**: Identifies potential arbitrage opportunities by representing the token universe as a log transformed raph, then using Bellman Ford to detect negative weight cycles. Such cycles are then arbitrage opportunities.

## Future Additions 

- **Automated Trading**: Executing trades automatically when profitable arbitrage opportunities are detected.
- **Gas Cost Calculation**: Account for gas costs when considering and arbitrage opportunity. 
- **Re-write Without API**: API throttling limitations mean that it is extremely unlikely that this implementation is ever fast enough to detect arbitrage. 

