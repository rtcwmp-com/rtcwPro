FROM ubuntu:18.04
RUN apt update && apt install -y g++-multilib mingw-w64 make
