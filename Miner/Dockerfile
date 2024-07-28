# Use an official Ubuntu base image
FROM ubuntu:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    zlib1g-dev

# Create directory for the project
WORKDIR /usr/src/miner

# Copy the current directory contents into the container
COPY . .

# Create mnt/mta directory
RUN mkdir -p /usr/src/mnt/mta

# Build the project
RUN cmake . && make

# Run the miner executable
CMD ["./newMiner"]