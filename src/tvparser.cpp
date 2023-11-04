#include "tvparser.hpp"
#include <boost/json.hpp>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#define TOKEN_LENGTH 12

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

  for (auto i = 0; i < 12; ++i) {
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

void TVParser::Read() { websocket.read(dataBuffer); }

std::vector<char> TVParser::ReadParseData() { return {}; }

std::vector<char> TVParser::GetSymbolData(const std::string &symbol) {

  auto const sessionToken = "qs_" + GenRandomToken();
  auto const chartToken = "cs_" + GenRandomToken();

  const static boost::json::array fieldsMsg{"ch",
                                            "chp",
                                            "current_session",
                                            "description",
                                            "local_description",
                                            "language",
                                            "exchange",
                                            "fractional",
                                            "is_tradable",
                                            "lp",
                                            "lp_time",
                                            "minmov",
                                            "minmove2",
                                            "original_name",
                                            "pricescale",
                                            "pro_name",
                                            "short_name",
                                            "type",
                                            "update_mode",
                                            "volume",
                                            "currency_code",
                                            "rchp",
                                            "rtc"};

  Connect();

  if (!websocket.is_open()) {
    return {};
  }

  boost::json::array sessionFieldsMsg;
  sessionFieldsMsg.reserve(fieldsMsg.size() + 1);
  sessionFieldsMsg.emplace_back(sessionToken);
  sessionFieldsMsg.insert(sessionFieldsMsg.end(), fieldsMsg.begin(),
                          fieldsMsg.end());

  const boost::json::object flagsObj{
      {"flags", boost::json::array{"force_permission"}}};

  const auto setAuthTokenMsg =
      PrepareMessage("set_auth_token", {"unauthorized_user_token"});
  const auto chartCreateSessionMsg =
      PrepareMessage("chart_create_session", {chartToken, ""});

  const auto quoteCreateSessionMsg =
      PrepareMessage("quote_create_session", {sessionToken});
  const auto quoteSetFieldsMsg =
      PrepareMessage("quote_set_fields", sessionFieldsMsg);
  const auto quoteAddSymbolsMsg =
      PrepareMessage("quote_add_symbols", {sessionToken, symbol});
  const auto quoteFastSymbolsMsg =
      PrepareMessage("quote_fast_symbols", {sessionToken, symbol});
  const auto resolveSymbolsMsg =
      PrepareMessage("resolve_symbol",
                     {chartToken, "symbol_1",
                      R"(={"symbol":")" + symbol +
                          R"(","adjustment":"splits","session":"extended"})"});
  const auto createSeriesSymbolMsg = PrepareMessage(
      "create_series", {chartToken, "s1", "s1", "symbol_1", "1", 5000});

  Write(setAuthTokenMsg);
  Write(chartCreateSessionMsg);
  Write(quoteCreateSessionMsg);
  Write(quoteSetFieldsMsg);
  Write(quoteAddSymbolsMsg);
  Write(quoteFastSymbolsMsg);
  Write(resolveSymbolsMsg);
  Write(createSeriesSymbolMsg);

  return ReadParseData();
}
