#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <list>

struct Request
{
  String method;
  String path;
  JsonDocument (*request_function)(JsonDocument param);
};

class RequestManager
{
  WiFiServer* server;
  std::list<Request> requests_list;

  String extract_path(String request);
  String read_headers(WiFiClient* client, int* content_length, String* content_type);
  void send_header(WiFiClient* client, bool ok, String content_type);
  JsonDocument parse_parameters(String request, String body, String content_type);

  public:
    RequestManager(WiFiServer* s);
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