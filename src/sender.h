#include <boost/asio.hpp>
// #include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
typedef ssl::stream<tcp::socket> ssl_socket;

#define MIN(a,b) ((a) < (b) ? (a) : (b))

int sendPhotoToTelegram(string filename, string botCreds, string telegramChannelName) {
    // long long fileSize = boost::filesystem::file_size(filename);

    //Read file into memory
    FILE* pFile = fopen(filename.c_str(), "rb");
    if (pFile == NULL) {fputs ("File error",stderr); exit (1);}
    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    long fileSize = ftell (pFile);
    rewind (pFile);

    char * charArray = (char*) malloc (sizeof(char)*fileSize);
    if (charArray == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    size_t result = fread (charArray,1,fileSize,pFile);
    if (result != fileSize) {fputs ("Reading error",stderr); exit (3);}
    /* the whole file is now loaded in the memory buffer. */

    // terminate
    fclose (pFile);
    free (charArray);

    //Setup Socket
    boost::asio::io_service io_service;
    tcp::endpoint ep;
    ep.port(443);
    ep.address(boost::asio::ip::address_v4::from_string("https://api.telegram.org"));

    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ssl_socket sslSocket(io_service, ctx);
    sslSocket.lowest_layer().connect(ep);
    sslSocket.set_verify_mode(boost::asio::ssl::verify_none);
    sslSocket.handshake(ssl_socket::client);

    string PREFIX = "--";
    //Use GUID as boundary
    string BOUNDARY = boost::uuids::to_string(boost::uuids::random_generator()());
    string NEWLINE = "\r\n";
    int NEWLINE_LENGTH = NEWLINE.length();

    string contentMessage = string("Content-Disposition: form-data; name=\"fmChunk\"; filename=\"" + filename + "\"");

    //Calculate length of entire HTTP request - goes into header
    long long lengthOfRequest = 0;
    lengthOfRequest += PREFIX.length() + BOUNDARY.length() + NEWLINE_LENGTH;
    lengthOfRequest += contentMessage.length();
    lengthOfRequest += NEWLINE_LENGTH + NEWLINE_LENGTH;
    lengthOfRequest += fileSize;
    lengthOfRequest += NEWLINE_LENGTH + PREFIX.length() + BOUNDARY.length() + PREFIX.length() + NEWLINE_LENGTH;

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    string URL = "/" + botCreds + "/sendPhoto?chat_id=" + telegramChannelName;
    request_stream << "POST " + URL + " HTTP/1.1" << NEWLINE;
    request_stream << "Host: localhost" << NEWLINE; // << ":" << port << NEWLINE;
    request_stream << "User-Agent: FilemailDesktop2Cpp" << NEWLINE;
    request_stream << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" << NEWLINE;
    request_stream << "Accept-Language: nb,no;q=0.8,nn;q=0.6,en-us;q=0.4,en;q=0.2" << NEWLINE;
    request_stream << "Accept-Encoding: gzip;q=0,deflate;q=0" << NEWLINE; //Disables compression
    request_stream << "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7" << NEWLINE;
    request_stream << "Connection: close" << NEWLINE;
    request_stream << "Content-Length: " << lengthOfRequest << NEWLINE;
    request_stream << "Content-Type: multipart/form-data; boundary=" << BOUNDARY << NEWLINE;
    request_stream << NEWLINE;

    request_stream << PREFIX;
    request_stream << BOUNDARY;
    request_stream << NEWLINE;
    request_stream << contentMessage;
    request_stream << NEWLINE;
    request_stream << NEWLINE;

    auto data = request.data();
    sslSocket.write_some(buffer(data));

    //Send Data (Paytload)
    auto bytesSent = 0;
    while (bytesSent < fileSize)
    {
        int bytesToSendNow = MIN(fileSize - bytesSent, 1024);
        sslSocket.write_some(boost::asio::buffer(charArray + bytesSent, bytesToSendNow));
        bytesSent += bytesToSendNow;
    }

    //Close request
    sslSocket.write_some(boost::asio::buffer(NEWLINE));
    sslSocket.write_some(boost::asio::buffer(PREFIX));
    sslSocket.write_some(boost::asio::buffer(BOUNDARY));
    sslSocket.write_some(boost::asio::buffer(PREFIX));
    sslSocket.write_some(boost::asio::buffer(NEWLINE));


    //Read Response
    boost::asio::streambuf response;
    read_until(sslSocket, response, "\r\n");
    string strResponse(boost::asio::buffer_cast<const char*>(response.data()), response.size());

    //Check Response
    if (strResponse.find("200 OK") != string::npos){
        // cout << "OK";
        return 0;
    }
    else
    {
        // BOOST_FAIL("Upload failed");
        return 1;
    }
}
