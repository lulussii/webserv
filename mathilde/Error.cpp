#include "Request.hpp"
#include "Get.hpp"

/**
 * @brief `search html body for error`
 *
 * if code is 404 so the new path is root/404.html
 */
void errorCode(responseT &response, serverT &serverConfig, request &request)
{
    for (std::map<int, std::string>::iterator it = serverConfig.errorPage.begin();
         it != serverConfig.errorPage.end(); it++)
    {
        if (it->first == response.code)
        {
            if (request._url == "/")
                response.path = serverConfig.root + it->second;
            else
                response.path = serverConfig.root + it->second;
            readFile(response);
            return;
        }
    }
    response.body = "";
}

void error404(responseT &response, serverT &serverConfig, request &request)
{
    response.infos.error = true;
    response.infos.fileExist = false;
    response.code = 404;
    response.contentType += "text/plain";
    errorCode(response, serverConfig, request);
    response.contentLen = response.body.size();
}

void error403(responseT &response, serverT &serverConfig, request &request)
{
    response.infos.error = true;
    response.infos.fileExist = false;
    response.code = 403;
    response.contentType += "text/plain";
    errorCode(response, serverConfig, request);
    response.contentLen = response.body.size();
}

void error405(responseT &response)
{
    response.infos.error = true;
    response.infos.fileExist = false;
    response.code = 405;
    response.contentType += "text/plain";
    response.body = "";
    response.contentLen = response.body.size();
}
