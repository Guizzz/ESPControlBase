#include<request_manager.h>


RequestManager::RequestManager(String ssid, String psw, WiFiServer *s)
{
    this->ssid=ssid;
    this->psw=psw;
    this->server=s;
}

void RequestManager::init_request()
{
  Serial.println("Starting...");
  Serial.println("Inizializing request manager");
  WiFi.begin(ssid, psw);
  Serial.print("Connecting to Wifi:");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("Server started");
  (*server).begin();

  Serial.print("IP Address of network: ");
  Serial.println(WiFi.localIP());
  Serial.print("Copy and paste the following URL: http://");
  Serial.print(WiFi.localIP().toString());
  Serial.println("/");

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

NetInfo RequestManager::get_net_info()
{
    NetInfo info;
    info.ssid= ssid;
    info.ip = WiFi.localIP().toString();
    return info;
}

void RequestManager::add_request(String method, String path, JsonDocument (*request_function)(JsonDocument param))
{
    Request new_request;
    new_request.method = method;
    new_request.path = path;
    new_request.request_function = request_function;
    requests_list.push_back(new_request);
}

void RequestManager::handle_request()
{
    WiFiClient client;

    client = (*server).accept();
    if (!client)
        return;

    Serial.println("Waiting for new client");

    while (!client.available()) {
        delay(1);
    }
    
    String request = client.readStringUntil('\r');

    client.flush();
    Serial.println(request);
    JsonDocument params = parse_parameters(request);

    for(auto r: requests_list)
    {
        if (request.indexOf(r.path) != -1 && params["method"] == r.method) {

            JsonDocument resp = r.request_function(params);
            String out;
            serializeJson(resp, out);
            send_header(&client, !resp.containsKey("error"), "application/json");
            client.print(out);
            return;
        }
    }

    if (request.indexOf(" / ") != -1 && params["method"] == "GET")
        web_page(&client);
    
    JsonDocument resp;
    resp["error"] = "Unknown request";
    String out;
    serializeJson(resp, out);
    send_header(&client, false, "application/json");
    client.print(out);
}

void RequestManager::send_header(WiFiClient *client, bool ok, String content_type)
{
    if(!ok)
        (*client).println("HTTP/1.1 500 OK");
    else
        (*client).println("HTTP/1.1 200 OK");
    (*client).println("Content-Type: " + content_type);
    (*client).println("");
}

void RequestManager::web_page(WiFiClient* client)
{   
    File file = SPIFFS.open("/basic.html", "r");
    //TODO: IMPLEMENT SENSOR READING 
    if(!file.available())
    {
        send_header(client, false, "text/html");
        (*client).println("ERROR");
        return;
    }

    String page = file.readString();
    // JsonDocument resp = (*pin_manager).status();
    // bool relay = resp["relay_status"].as<bool>();

    // page.replace("\%status\%", relay?"ON":"OFF");

    send_header(client, true, "text/html");
    (*client).println(page);

    delay(1);

    Serial.println("Client disconnected");
    Serial.println("");
}

JsonDocument RequestManager::parse_parameters(String request)
{
    JsonDocument parsed;
    String method = request.substring(0, request.indexOf(" "));
    parsed["method"] = method;

    if (request.indexOf("?") == -1)
        return parsed; 

    request = request.substring(request.indexOf(" ")+1, request.length());
    String parameters = request.substring(request.indexOf("?")+1, request.indexOf(" "));

    Serial.println("Parameters: " + parameters);
 
    while (parameters.indexOf("&") != -1)
    {
        
        String param_name = parameters.substring(0, parameters.indexOf("="));
        parsed[param_name] = parameters.substring(parameters.indexOf("=")+1, parameters.indexOf("&"));

        Serial.print("Param name: ");
        Serial.print(param_name);
        Serial.print(" value: ");
        String s = parsed[param_name];
        Serial.println(s);

        parameters = parameters.substring(parameters.indexOf("&")+1, parameters.length());
    }

    String param_name = parameters.substring(0, parameters.indexOf("="));
    parsed[param_name] = parameters.substring(parameters.indexOf("=")+1, parameters.length());

    Serial.print("Param name: ");
    Serial.print(param_name);
    Serial.print(" value: ");
    String s = parsed[param_name];
    Serial.println(s);

    return parsed;
}