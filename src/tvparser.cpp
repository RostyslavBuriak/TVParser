#include "tvparser.hpp"
#include <algorithm>
#include <boost/json.hpp>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>

#define TOKEN_LENGTH 12
#define READ_NUM 3

TVParser::TVParser(const std::string &host, const std::string &port,
                   const std::string &origin, const std::string &path)
    : wsHost(host), wsPort(port), wsOrigin(origin), wsPath(path),
      ctxWS(boost::asio::ssl::context::tlsv12_client), resolverWS(iocWS),
      websocket(iocWS, ctxWS) {}

std::string TVParser::GenRandomToken() {
  std::random_device rD;
  std::mt19937 gen(rD());
  std::uniform_int_distribution<int> dis(0, 25);

  std::string token;

  for (auto i = 0; i < TOKEN_LENGTH; ++i) {
    token.push_back('a' + dis(gen));
  }

  return token;
}

TVParser::~TVParser() { CloseConnection(); }

void TVParser::Connect() {
  if (!SSL_set_tlsext_host_name(websocket.next_layer().native_handle(),
                                wsHost.c_str())) {
    return;
  }

  try {
    auto const resolved = resolverWS.resolve(wsHost, wsPort);

    boost::asio::connect(websocket.next_layer().next_layer(), resolved.begin(),
                         resolved.end());

    websocket.next_layer().handshake(boost::asio::ssl::stream_base::client);

    websocket.set_option(boost::beast::websocket::stream_base::decorator(
        [wsOrigin = wsOrigin](
            boost::beast::http::request<boost::beast::http::empty_body> &req) {
          req.set(boost::beast::http::field::origin, wsOrigin);
        }));

    websocket.handshake(wsHost, wsPath);
  } catch (...) {
    if (websocket.is_open()) {
      CloseConnection();
    }
  }
}

void TVParser::AddHeader(std::string &msg) {
  msg = "~m~" + std::to_string(msg.length()) + "~m~" + msg;
}

std::string TVParser::PrepareMessage(const std::string &func,
                                     const boost::json::array &paramList) {

  boost::json::object msg;
  msg["m"] = func;
  msg["p"] = paramList;

  auto sBody = boost::json::serialize(msg);
  AddHeader(sBody);

  return sBody;
}

void TVParser::CloseConnection() {
  if (websocket.is_open()) {
    websocket.close(boost::beast::websocket::close_code::normal);
  }
}

void TVParser::Write(const std::string &data) {

  if (!websocket.is_open()) {
    return;
  }

  websocket.write(boost::asio::buffer(data));
}

void TVParser::Read(boost::beast::flat_buffer &dataBuffer) {
  websocket.read(dataBuffer);
}

std::vector<char> TVParser::ReadParseData() {

  std::vector<char> parsedData;

  try {
    std::regex errPattern("error");
    std::regex lenPattern("~m~(\\d+)~m~");
    std::regex msgPattern("\"m\":\"([a-zA-Z0-9_]+)\"");

    auto n = READ_NUM;
    while (n--) {
      boost::beast::flat_buffer dataBuffer;
      Read(dataBuffer);
      auto cData = static_cast<char *>(dataBuffer.data().data());
      auto size = dataBuffer.data().size();

      if (std::regex_search(cData, cData + size, errPattern)) {
        std::cout << "Error occured: " << std::string(cData, size) << std::endl;
        return {};
      }

      std::cmatch lenMatch;

      while (std::regex_search(cData, cData + size, lenMatch, lenPattern)) {
        auto match = lenMatch[1].first;
        auto number = std::stoull(match);
        auto pos = lenMatch[0].second - cData;

        cData += pos;

        if (size >= number + pos) {
          std::cmatch msgMatch;

          if (std::regex_search(cData, cData + number, msgMatch, msgPattern)) {
            if (msgMatch[1] == "timescale_update") {
              parsedData.resize(number);
              std::memcpy(parsedData.data(), cData, number);
              return parsedData;
            }
          }
        } else {
          std::cout << "Data is corrupted\n";
          return {};
        }

        cData += number;
      }
    }
  } catch (...) {
    return {};
  }

  return parsedData;
}

std::vector<char> TVParser::GetSymbolData(const std::string &symbol) {

  auto const sessionToken = "qs_" + GenRandomToken();
  auto const chartToken = "cs_" + GenRandomToken();

  Connect();

  if (!websocket.is_open()) {
    return {};
  }

  const auto setAuthTokenMsg =
      PrepareMessage("set_auth_token", {"unauthorized_user_token"});
  const auto chartCreateSessionMsg =
      PrepareMessage("chart_create_session", {chartToken, ""});
  const auto quoteCreateSessionMsg =
      PrepareMessage("quote_create_session", {sessionToken});
  const auto quoteAddSymbolsMsg =
      PrepareMessage("quote_add_symbols", {sessionToken, symbol});
  const auto quoteFastSymbolsMsg =
      PrepareMessage("quote_fast_symbols", {sessionToken, symbol});
  const auto resolveSymbolsMsg =
      PrepareMessage("resolve_symbol", {chartToken, "symbol_1",
                                        R"(={"symbol":")" + symbol + "\"}"});
  const auto createSeriesSymbolMsg = PrepareMessage(
      "create_series", {chartToken, "s1", "s1", "symbol_1", "1", 5000});

  try {
    Write(setAuthTokenMsg);
    Write(chartCreateSessionMsg);
    Write(quoteCreateSessionMsg);
    Write(quoteAddSymbolsMsg);
    Write(resolveSymbolsMsg);
    Write(createSeriesSymbolMsg);
  } catch (...) {
    return {};
  }

  return ReadParseData();
}
