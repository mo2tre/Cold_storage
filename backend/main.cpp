#include <drogon/drogon.h>
#include <json/json.h>

#include <string>

int main(int argc, char* argv[])
{
    const std::string configPath = (argc > 1) ? argv[1] : "config/config.json";

    // GET /api/v1/health  ->  standard success envelope
    drogon::app().registerHandler(
        "/api/v1/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value body;
            body["success"] = true;
            body["data"]["status"] = "ok";
            body["data"]["service"] = "cold-storage-backend";
            body["message"] = "Operation completed";
            callback(drogon::HttpResponse::newHttpJsonResponse(body));
        },
        {drogon::Get});

    drogon::app().loadConfigFile(configPath);
    drogon::app().run();
    return 0;
}
