
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include "ft_irc.hpp"

vec_str ft_split(const str& string, char c)
{
	vec_str strs;
	str part;
	std::istringstream part_stream(string);

	while (std::getline(part_stream, part, c))
		strs.push_back(part);
	return strs;
}

str dateString(void)
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%c", timeinfo);
	return (buffer);
}

str intToString(int num)
{
	std::ostringstream ss;
	ss << num;
	return (ss.str());
}

bool containsOnlyDigits(const str &string)
{
	return string.find_first_not_of("0123456789") == str::npos;
}
