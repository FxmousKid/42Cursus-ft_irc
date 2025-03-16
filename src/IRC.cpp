/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inazaria <inazaria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 15:32:54 by inazaria          #+#    #+#             */
/*   Updated: 2025/03/16 15:52:14 by inazaria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRC.hpp"

IRC::IRC(): port(0), password("") {
	
}

IRC::~IRC() {

}

IRC::IRC(const IRC& other) {
	*this = other; 
}

IRC&	IRC::operator=(const IRC& other) {
	if (this != &other) {
		this->port = other.port;
		this->password = other.password;
	}
	return (*this);
}

IRC::IRC(int port, str password): port(port), password(password) {

}
