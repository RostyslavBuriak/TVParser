#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <string>
#include <vector>

class TVParser {
public:
  TVParser(const std::string &, const std::string &, const std::string &,
           const std::string &);

  ~TVParser();

  std::vector<char> GetSymbolData(const std::string &);
  void Read(boost::beast::flat_buffer &);

private:
  static std::string GenRandomToken();

  void Connect();
  void CloseConnection();

  void Write(const std::string &data);

  static void AddHeader(std::string &);
  std::string PrepareMessage(const std::string &, const boost::json::array &);

  std::vector<char> ReadParseData();

private:
  std::string wsHost;
  std::string wsPort;
  std::string wsOrigin;
  std::string wsPath;
  boost::asio::io_context iocWS;
  boost::asio::ssl::context ctxWS;
  boost::asio::ip::tcp::resolver resolverWS;
  boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>
      websocket;
};
