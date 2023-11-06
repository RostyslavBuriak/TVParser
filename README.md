# TVParser

TVParser is a tool for parsing TradingView chart data to retrieve the last 5000 ticks of OHLC data in JSON format for a specified symbol.

## Dependencies

1. Boost.Beast
2. Boost.json
3. OpenSSL

## Features

- Parses and retrieves JSON OHLC data for a given symbol.
- Works with the last 5000 ticks of trading data.

## Sample Usage

To use TVParser, you can include the "tvparser.hpp" header and follow this sample usage:

```cpp
#include "tvparser.hpp"
#include <fstream>

int main() {
  // Create an instance of TVParser with WebSocket and endpoint configuration.
  // All defines are defined in tvparser.hpp file
  TVParser parser(TRADINGVIEW_WS, WSS_PORT, TRADINGVIEW_ORIGIN, TRADINGVIEW_WS_ENDPOINT);

  // Get OHLC data for the "BTCUSD" symbol with a time range of 1 minute.
  auto v = parser.GetSymbolData("BTCUSD", TimeRange::m_1);

  // Write the retrieved data to a JSON file named "data.json".
  std::ofstream s("data.json");
  s.write(v.data(), v.size());
  s.close();

  return 0;
}
