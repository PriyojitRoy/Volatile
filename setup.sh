#!/usr/bin/env bash
# Volatile Setup Script for Unix-like systems (Linux, macOS, Termux)
set -e

echo "====================================="
echo "  Volatile Setup Script (Unix)"
echo "====================================="

# ==========================================
# Volatile Setup Configuration
# ==========================================
REQUIRED_TOOLS=("cmake" "make")
OPTIONAL_TOOLS=("fastchess" "cutechess-cli")
VENV_DIR=".venv"

# 1. Resolve Python (python3 vs python)
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python"
else
    PYTHON_CMD="python3" # Fallback so it fails gracefully in check loop
fi
REQUIRED_TOOLS+=("$PYTHON_CMD")

# 2. Resolve C++ Compiler (g++ vs clang++)
if command -v g++ &> /dev/null; then
    COMPILER_CMD="g++"
elif command -v clang++ &> /dev/null; then
    COMPILER_CMD="clang++"
else
    COMPILER_CMD="g++" # Fallback so it fails gracefully in check loop
fi
REQUIRED_TOOLS+=("$COMPILER_CMD")

# ==========================================
# Core Functions
# ==========================================

check_tool() {
    local tool=$1
    local is_required=$2
    
    if command -v "$tool" &> /dev/null; then
        # Capture the first line of the version string
        local version_info=$("$tool" --version 2>&1 | head -n 1 || echo "Found")
        echo "[+] Found $tool: $version_info"
    else
        if [ "$is_required" = true ]; then
            echo "[-] Error: Required tool '$tool' is missing."
            exit 1
        else
            echo "[-] Warning: Optional tool '$tool' is missing."
        fi
    fi
}

setup_venv() {
    local python_cmd=$1
    echo "-------------------------------------"
    
    # Termux specific fixes for numpy/ninja compilation issues
    local is_termux=false
    local venv_args=""
    if [[ "$(uname -o 2>/dev/null)" == *"Android"* ]] || [[ "$PREFIX" == *"com.termux"* ]]; then
        is_termux=true
        echo "Termux environment detected!"
        echo "Translating requirements.txt into native Termux packages..."
        
        # Read requirements.txt, extract base package names, convert to lowercase, and prefix with python-
        local termux_pkgs=""
        while read -r line; do
            # Skip comments and empty lines
            [[ "$line" =~ ^#.*$ ]] || [[ -z "$line" ]] && continue
            # Extract package name (everything before version specifiers like >=, ==)
            local pkg_name=$(echo "$line" | grep -o '^[A-Za-z0-9_-]*' | tr '[:upper:]' '[:lower:]')
            if [ -n "$pkg_name" ]; then
                termux_pkgs="$termux_pkgs python-$pkg_name"
            fi
        done < requirements.txt
        
        echo "Installing system-level packages: $termux_pkgs"
        pkg install -y $termux_pkgs
        venv_args="--system-site-packages"
    fi

    if [ -d "$VENV_DIR" ]; then
        echo "Removing existing virtual environment to ensure a clean build..."
        rm -rf "$VENV_DIR"
    fi
    
    echo "Creating Python virtual environment in $VENV_DIR..."
    "$python_cmd" -m venv $venv_args "$VENV_DIR"

    echo "Activating virtual environment..."
    source "$VENV_DIR/bin/activate"

    echo "Installing Python dependencies..."
    python -m pip install --upgrade pip
    
    if [ "$is_termux" = true ]; then
        echo "Skipping pip install on Termux as dependencies were installed natively via pkg."
    else
        python -m pip install -r requirements.txt
    fi
}

# ==========================================
# Main Execution
# ==========================================

# Check Required Tools
for tool in "${REQUIRED_TOOLS[@]}"; do
    check_tool "$tool" true
done

# Check Optional Tools
for tool in "${OPTIONAL_TOOLS[@]}"; do
    check_tool "$tool" false
done

# Setup Python Environment
setup_venv "$PYTHON_CMD"

echo "====================================="
echo "Setup complete!"
echo "To activate the virtual environment manually, run: source $VENV_DIR/bin/activate"
echo "To build the engine, run:"
echo "  mkdir -p build && cd build && cmake .. -DUSE_NNUE=OFF && make -j\$(nproc)"
