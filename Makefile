all:
	g++ -Ithird_party/tclap/include/ -std=c++14 -lsndfile frontend.cpp -o safety_limiter
