NAME    := scop
SRCS    := $(wildcard src/*.cpp)
OBJS    := $(SRCS:.cpp=.o)
CXX     := c++
CXXFLAGS:= -Wall -Wextra -Werror -std=c++11 -I./includes

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
    LIBS := -lglfw -lGL -lGLEW
else ifeq ($(UNAME_S), Darwin)
    LIBS := -lglfw -framework OpenGL -lglew
endif

$(NAME): $(OBJS) shader get
	$(CXX) -o $(NAME) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

shader:
	chmod +rwx shaders/*

get:
	wget https://cdn.intra.42.fr/document/document/34765/resources.tgz
	tar -zxvf resources.tgz

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all