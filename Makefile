CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -g -lvlc -pthread -lncursesw -lcurl -ljsoncpp

SRC :=  $(shell find src -name "*.cpp")
SRCNAME = $(notdir $(SRC))
SRCOBJ = $(SRCNAME:%.cpp=%.o)


all: main
	@ echo "> done :)"
	@ echo "> run \"./bot"

main: $(SRCOBJ)
	@ echo "> Compiling: project"
	@g++ $(CXXFLAGS) -o bin/bot $(addprefix obj/,$(SRCOBJ)) $(LDFLAGS)

$(SRCOBJ): $(SRC)
	@ echo "> Compiling: "$(@:%.o=%.cpp)
	@g++ $(CXXFLAGS) -o obj/$@ -c $(filter %/$(@:%.o=%.cpp), $^) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f obj/*.o bin/bot