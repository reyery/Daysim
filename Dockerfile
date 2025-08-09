FROM debian:12-slim AS build

RUN apt-get update && apt-get install -y \
cmake \
build-essential

COPY . /Daysim
WORKDIR /Daysim

# create build folders at root
RUN mkdir build /build 

# build all CEA required programs using automated cea_targets
RUN cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release /Daysim \
    && make cea_targets \
    && cd bin \
    && mv ds_illum epw2wea gen_dc oconv radfiles2daysim rtrace_dc /build

FROM debian:12-slim AS run
COPY --from=build /build /Daysim

FROM scratch AS output
COPY --from=build /build /
