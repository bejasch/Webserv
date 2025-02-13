FROM ubuntu:22.04 

# Set noninteractive installation and timezone
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Install required packages with minimal dependencies
RUN apt update && apt install -y \
    gcc \
    g++ \
    make \
    python3-minimal \
    php-cli \
    php-cgi \
    && rm -rf /var/lib/apt/lists/*

# Copy all files to root
COPY . /

# Make CGI scripts executable
RUN chmod +x /data/cgi-bin/*

# Build the project using make
RUN make

# Allow command-line arguments when running the container
ENTRYPOINT ["./webserv"]
CMD ["config/test.conf"]