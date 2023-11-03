#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <string>
#include <vector>

class TVParser {
public:
  TVParser(const std::string &, const std::string &, const std::string &,
           const std::string &);

  std::vector<char> GetSymbolData(const std::string &);

private:
  std::string GenRandomToken();

  void Connect();
  void CloseConnection();

  void Write(const std::string &data);
  void Read();

private:
  std::string wsHost;
  std::string wsPort;
  std::string wsOrigin;
  std::string wsPath;
  boost::beast::flat_buffer dataBuffer;
  boost::asio::io_context iocWS;
  boost::asio::ssl::context ctxWS;
  boost::asio::ip::tcp::resolver resolverWS;
  boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>
      websocket;
};
