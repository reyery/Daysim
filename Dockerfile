FROM debian:12-slim AS build

RUN apt-get update && apt-get install -y \
cmake \
build-essential \
libgl1-mesa-dev \
libglu1-mesa-dev

COPY . /Daysim
WORKDIR /Daysim

# create final build folder at root
RUN mkdir /build

# only build required binaries
RUN mkdir build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_HEADLESS=ON -DUSE_OPENGL=ON /Daysim \
    && make ds_illum \
    && make epw2wea \
    && make gen_dc \
    && make oconv \
    && make radfiles2daysim \
    && cd bin \
    && mv ds_illum epw2wea gen_dc oconv radfiles2daysim /build

# uncommenting line in CMakeLists to build rtrace_dc
RUN sed -i 's/#add_definitions(-DDAYSIM)/add_definitions(-DDAYSIM)/' /Daysim/src/rt/CMakeLists.txt \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_HEADLESS=ON -DUSE_OPENGL=ON /Daysim \
    && make rtrace \
    && cd bin \
    && mv rtrace /build/rtrace_dc

FROM debian:12-slim AS run
COPY --from=build /build /Daysim

FROM scratch AS output
COPY --from=build /build /
