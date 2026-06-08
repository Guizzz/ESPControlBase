#include<request_manager.h>


RequestManager::RequestManager(WiFiServer *s)
{
    this->server=s;
}

void RequestManager::add_request(String method, String path, JsonDocument (*request_function)(JsonDocument param))
{
    Request new_request;
    new_request.method = method;
    new_request.path = path;
    new_request.request_function = request_function;
    requests_list.push_back(new_request);
}

String RequestManager::extract_path(String request)
{
    int first = request.indexOf(" ");
    if (first == -1) return "";
    int second = request.indexOf(" ", first + 1);
    if (second == -1) return "";
    String full = request.substring(first + 1, second);
    int qmark = full.indexOf("?");
    if (qmark == -1) return full;
    return full.substring(0, qmark);
}

String RequestManager::read_headers(WiFiClient* client, int* content_length, String* content_type)
{
    String headers = "";
    *content_length = 0;
    *content_type = "";

    while (client->connected())
    {
        String line = client->readStringUntil('\n');
        headers += line;
        if (line.startsWith("Content-Length: "))
        {
            *content_length = line.substring(16).toInt();
        }
        if (line.startsWith("Content-Type: "))
        {
            *content_type = line.substring(14);
            (*content_type).trim();
        }
        if (line == "\r" || line.length() == 0)
            break;
    }
    return headers;
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
    Serial.println(request);

    int content_length;
    String content_type;
    read_headers(&client, &content_length, &content_type);

    String body = "";
    if (content_length > 0)
    {
        for (int i = 0; i < content_length; i++)
        {
            while (!client.available()) { delay(1); }
            body += (char)client.read();
        }
    }

    JsonDocument params = parse_parameters(request, body, content_type);
    String path = extract_path(request);

    for(auto r: requests_list)
    {
        if (path == r.path && params["method"] == r.method) {

            JsonDocument resp = r.request_function(params);
            String out;
            serializeJson(resp, out);
            send_header(&client, !resp.containsKey("error"), "application/json");
            client.print(out);
            return;
        }
    }

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
        (*client).println("HTTP/1.1 500 Internal Server Error");
    else
        (*client).println("HTTP/1.1 200 OK");
    (*client).println("Content-Type: " + content_type);
    (*client).println("Connection: close");
    (*client).println("");
}



void parse_urlencoded(String input, JsonDocument& doc)
{
    while (input.length() > 0)
    {
        int amp = input.indexOf("&");
        String pair = (amp == -1) ? input : input.substring(0, amp);

        int eq = pair.indexOf("=");
        if (eq != -1)
        {
            String key = pair.substring(0, eq);
            String val = pair.substring(eq + 1);
            doc[key] = val;
        }

        if (amp == -1) break;
        input = input.substring(amp + 1);
    }
}

JsonDocument RequestManager::parse_parameters(String request, String body, String content_type)
{
    JsonDocument parsed;
    String method = request.substring(0, request.indexOf(" "));
    parsed["method"] = method;

    // Parsa query string
    int qmark = request.indexOf("?");
    if (qmark != -1)
    {
        int space = request.indexOf(" ", qmark);
        if (space == -1) space = request.length();
        String query = request.substring(qmark + 1, space);
        parse_urlencoded(query, parsed);
    }

    // Parsa body POST
    if (body.length() > 0)
    {
        if (content_type.indexOf("application/json") != -1)
        {
            JsonDocument body_doc;
            DeserializationError err = deserializeJson(body_doc, body);
            if (!err)
            {
                for (auto kv : body_doc.as<JsonObject>())
                    parsed[kv.key().c_str()] = kv.value();
            }
        }
        else
        {
            parse_urlencoded(body, parsed);
        }
    }

    return parsed;
}