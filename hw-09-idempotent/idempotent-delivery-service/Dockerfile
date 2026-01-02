FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y build-essential cmake git libboost-all-dev libssl-dev libpq-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libboost-system1.74.0 libboost-thread1.74.0 libssl3 libpq5 curl && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/build/idempotent-delivery-service .
COPY --from=builder /app/build/config.json .
EXPOSE 8003
HEALTHCHECK --interval=30s --timeout=3s CMD curl -f http://localhost:8003/health || exit 1
CMD ["./idempotent-delivery-service"]
