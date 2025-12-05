# Compiler
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -Wc++11-extensions

# Targets
all: test_robot RobotBase.o RobotWarz Scans.o

RobotBase.o: RobotBase.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -c RobotBase.cpp
Scans.o: Scans.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -c Scans.cpp


test_robot: test_robot.cpp RobotBase.o
	$(CXX) $(CXXFLAGS) test_robot.cpp RobotBase.o -ldl -o test_robot
RobotWarz: RobotBase.o RobotWarz.cpp
	$(CXX) $(CXXFLAGS) RobotWarz.cpp RobotBase.o -o RobotWarz 
clean:
	rm -f *.o test_robot *.so
