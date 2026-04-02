FROM gcc:latest

RUN apt update && apt install cppcheck valgrind -y
