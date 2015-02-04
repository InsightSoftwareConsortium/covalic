FROM ubuntu:14.04
MAINTAINER Zach Mullen <zach.mullen@kitware.com>

RUN mkdir /covalic
RUN mkdir /covalic/_build

WORKDIR /covalic/_build
COPY CMake /covalic/CMake
COPY Code /covalic/Code
COPY Documentation /covalic/Documentation
COPY Utilities /covalic/Utilities
COPY CMakeLists.txt /covalic/CMakeLists.txt

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git
RUN cmake /covalic
RUN make -j4

ENTRYPOINT ["Covalic-Build/Code/Testing/validateLabelImages"]
