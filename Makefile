CXX := clang++
CXXFLAGS := -Oz -march=native
INC := -I../library
LIBS := 

SRC := ./src/
OBJ := main.o logging.o webserver.o http.o utils.o
OUT := kawaiserver.out

$(OUT): $(OBJ)
	$(CXX) $^ $(LIBS) -o $(OUT)

$(OBJ): %.o: $(SRC)%.cpp
	$(CXX) -c $(INC) $(CXXFLAGS) $^ -o $@

clean:
	rm $(OUT) $(OBJ)
