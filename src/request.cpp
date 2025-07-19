#include "request.hpp"
#include <curl/curl.h>
#include <sstream>
#include <thread>

extern HSQUIRRELVM v;

namespace SqRequest
{

    moodycamel::ReaderWriterQueue<HttpResponse> responseQueue;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
    {
        output->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    Request::Request(const std::string &method)
        : method_(method), vm_(nullptr) {}

    Request::Request() : method_("GET"), vm_(nullptr) {}

    Request::~Request() {}

    Request &Request::setURL(const std::string &url)
    {
        url_ = url;
        return *this;
    }

    Request &Request::setHeader(const std::string &key, const std::string &value)
    {
        headers_.push_back(key + ": " + value);
        return *this;
    }

    Request &Request::setTag(const std::string &tag)
    {
        tag_ = tag;
        return *this;
    }

    Request &Request::setBody(const std::string &body)
    {
        body_ = body;
        return *this;
    }

    void Request::send()
    {
        OutputDebug("Sending async HTTP request [%s]\n", method_.c_str());

        std::string method = method_;
        std::string url = url_;
        std::vector<std::string> headers = headers_;
        std::string body = body_;
        std::string tag = tag_;

        std::thread([method, url, headers, body, tag]()
                    {
        try {
            CURL *curl = curl_easy_init();
            if (!curl)
            {
                OutputDebug("curl_easy_init failed!\n");
                return;
            }

            std::string response_string;
            long response_code = 0;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

            struct curl_slist *chunk = nullptr;
            for (const auto &header : headers)
                chunk = curl_slist_append(chunk, header.c_str());

            if (!body.empty())
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            }

            if (chunk)
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK)
            {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                HttpResponse resp{tag, url, response_code, response_string};
                OutputDebug("Request to %s completed with code %ld\n", url.c_str(), response_code);
                responseQueue.enqueue(resp);
            }
            else
            {
                OutputDebug("curl_easy_perform error: %s\n", curl_easy_strerror(res));
            }

            if (chunk)
                curl_slist_free_all(chunk);
            curl_easy_cleanup(curl);
        } catch (const std::exception& e) {
            OutputDebug("Exception in HTTP thread: %s\n", e.what());
        } })
            .detach();
    }

    void SqRequest::Request::Process()
    {
        if (!v)
        {
            OutputDebug("SqRequest: Squirrel VM is null. Skipping Process.\n");
            return;
        }

        HttpResponse resp;
        while (responseQueue.try_dequeue(resp))
        {
            OutputDebug("SqRequest: Processing HTTP response...\n");
            OutputDebug("Tag: %s, URL: %s, StatusCode: %d\n", resp.tag.c_str(), resp.url.c_str(), resp.statusCode);

            try
            {
                Sqrat::Object root = Sqrat::RootTable(v);
                if (!root.IsNull())
                {
                    OutputDebug("SqRequest: Root table was successful");
                    Sqrat::Function cb = Sqrat::RootTable(v).GetFunction(_SC("Request_OnResponse"));
                    if (!cb.IsNull())
                    {
                        OutputDebug("SqRequest: tag: %s (len=%zu)", resp.tag.c_str(), resp.tag.length());
                        OutputDebug("SqRequest: url: %s (len=%zu)", resp.url.c_str(), resp.url.length());
                        OutputDebug("SqRequest: response: %s (len=%zu)", resp.response.c_str(), resp.response.length());
                        cb.Execute(resp.tag, resp.url, resp.statusCode, resp.response);
                    }
                    else
                    {
                        OutputDebug("SqRequest: Callback Request_OnResponse not found in RootTable.\n");
                    }
                }
                else
                {
                    OutputDebug("SqRequest: RootTable is null.\n");
                }
            }
            catch (const Sqrat::Exception &e)
            {
                OutputDebug("SqRequest: Callback execution failed: %s\n", e.Message().c_str());
            }
        }
    }

    // Factory methods
    Request *GET() { return new Request("GET"); }
    Request *POST() { return new Request("POST"); }
    Request *PUT() { return new Request("PUT"); }
    Request *DELETE_() { return new Request("DELETE"); }

    void SqRequest::Request::Register_SqRequest(Sqrat::Table tb)
    {
        using namespace Sqrat;

        tb.Func(_SC("GET"), &GET);
        tb.Func(_SC("POST"), &POST);
        tb.Func(_SC("PUT"), &PUT);
        tb.Func(_SC("DELETE"), &DELETE_);

        tb.Bind(_SC("Request"),
                Class<Request>(v, _SC("Request"))
                    .Func(_SC("setURL"), &Request::setURL)
                    .Func(_SC("setHeader"), &Request::setHeader)
                    .Func(_SC("setBody"), &Request::setBody)
                    .Func(_SC("setTag"), &Request::setTag)
                    .Func(_SC("send"), &Request::send));
    }
}
