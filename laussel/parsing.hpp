/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 14:45:27 by mathildelau       #+#    #+#             */
/*   Updated: 2025/12/15 15:26:25 by mathildelau      ###   ########.fr       */
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