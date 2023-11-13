# TVParser

TVParser is a tool for parsing TradingView chart data to retrieve the last 5000 ticks of OHLC data in JSON format for a specified symbol.

## Dependencies

1. Boost.Beast
2. Boost.json
3. OpenSSL

## Features

- Parses and retrieves JSON/CSV OHLC data for a given symbol.
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
    auto json = parser.GetSymbolDataJson("BTCUSD", TimeRange::m_1);
    auto csv = parser.GetSymbolDataCSV("BTCUSD", TimeRange::m_1);

    // Write the retrieved data to a JSON file named "data.json".
    std::ofstream jsonFile("data.json");
    jsonFile.write(json.data(), json.size());
    jsonFile.close();

    // Write the retrieved data to a CSV file named "data.csv".
    std::ofstream csvFile("data.csv");
    csvFile.write(csv.data(), csv.size());
    csvFile.close();

    return 0;
}
```

Or you can build it with cmake and use it from cmd, passing arguments to it:

./tvparser -s <symbolname> -t <filetype> -o <outputfilename>

Example:

./tvparser -s XAUUSD -t csv -o XAUUSD