#!/bin/sh
# shellcheck shell=dash
#
# Install script for queue (processq)
# https://github.com/GNITOAHC/processq
#
# Usage:
#   curl -LsSf https://raw.githubusercontent.com/GNITOAHC/processq/main/install.sh | sh
#   wget -qO- https://raw.githubusercontent.com/GNITOAHC/processq/main/install.sh | sh
#
# Environment variables:
#   QUEUE_VERSION       - Install a specific version (e.g., v0.2.1)
#   QUEUE_INSTALL_DIR   - Custom install directory (default: $HOME/.local/bin)

set -eu

# ============================================================================
# Configuration
# ============================================================================

APP_NAME="queue"
REPO_OWNER="GNITOAHC"
REPO_NAME="processq"
GITHUB_API_URL="https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/latest"
GITHUB_RELEASE_URL="https://github.com/${REPO_OWNER}/${REPO_NAME}/releases/download"

# ============================================================================
# Output helpers
# ============================================================================

say() {
    printf '%s\n' "$1"
}

say_verbose() {
    if [ "${QUEUE_VERBOSE:-0}" = "1" ]; then
        printf '%s\n' "$1" >&2
    fi
}

err() {
    say "error: $1" >&2
    exit 1
}

warn() {
    say "warning: $1" >&2
}

# ============================================================================
# Requirement checks
# ============================================================================

need_cmd() {
    if ! command -v "$1" > /dev/null 2>&1; then
        err "need '$1' (command not found)"
    fi
}

check_cmd() {
    command -v "$1" > /dev/null 2>&1
}

# ============================================================================
# Downloader
# ============================================================================

# Download a file using curl or wget
# Usage: downloader <url> <output_file>
downloader() {
    local _url="$1"
    local _output="$2"

    if check_cmd curl; then
        curl -fsSL "$_url" -o "$_output"
    elif check_cmd wget; then
        wget -qO "$_output" "$_url"
    else
        err "need 'curl' or 'wget' to download files"
    fi
}

# Fetch content from URL and print to stdout
# Usage: fetch_url <url>
fetch_url() {
    local _url="$1"

    if check_cmd curl; then
        curl -fsSL "$_url"
    elif check_cmd wget; then
        wget -qO- "$_url"
    else
        err "need 'curl' or 'wget' to download files"
    fi
}

# ============================================================================
# Platform detection
# ============================================================================

get_architecture() {
    local _os
    local _arch
    local _ostype
    local _cputype

    _ostype="$(uname -s)"
    _cputype="$(uname -m)"

    # Determine OS
    case "$_ostype" in
        Linux | linux)
            _os="linux"
            ;;
        Darwin)
            _os="darwin"
            ;;
        *)
            err "unsupported OS: $_ostype (supported: Linux, Darwin)"
            ;;
    esac

    # Determine architecture
    case "$_cputype" in
        x86_64 | amd64)
            _arch="amd64"
            ;;
        aarch64 | arm64)
            _arch="arm64"
            ;;
        *)
            err "unsupported architecture: $_cputype (supported: x86_64/amd64, aarch64/arm64)"
            ;;
    esac

    # Validate supported platform combinations
    case "${_os}-${_arch}" in
        darwin-arm64)
            ;;
        linux-amd64)
            ;;
        linux-arm64)
            ;;
        darwin-amd64)
            err "darwin-amd64 (Intel Mac) is not supported. Pre-built binaries are only available for Apple Silicon (darwin-arm64)."
            ;;
        *)
            err "unsupported platform: ${_os}-${_arch}"
            ;;
    esac

    echo "${_os}-${_arch}"
}

# ============================================================================
# Version resolution
# ============================================================================

get_latest_version() {
    local _response
    local _version

    say_verbose "fetching latest version from GitHub API..."

    _response="$(fetch_url "$GITHUB_API_URL" 2>/dev/null)" || {
        err "failed to fetch latest version from GitHub API"
    }

    # Extract tag_name from JSON response using grep and sed
    # This avoids requiring jq as a dependency
    _version="$(echo "$_response" | grep '"tag_name"' | sed -E 's/.*"tag_name": *"([^"]+)".*/\1/')"

    if [ -z "$_version" ]; then
        err "failed to parse version from GitHub API response"
    fi

    echo "$_version"
}

# ============================================================================
# Installation
# ============================================================================

install_binary() {
    local _version="$1"
    local _arch="$2"
    local _install_dir="$3"

    local _archive_name="${APP_NAME}-${_version}-${_arch}.tar.gz"
    local _download_url="${GITHUB_RELEASE_URL}/${_version}/${_archive_name}"
    local _tmp_dir

    # Create temporary directory
    _tmp_dir="$(mktemp -d)" || err "failed to create temporary directory"

    # Ensure cleanup on exit
    trap "rm -rf '$_tmp_dir'" EXIT

    local _archive_path="${_tmp_dir}/${_archive_name}"

    say "downloading ${APP_NAME} ${_version} (${_arch})..."
    say_verbose "  from: $_download_url"
    say_verbose "  to: $_archive_path"

    if ! downloader "$_download_url" "$_archive_path"; then
        err "failed to download ${_download_url}
This may be a network error, or the release may not exist for your platform.
Please check: https://github.com/${REPO_OWNER}/${REPO_NAME}/releases"
    fi

    say "extracting archive..."

    # Extract the archive
    tar -xzf "$_archive_path" -C "$_tmp_dir" || err "failed to extract archive"

    # Create install directory if it doesn't exist
    if [ ! -d "$_install_dir" ]; then
        say "creating install directory: $_install_dir"
        mkdir -p "$_install_dir" || err "failed to create install directory: $_install_dir"
    fi

    # Move binary to install directory
    local _binary_path="${_tmp_dir}/${APP_NAME}"
    if [ ! -f "$_binary_path" ]; then
        err "binary not found in archive: ${APP_NAME}"
    fi

    say "installing to ${_install_dir}/${APP_NAME}..."
    mv "$_binary_path" "${_install_dir}/${APP_NAME}" || err "failed to move binary to install directory"
    chmod +x "${_install_dir}/${APP_NAME}" || err "failed to set executable permission"

    say "installed ${APP_NAME} ${_version} to ${_install_dir}/${APP_NAME}"
}

# ============================================================================
# Usage
# ============================================================================

usage() {
    cat << EOF
${APP_NAME}-installer

Install script for ${APP_NAME} - a simple process queue manager

USAGE:
    curl -LsSf https://raw.githubusercontent.com/${REPO_OWNER}/${REPO_NAME}/main/install.sh | sh

ENVIRONMENT VARIABLES:
    QUEUE_VERSION         Install a specific version (e.g., v0.2.1)
    QUEUE_INSTALL_DIR     Custom install directory (default: \$HOME/.local/bin)
    QUEUE_VERBOSE         Set to 1 for verbose output

OPTIONS:
    -h, --help            Show this help message
    -v, --verbose         Enable verbose output

EOF
}

# ============================================================================
# Main
# ============================================================================

main() {
    # Parse arguments
    for arg in "$@"; do
        case "$arg" in
            -h | --help)
                usage
                exit 0
                ;;
            -v | --verbose)
                QUEUE_VERBOSE=1
                ;;
            *)
                err "unknown option: $arg"
                ;;
        esac
    done

    # Check required commands
    need_cmd uname
    need_cmd mktemp
    need_cmd chmod
    need_cmd mkdir
    need_cmd tar
    need_cmd grep
    need_cmd sed

    # Detect platform
    local _arch
    _arch="$(get_architecture)"
    say_verbose "detected platform: $_arch"

    # Determine version
    local _version
    if [ -n "${QUEUE_VERSION:-}" ]; then
        _version="$QUEUE_VERSION"
        say_verbose "using specified version: $_version"
    else
        _version="$(get_latest_version)"
        say_verbose "latest version: $_version"
    fi

    # Determine install directory
    local _install_dir
    local _install_dir_expr
    if [ -n "${QUEUE_INSTALL_DIR:-}" ]; then
        _install_dir="$QUEUE_INSTALL_DIR"
        _install_dir_expr="$QUEUE_INSTALL_DIR"
    else
        _install_dir="${HOME}/.local/bin"
        _install_dir_expr='$HOME/.local/bin'
    fi
    say_verbose "install directory: $_install_dir"

    # Install the binary
    install_binary "$_version" "$_arch" "$_install_dir"

    say ""
    say "Installation complete!"
    say ""
    say "Make sure ${_install_dir_expr} is in your PATH."
}

main "$@"
