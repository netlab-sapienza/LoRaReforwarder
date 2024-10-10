FROM alpine:3.17.0

ADD crontab /crontab
ADD databackup.sh /databackup.sh
COPY entry.sh /entry.sh
RUN chmod 755 /databackup.sh /entry.sh
RUN /usr/bin/crontab /crontab

RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        clang15=15.0.7-r0 \
        cmake=3.24.4-r0 \
        boost1.80-dev=1.80.0-r3 \
        mariadb-dev=10.6.16-r0 \
        mariadb-client=10.6.16-r0 \
        tzdata

ENV TZ=Europe/Rome

WORKDIR /lorareforwarder
COPY src/ ./src/
COPY third_party ./third_party
COPY CMakeLists.txt .
COPY .env .

WORKDIR /lorareforwarder/third_party/mariadb-connector-cpp/build
RUN cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL && \ 
    cmake --build . --config RelWithDebInfo --parallel 8 && \
    make install

ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib/mariadb"

WORKDIR /lorareforwarder/build
COPY ../.env .
RUN cmake .. && \
    cmake --build . --parallel 8

ENTRYPOINT [ "/entry.sh" ]