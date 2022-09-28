FROM fuzzers/libfuzzer:12.0 as builder

RUN apt-get update
RUN apt install -y make automake autotools-dev clang cmake zlib1g-dev git
ADD . /rapidfuzz-cpp
WORKDIR /rapidfuzz-cpp
RUN mkdir build
WORKDIR ./build
RUN cmake -DCMAKE_CXX_COMPILER=clang -DRAPIDFUZZ_BUILD_FUZZERS=ON ..
RUN make install


FROM fuzzers/libfuzzer:12.0 
COPY --from=builder /rapidfuzz-cpp/build/fuzzing/ /

ENTRYPOINT []
CMD ["/fuzz_osa_distance"]
