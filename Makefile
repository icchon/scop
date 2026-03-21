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

$(NAME): $(OBJS) resource
	$(CXX) -o $(NAME) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

resource:
# 	if [ ! -d "resources" ]; then \
# 		wget https://cdn.intra.42.fr/document/document/34765/resources.tgz; \
# 		tar -xvf resources.tgz || zcat resources.tgz | zcat | tar -xvf -; \
# 	fi
	if [ ! -d "resources" ]; then \
		wget https://drive.google.com/file/d/1XzxqU4xciSvwcM4-2pJz2JLy8T6gl7lR resources.zip; \
		unzip resources.zip; \
	fi

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all