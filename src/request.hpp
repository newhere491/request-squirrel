#pragma once
#include <vector>
#include <readerwriterqueue.h>
#include "includes.hpp"

namespace SqRequest
{
    struct HttpResponse
    {
        std::string tag;
        std::string url;
        long statusCode;
        std::string response;
    };
    class Request
    {
    public:
        Request();
        Request(const std::string &method);
        ~Request();

        Request &setURL(const std::string &url);
        Request &setHeader(const std::string &key, const std::string &value);
        Request &setTag(const std::string &tag);

        Request &setBody(const std::string &body);
        void send();

        static void Process();
        static void Register_SqRequest(Sqrat::Table tb);

    private:
        std::string method_;
        std::string url_;
        std::vector<std::string> headers_;
        std::string body_;
        std::string tag_;
        HSQUIRRELVM vm_;
    };

    // Factory functions
    Request *GET();
    Request *POST();
    Request *PUT();
    Request *DELETE_();
    extern moodycamel::ReaderWriterQueue<HttpResponse> responseQueue;
}
