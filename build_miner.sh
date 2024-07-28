#!/bin/bash

# Navigate to the project root directory
cd "$(dirname "$0")"

# Build the Docker image for the miner
docker build -t miner_image -f Miner/Dockerfile .