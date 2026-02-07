#!/bin/bash
# Generate PNG diagrams from PlantUML files
# Renders both common diagrams (reference/diagrams/) and
# chapter-specific diagrams (chapters/XX-topic/)
#
# Usage: ./render_diagrams.sh [--common|--chapters|--all] [--local|--docker]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIAGRAMS_DIR="$SCRIPT_DIR"
CHAPTERS_DIR="$REPO_ROOT/chapters"
MODE="${1:-all}"
METHOD="${2:-auto}"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_status()  { echo -e "${BLUE}[PlantUML]${NC} $1"; }
print_success() { echo -e "${GREEN}✓${NC} $1"; }
print_warning() { echo -e "${YELLOW}⚠${NC} $1"; }
print_error()   { echo -e "${RED}✗${NC} $1"; }

count=0
errors=0

# Render a single PUML file, placing the PNG alongside it
render_file() {
    local puml_file="$1"
    local output_dir
    output_dir="$(dirname "$puml_file")"
    local basename
    basename="$(basename "$puml_file" .puml)"

    print_status "Rendering $basename..."
    if plantuml -tpng "$puml_file" -o "$output_dir" 2>/dev/null; then
        print_success "$output_dir/$basename.png"
        count=$((count + 1))
    else
        print_error "Failed: $puml_file"
        errors=$((errors + 1))
    fi
}

render_file_docker() {
    local puml_file="$1"
    local output_dir
    output_dir="$(dirname "$puml_file")"
    local basename
    basename="$(basename "$puml_file" .puml)"

    print_status "Rendering $basename..."
    if docker run --rm \
        -v "$output_dir:/data" \
        plantuml/plantuml:latest \
        "/data/$(basename "$puml_file")" -o /data 2>/dev/null; then
        print_success "$output_dir/$basename.png"
        count=$((count + 1))
    else
        print_error "Failed: $puml_file"
        errors=$((errors + 1))
    fi
}

# Detect available rendering method
detect_method() {
    if command -v plantuml &>/dev/null; then
        echo "local"
    elif command -v docker &>/dev/null; then
        echo "docker"
    else
        echo "none"
    fi
}

# Render common diagrams (reference/diagrams/*.puml)
render_common() {
    local method="$1"
    local found=0
    for puml_file in "$DIAGRAMS_DIR"/*.puml; do
        [ -f "$puml_file" ] || continue
        found=$((found + 1))
        if [ "$method" = "docker" ]; then
            render_file_docker "$puml_file"
        else
            render_file "$puml_file"
        fi
    done
    print_status "Common diagrams found: $found"
}

# Render chapter-specific diagrams (chapters/XX-topic/*.puml)
render_chapters() {
    local method="$1"
    local found=0
    for puml_file in "$CHAPTERS_DIR"/*/*.puml; do
        [ -f "$puml_file" ] || continue
        found=$((found + 1))
        if [ "$method" = "docker" ]; then
            render_file_docker "$puml_file"
        else
            render_file "$puml_file"
        fi
    done
    print_status "Chapter diagrams found: $found"
}

# --- Main ---

# Handle --help
if [ "$MODE" = "--help" ] || [ "$MODE" = "-h" ]; then
    echo "Usage: $(basename "$0") [--common|--chapters|--all] [--local|--docker]"
    echo ""
    echo "Scope:"
    echo "  --common    Render only reference/diagrams/*.puml (4 common diagrams)"
    echo "  --chapters  Render only chapters/XX-topic/*.puml  (31 chapter diagrams)"
    echo "  --all       Render all PUML files (default)"
    echo ""
    echo "Method:"
    echo "  --local     Use system plantuml command"
    echo "  --docker    Use plantuml/plantuml Docker image"
    echo "  (default)   Auto-detect available method"
    exit 0
fi

# Resolve rendering method
if [ "$METHOD" = "auto" ]; then
    METHOD=$(detect_method)
    if [ "$METHOD" = "none" ]; then
        print_error "No rendering method available."
        print_warning "Install PlantUML:  sudo apt-get install plantuml"
        print_warning "Or use Docker:     docker pull plantuml/plantuml"
        exit 1
    fi
    print_status "Auto-detected method: $METHOD"
else
    METHOD="${METHOD#--}"  # strip leading --
fi

# Validate method
if [ "$METHOD" = "local" ] && ! command -v plantuml &>/dev/null; then
    print_error "plantuml not found. Install with: sudo apt-get install plantuml"
    exit 1
fi
if [ "$METHOD" = "docker" ] && ! command -v docker &>/dev/null; then
    print_error "docker not found. Install from https://docker.com"
    exit 1
fi

# Strip leading -- from mode
MODE="${MODE#--}"

print_status "Scope: $MODE | Method: $METHOD"
echo ""

case "$MODE" in
    common)
        render_common "$METHOD"
        ;;
    chapters)
        render_chapters "$METHOD"
        ;;
    all|*)
        render_common "$METHOD"
        echo ""
        render_chapters "$METHOD"
        ;;
esac

echo ""
if [ "$errors" -gt 0 ]; then
    print_warning "Rendered $count diagrams with $errors errors"
    exit 1
else
    print_success "All $count diagrams rendered successfully!"
fi
