#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <map>
#include <string>
#include <vector>

#define TRADINGVIEW_ORIGIN "tradingview.com"
#define TRADINGVIEW_WS "data.tradingview.com"
#define TRADINGVIEW_WS_ENDPOINT "/socket.io/websocket"

#define WSS_PORT "443"

enum class TimeRange {
  m_1,
  m_3,
  m_5,
  m_15,
  m_30,
  m_45,
  H_1,
  H_2,
  H_3,
  H_4,
  D_1,
  W_1,
  M_1,
  M_3,
  M_12
};

class TVParser {
public:
  TVParser(const std::string &, const std::string &, const std::string &,
           const std::string &);

  TVParser(const TVParser &) = delete;
  TVParser(TVParser &&) = delete;

  ~TVParser();

  TVParser &operator=(const TVParser &) = delete;
  TVParser &operator=(TVParser &&) = delete;

  std::string GetSymbolDataJson(const std::string &, TimeRange);
  std::string GetSymbolDataCSV(const std::string &, TimeRange);

private:
  static std::string GenRandomToken();

  void Connect();
  void CloseConnection();

  void Write(const std::string &data);
  void Read(boost::beast::flat_buffer &);

  static void AddHeader(std::string &);
  static std::string PrepareMessage(const std::string &,
                                    const boost::json::array &);

  static std::string JsonToCSV(const std::string &);

  std::string FetchSymbolData(const std::string &, TimeRange);
  std::string ReadParseData();

private:
  static const std::map<TimeRange, std::string> timeRangeToStringMap;

  boost::asio::io_context iocWS;
  boost::asio::ssl::context ctxWS;
  boost::asio::ip::tcp::resolver resolverWS;
  boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>
      websocket;

  std::string wsHost;
  std::string wsPort;
  std::string wsOrigin;
  std::string wsPath;
};
