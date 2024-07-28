#!/bin/bash

# Navigate to the project root directory
cd "$(dirname "$0")"

# Build the Docker image for the server
docker build -t server_image -f Server/Dockerfile .