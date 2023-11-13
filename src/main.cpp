#include "tvparser.hpp"
#include <iostream>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " -s <symbolname> -t <filetype> -o <outputfilename>" << std::endl;
        return 1;
    }

    std::string symbolName, fileType, outputFileName;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-s") == 0) {
            symbolName = argv[i + 1];
        } else if (strcmp(argv[i], "-t") == 0) {
            fileType = argv[i + 1];
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFileName = argv[i + 1];
        } else {
            std::cerr << "Invalid argument: " << argv[i] << std::endl;
            return 1;
        }
    }

    if (symbolName.empty() || fileType.empty() || outputFileName.empty()) {
        std::cerr << "Missing required arguments. Usage: " << argv[0] << " -s <symbolname> -t <filetype> -o <outputfilename>" << std::endl;
        return 1;  // Return an error code
    }

    if (fileType != "json" && fileType != "csv") {
        std::cerr << "Invalid filetype. It must be either 'json' or 'csv'." << std::endl;
        return 1;
    }

    auto parser = TVParser(TRADINGVIEW_WS, WSS_PORT, TRADINGVIEW_ORIGIN, TRADINGVIEW_WS_ENDPOINT);
    auto data = std::string{};

    if(fileType == "json"){
    	data = parser.GetSymbolDataJson(symbolName, TimeRange::m_1);
    }
    else{
    	data = parser.GetSymbolDataCSV(symbolName, TimeRange::m_1);
    }

    std::ofstream file(outputFileName + "." + fileType);
    file.write(data.data(), data.size());
    file.close();

    return 0;
}
