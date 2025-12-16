/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:08:28 by mlaussel          #+#    #+#             */
/*   Updated: 2025/12/16 13:08:31 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Request.hpp"

typedef struct parsingS
{
    std::string line;
    int fd;
    
} parsingT;

int parsingMain(char **argv, request &request, parsingT &p);