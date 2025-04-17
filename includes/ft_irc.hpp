#pragma once

# ifndef DEBUG
#  define DEBUG 0
# endif

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 8192
# endif

# define TRUE 1
# define FALSE 0

#include "ANSI-colors.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "CommandHandler.hpp"
# include "Command.hpp"
# include "rpl.hpp"

typedef std::string			str;
typedef std::vector<str>	vec_str;

// utils.cpp
vec_str				ft_split(const str& str, char c);
str					dateString(void);
str					intToString(int num);
bool				containsOnlyDigits(const str &str);
