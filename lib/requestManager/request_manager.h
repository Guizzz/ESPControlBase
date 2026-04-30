#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <list>

struct Request
{
  String method;
  String path;
  JsonDocument (*request_function)(JsonDocument param);
};

struct NetInfo
{
  String ssid;
  String ip;
};

class RequestManager
{
  String ssid, psw;
  WiFiServer* server;
  std::list<Request> requests_list;

  void send_header(WiFiClient* client, bool ok, String content_type);
  void web_page(WiFiClient* client);
  JsonDocument parse_parameters(String request);

  public:
    RequestManager(String ssid, String psw, WiFiServer* s);
    void init_request();
    NetInfo get_net_info();
    /*
    Add request to be handled by API server:

    method: specify the method of the request
    path: specify the path of the request
    request_function: define the function to be called to manage the request
    parm: param to pass to the request_function
    */
    void add_request(String method, String path, JsonDocument (*request_function)(JsonDocument param));
    void handle_request();
};