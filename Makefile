#<><><><><><><> Files <><><><><><><><><><><><><><><><><><><>

SRC_DIR		= src/
BUILD_DIR	= build/
INC_DIR		= includes/

# Source files
SRC_FILES_NAMES = main.cpp
SRC_FILES_NAMES += utils.cpp
SRC_FILES_NAMES += Client.cpp
SRC_FILES_NAMES += CommandHandler.cpp
SRC_FILES_NAMES += Server.cpp
SRC_FILES_NAMES += Channel.cpp

# Command files
SRC_FILES_NAMES += commands/USER.cpp
SRC_FILES_NAMES += commands/WHO.cpp
SRC_FILES_NAMES += commands/PONG.cpp
SRC_FILES_NAMES += commands/PRIVMSG.cpp
SRC_FILES_NAMES += commands/QUIT.cpp
SRC_FILES_NAMES += commands/PART.cpp
SRC_FILES_NAMES += commands/PASS.cpp
SRC_FILES_NAMES += commands/PING.cpp
SRC_FILES_NAMES += commands/LIST.cpp
SRC_FILES_NAMES += commands/MODE.cpp
SRC_FILES_NAMES += commands/NICK.cpp
SRC_FILES_NAMES += commands/NOTICE.cpp
SRC_FILES_NAMES += commands/INVITE.cpp
SRC_FILES_NAMES += commands/JOIN.cpp
SRC_FILES_NAMES += commands/KICK.cpp
SRC_FILES_NAMES += commands/TOPIC.cpp

# Full path to source files
SRC_FILES = $(addprefix $(SRC_DIR), $(SRC_FILES_NAMES))

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)%.cpp, $(BUILD_DIR)%.o, $(SRC_FILES))

# Dependency files
DEP_FILES = $(patsubst $(SRC_DIR)%.cpp, $(BUILD_DIR)%.d, $(SRC_FILES))

#<><><><><><><> Variables <><><><><><><><><><><><><><><><><>

NAME = ircserv
CXX = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -I$(INC_DIR) -MMD -MP
CFLAGS += -g3 -D DEBUG=$(DEBUG) -D BUFFER_SIZE=$(BUFFER_SIZE)
LFLAGS =

DEBUG = 1
BUFFER_SIZE	= 4096

MKDIR = mkdir -p
RM_RF = rm -rf
ECHO = echo -e


BLUE	:= $(shell echo -e "\033[34m") 
BROWN	:= $(shell echo -e "\033[33m")
GREEN	:= $(shell echo -e "\033[32m")
RED		:= $(shell echo -e "\033[31m")
NC		:= $(shell echo -e "\033[0m")

#<><><><><><><> Recipes <><><><><><><><><><><><><><><><><><>

# Include dependency files
-include $(DEP_FILES)

$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp Makefile | $(BUILD_DIR)
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CFLAGS)  -c $< -o $@
	@$(ECHO) "$(BLUE)[CMP] Compiling $<...$(END)"

$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)

$(NAME): $(OBJ_FILES)
	@$(CXX) $(LFLAGS) $(OBJ_FILES) -o $(NAME)
	@$(ECHO) "$(GREEN)[BLD] $(NAME) built successfully$(END)"

all: $(NAME)

clean:
	@$(RM_RF) $(BUILD_DIR)
	@$(ECHO) "$(YELLOW)[CLN] Object files removed$(END)"

fclean: clean
	@$(RM_RF) $(NAME) $(BUILD_DIR)
	@$(ECHO) "$(YELLOW)[CLN] $(NAME) removed$(END)"

re: fclean all

debugflags:
	$(eval DEBUG=1)

debug: debugflags all

.DEFAULT_GOAL := all
.PHONY: all clean fclean re debugflags debug
