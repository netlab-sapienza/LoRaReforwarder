FROM alpine:3.17.0 AS build
RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        clang15=15.0.7-r0 \
        cmake=3.24.4-r0 \
        boost1.80-dev=1.80.0-r3 \
        mariadb-client=10.6.12-r0

WORKDIR /lorareforwarder
COPY src/ ./src/
COPY third_party ./third_party
COPY CMakeLists.txt .

WORKDIR /lorareforwarder/build
RUN cmake .. && \
    cmake --build . --parallel 8


FROM alpine:3.17.0

RUN apk update && \
    apk add --no-cache \
    libstdc++ \
    boost1.80-program_options=1.80.0-r3

RUN addgroup -S shs && adduser -S shs -G shs
USER shs

COPY --chown=shs:shs --from=build \
    ./lorareforwarder/build/src/lorareforwarder \
    ./app/

ENTRYPOINT [ "./app/lorareforwarder" ]