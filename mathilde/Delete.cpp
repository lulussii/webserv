/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 11:55:23 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 11:21:55 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Delete.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <unistd.h>   //unlink()

/**
 * @brief `build the file path`
 *
 */
void pathBuildDelete(responseT &response, request &request)
{
    if (response.location.upload_dir != "")
        response.path = response.location.upload_dir + request._url;
}

/**
 * @brief `check if the file exist and if we can access before delete with unlink`
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
 * step 1 : build file path
 *
 * step 2 : check if the file exist and if we can access before delete
 *
 */
void deleteMain(request &request, responseT &response, serverT &serverConfig)
{
    // step 1 : build file path
    pathBuildDelete(response, request);

    // step 2 : check if the file exist and if we can access before delete
    existFileDelete(response, serverConfig);
}