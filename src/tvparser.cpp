#include "tvparser.hpp"
#include <random>
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

void TVParser::Connect() {
  if (!SSL_set_tlsext_host_name(websocket.next_layer().native_handle(),
                                wsHost.c_str())) {
    return;
  }

  auto const resolved = resolverWS.resolve(wsHost, wsPort);

  boost::asio::connect(websocket.next_layer().next_layer(), resolved.begin(),
                       resolved.end());

  websocket.next_layer().handshake(boost::asio::ssl::stream_base::client);

  websocket.set_option(boost::beast::websocket::stream_base::decorator(
      [wsOrigin = wsOrigin](
          boost::beast::http::request<boost::beast::http::empty_body> &req) {
        req.set(boost::beast::http::field::origin, wsOrigin.c_str());
      }));

  websocket.handshake(wsHost, wsPath);
}

void TVParser::Write(const std::string &data) {

  if (!websocket.is_open()) {
    return;
  }

  websocket.write(boost::asio::buffer(data));
}

void TVParser::Read() { websocket.read(dataBuffer); }

std::vector<char> TVParser::GetSymbolData(const std::string &symbol) {

  auto const sessionToken = "qs_" + GenRandomToken();
  auto const chartToken = "cs_" + GenRandomToken();

  return {};
}

int main() {
  TVParser parser("data.tradingview.com", "443", "https://www.tradingview.com",
                  "/socket.io/websocket");
}
