#!/bin/bash
# Slim's Editor Vita - Complete Project Setup Script
# This script creates the full multi-file structure

set -e

echo "=========================================="
echo "  Slim's Editor Vita - Project Setup"
echo "=========================================="
echo ""

# Check VitaSDK
if [ -z "$VITASDK" ]; then
    echo "ERROR: VITASDK environment variable not set!"
    exit 1
fi

echo "✓ VitaSDK found: $VITASDK"
echo ""

# Create directory structure
echo "Creating directory structure..."
mkdir -p src/ui
mkdir -p src/core
mkdir -p src/utils
mkdir -p src/data
mkdir -p build

echo "✓ Directories created"
echo ""

echo "=========================================="
echo "  File Creation Instructions"
echo "=========================================="
echo ""
echo "I've provided all the source files in the artifacts."
echo "You need to create these files in your project:"
echo ""
echo "1. CMakeLists.txt (root)"
echo "2. src/main.cpp"
echo "3. src/app.h"
echo "4. src/app.cpp"
echo "5. src/utils/colors.h"
echo "6. src/utils/input.h"
echo "7. src/data/rac1_data.h"
echo "8. src/core/save_file.h"
echo "9. src/core/save_file.cpp"
echo "10. src/core/game_data.cpp"
echo "11. src/ui/file_browser.h"
echo "12. src/ui/file_browser.cpp"
echo "13. src/ui/save_editor.h"
echo "14. src/ui/save_editor.cpp (part 1 + part 2)"
echo "15. src/ui/keyboard.h"
echo "16. src/ui/keyboard.cpp"
echo ""
echo "All files are in the Claude artifacts above."
echo "Copy each one to the correct location."
echo ""
echo "Once all files are in place, run:"
echo "  cd build"
echo "  cmake .."
echo "  make"
echo ""
echo "=========================================="
