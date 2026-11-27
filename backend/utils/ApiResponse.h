#pragma once
// Standard JSON envelopes (see project plan, section 6).
//   Success: { "success": true,  "data": {...}, "message": "..." }
//   Error:   { "success": false, "error": { "code": "...", "message": "...", "details": [] } }

#include <drogon/HttpResponse.h>
#include <json/json.h>

#include <string>

namespace api
{
inline drogon::HttpResponsePtr ok(const Json::Value& data = Json::Value(Json::objectValue),
                                  const std::string& message = "Operation completed",
                                  drogon::HttpStatusCode status = drogon::k200OK)
{
    Json::Value body;
    body["success"] = true;
    body["data"] = data;
    body["message"] = message;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(status);
    return resp;
}

inline drogon::HttpResponsePtr created(const Json::Value& data,
                                       const std::string& message = "Created")
{
    return ok(data, message, drogon::k201Created);
}

inline drogon::HttpResponsePtr error(drogon::HttpStatusCode status,
                                     const std::string& code,
                                     const std::string& message,
                                     const Json::Value& details = Json::Value(Json::arrayValue))
{
    Json::Value body;
    body["success"] = false;
    body["error"]["code"] = code;
    body["error"]["message"] = message;
    body["error"]["details"] = details;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(status);
    return resp;
}

inline drogon::HttpResponsePtr validationError(const std::string& message,
                                               const Json::Value& details = Json::Value(Json::arrayValue))
{
    return error(drogon::k422UnprocessableEntity, "VALIDATION_ERROR", message, details);
}

inline drogon::HttpResponsePtr badRequest(const std::string& message,
                                          const Json::Value& details = Json::Value(Json::arrayValue))
{
    return error(drogon::k400BadRequest, "BAD_REQUEST", message, details);
}

inline drogon::HttpResponsePtr conflict(const std::string& message)
{
    return error(drogon::k409Conflict, "CONFLICT", message);
}

inline drogon::HttpResponsePtr notFound(const std::string& message = "Resource not found")
{
    return error(drogon::k404NotFound, "NOT_FOUND", message);
}

inline drogon::HttpResponsePtr unauthorized(const std::string& message = "Authentication required")
{
    return error(drogon::k401Unauthorized, "UNAUTHORIZED", message);
}

inline drogon::HttpResponsePtr forbidden(const std::string& message = "Permission denied")
{
    return error(drogon::k403Forbidden, "FORBIDDEN", message);
}

inline drogon::HttpResponsePtr internalError(const std::string& message = "Internal server error")
{
    return error(drogon::k500InternalServerError, "INTERNAL_ERROR", message);
}
}  // namespace api
