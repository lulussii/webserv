/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 11:55:23 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/10 17:17:43 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Delete.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <unistd.h>   //unlink()

/**
 * @brief `build the file path for DELETE method`
 * 
 * step 1 : Check if location.upload_dir is set
 * 
 * step 2 : Concatenate upload_dir with request URL to set response.path
 *
 */
void pathBuildDelete(responseT &response, request &request)
{
    if (response.location.upload_dir != "")
        response.path = response.location.upload_dir + request._url;
}

/**
 * @brief `check file existence and permissions before deleting with unlink`
 * 
 * step 1 : Use stat() to check if the file exists
 *   - If not, call errorCode with 404
 *
 * step 2 : Check read access with access()
 *   - If access denied, call errorCode with 403
 *
 * step 3 : Attempt to delete the file using unlink()
 *   - If unlink fails, call errorCode with 500
 *
 * step 4 : If deletion succeeds, set response.code to 204 (No Content)
 *
 */
void existFileDelete(responseT &response, serverT &serverConfig)
{
    struct stat test;
    if (stat(response.path.c_str(), &test) == -1)
        errorCode(response, serverConfig, 404);
    else
    {
        if (access(response.path.c_str(), R_OK) == -1)
        {
            errorCode(response, serverConfig, 403);
            return;
        }
        if (unlink(response.path.c_str()) != 0)
        {
            errorCode(response, serverConfig, 500);
            return;
        }
        response.code = 204;
    }
}

/**
 * @brief `DELETE method main`
 *
 * step 0 : Check if the request targets a CGI
 *   - If yes, return 405 Method Not Allowed
 *
 * step 1 : Build the full file path to delete
 *
 * step 2 : Check existence, permissions, and perform deletion
 *
 */
void deleteMain(request &request, responseT &response, serverT &serverConfig)
{
     if (response.cgi == true)
    {
        errorCode(response, serverConfig, 405);
        return;
    }

    pathBuildDelete(response, request);

    existFileDelete(response, serverConfig);
}