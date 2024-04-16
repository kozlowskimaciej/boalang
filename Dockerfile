# Start from Ubuntu 22.04 image
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    clang-14 \
    clang-tidy-14 \
    cmake \
    python3 \
    python3.10-venv

# Set the default C and C++ compiler to Clang
ENV CC=/usr/bin/clang-14
ENV CXX=/usr/bin/clang++-14

# Set the working directory
WORKDIR .

# Copy the current directory contents into the container
COPY . .

# Set the default command to run when starting a container
CMD ["./configure.sh"]