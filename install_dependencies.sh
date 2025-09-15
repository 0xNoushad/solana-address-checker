#!/usr/bin/env bash
set -e

echo "Installing dependencies..."

if [[ "$OSTYPE" == "darwin"* ]]; then
    if command -v brew >/dev/null 2>&1; then
        brew install json-c curl pkg-config
    else
        echo "Homebrew not found. Please install it first: https://brew.sh/"
        exit 1
    fi
elif [[ -f /etc/debian_version ]]; then
    sudo apt-get update
    sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev build-essential pkg-config
elif [[ -f /etc/redhat-release ]]; then
    if command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y libcurl-devel json-c-devel gcc make pkg-config
    else
        sudo yum install -y libcurl-devel json-c-devel gcc make pkg-config
    fi
else
    echo "Unsupported OS. Install json-c, curl, gcc, and pkg-config manually."
    exit 1
fi

echo "Dependencies installed!"
