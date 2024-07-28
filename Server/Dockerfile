# Server/Dockerfile
FROM ubuntu:20.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    zlib1g-dev \
    && apt-get clean
    
    # Set up working directory
WORKDIR /app

RUN mkdir -p /app/mnt/mta


# Copy the entire project directory to /app
COPY . /app

# Build the shared library
WORKDIR /app/SharedLibrary/build
RUN cmake .. && make

# Build the server
WORKDIR /app/Server/build
RUN cmake .. && make


WORKDIR /app
# Set the entrypoint for the container
CMD ["/app/Server/build/newServer"]
