/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: inazaria <inazaria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 15:18:20 by inazaria          #+#    #+#             */
/*   Updated: 2025/03/16 15:50:45 by inazaria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

typedef std::string str;

class IRC {
public:
	IRC();
	IRC(const IRC& other);
	IRC& operator=(const IRC& other);
	~IRC();
	
	IRC(int port, str password);

private:
	int	port;
	str	password;
};
