FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install WebSocket++
RUN wget https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz && \
    tar xzvf 0.8.2.tar.gz && \
    cp -r websocketpp-0.8.2/websocketpp /usr/local/include/ && \
    rm -rf websocketpp-0.8.2 0.8.2.tar.gz

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Expose the WebSocket port
EXPOSE 9002

# Run the server
CMD ["./build/iot_sensor_api"] 