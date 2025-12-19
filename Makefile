# ************************************************************************** #
#                               PROJECT CONFIG                                #
# ************************************************************************** #

NAME 		= webserv

CC 			= c++
CFLAGS 		= -Wall -Wextra -Werror -std=c++98 -Iinclude -g -MMD -MP

SRC_DIR 	= src
HEADER_DIR 	= include

HEADER 		= $(HEADER_DIR)/

SRC 		= $(SRC_DIR)/main.cpp 				\

OBJ_DIR 	= objs
OBJ 		= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# ************************************************************************** #
#                                   COLORS                                   #
# ************************************************************************** #

HOT_PINK = \033[38;5;212m
PINK = \033[38;5;198m
PALE_PINK = \033[38;5;212m
RESET = \033[0m

# ************************************************************************** #
#                            PROGRESS BAR CONFIG                             #
# ************************************************************************** #

TOTAL_FILES := $(words $(SRC))
CURRENT_FILE = 0

define progress_bar
	@$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
	@$(eval PERCENT=$(shell echo $$(($(CURRENT_FILE) * 100 / $(TOTAL_FILES)))))
	@printf "\r$(PINK)Compilation: [$(PALE_PINK)"
	@for i in `seq 1 $(shell echo $$(($(PERCENT) / 5)))`; do printf "#"; done
	@for i in `seq 1 $(shell echo $$((20 - $(PERCENT) / 5)))`; do printf " "; done
	@printf "$(PINK)] %3d%% - %s$(RESET)" $(PERCENT) $<
endef

define complete_progress
	@printf "\r$(PINK)Compilation: [$(PALE_PINK)####################$(PINK)] 100%%                                 $(RESET)\n"
endef

# ************************************************************************** #
#                                COMPILATION                                 #
# ************************************************************************** #

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(call progress_bar)
	@$(CC) $(CFLAGS) -c $< -o $@

# ************************************************************************** #
#                                  TARGETS                                   #
# ************************************************************************** #

all: $(NAME)

$(NAME): $(OBJ)
	$(call complete_progress)
	@echo "$(HOT_PINK)Création de l'executable $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "$(HOT_PINK)Executable $(NAME) créée !$(RESET)"

clean:
	@echo "$(PINK)Suppression des fichiers objets...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(HOT_PINK)Fichiers objets supprimés !$(RESET)"

fclean: clean
	@echo "$(PINK)Suppression de $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@echo "$(HOT_PINK)$(NAME) supprimé !$(RESET)"

re: fclean all

-include $(OBJ:.o=.d)

.PHONY: all clean fclean re