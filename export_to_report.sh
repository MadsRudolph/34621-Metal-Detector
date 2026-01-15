#!/bin/bash
#
# export_to_report.sh - Export assets from metal detector project to report repository
#
# This script copies figures, schematics, code snippets, and data from the
# Report_Export folder to the Overleaf/LaTeX report repository.
#
# Usage:
#   ./export_to_report.sh <report_repo_path>
#
# Examples:
#   ./export_to_report.sh ../34621-Report
#   ./export_to_report.sh /home/user/Documents/34621-Report
#
# The script will:
#   - Copy figures/ to report repo figures/
#   - Copy schematics/ to report repo figures/schematics/
#   - Copy code_snippets/ to report repo code/
#   - Copy data/ to report repo data/
#   - Skip empty folders (only .gitkeep)
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Get script directory (metal detector project root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXPORT_DIR="${SCRIPT_DIR}/Report_Export"

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if directory has real files (not just .gitkeep)
has_real_files() {
    local dir="$1"
    local count=$(find "$dir" -type f ! -name ".gitkeep" 2>/dev/null | wc -l)
    [ "$count" -gt 0 ]
}

# Function to copy files excluding .gitkeep
copy_files() {
    local src="$1"
    local dst="$2"
    local name="$3"

    if [ ! -d "$src" ]; then
        print_warning "Source directory not found: $src"
        return 1
    fi

    if ! has_real_files "$src"; then
        print_warning "Skipping $name (no files to export)"
        return 0
    fi

    # Create destination if it doesn't exist
    mkdir -p "$dst"

    # Copy files excluding .gitkeep
    local copied=0
    for file in "$src"/*; do
        if [ -f "$file" ] && [ "$(basename "$file")" != ".gitkeep" ]; then
            cp "$file" "$dst/"
            copied=$((copied + 1))
            print_info "  Copied: $(basename "$file")"
        fi
    done

    if [ $copied -gt 0 ]; then
        print_success "Exported $copied file(s) from $name"
    fi

    return 0
}

# Print usage
usage() {
    echo "Usage: $0 <report_repo_path>"
    echo ""
    echo "Export assets from Report_Export/ to the report repository."
    echo ""
    echo "Arguments:"
    echo "  report_repo_path  Path to the report repository (e.g., ../34621-Report)"
    echo ""
    echo "Examples:"
    echo "  $0 ../34621-Report"
    echo "  $0 /home/user/Documents/34621-Report"
    exit 1
}

# Main script
main() {
    # Check arguments
    if [ $# -ne 1 ]; then
        print_error "Missing report repository path"
        usage
    fi

    REPORT_REPO="$1"

    # Convert to absolute path if relative
    if [[ ! "$REPORT_REPO" = /* ]]; then
        REPORT_REPO="${SCRIPT_DIR}/${REPORT_REPO}"
    fi

    # Verify paths exist
    if [ ! -d "$EXPORT_DIR" ]; then
        print_error "Export directory not found: $EXPORT_DIR"
        print_info "Please ensure Report_Export/ exists in the project root"
        exit 1
    fi

    if [ ! -d "$REPORT_REPO" ]; then
        print_error "Report repository not found: $REPORT_REPO"
        print_info "Please provide a valid path to the report repository"
        exit 1
    fi

    echo ""
    echo "=========================================="
    echo "  Metal Detector Report Asset Export"
    echo "=========================================="
    echo ""
    print_info "Source: $EXPORT_DIR"
    print_info "Destination: $REPORT_REPO"
    echo ""

    # Track if any files were exported
    exported_any=false

    # Export figures
    echo "--- Figures ---"
    if copy_files "$EXPORT_DIR/figures" "$REPORT_REPO/figures" "figures"; then
        if has_real_files "$EXPORT_DIR/figures"; then
            exported_any=true
        fi
    fi
    echo ""

    # Export schematics (to figures/schematics in report)
    echo "--- Schematics ---"
    if copy_files "$EXPORT_DIR/schematics" "$REPORT_REPO/figures/schematics" "schematics"; then
        if has_real_files "$EXPORT_DIR/schematics"; then
            exported_any=true
        fi
    fi
    echo ""

    # Export code snippets
    echo "--- Code Snippets ---"
    if copy_files "$EXPORT_DIR/code_snippets" "$REPORT_REPO/code" "code_snippets"; then
        if has_real_files "$EXPORT_DIR/code_snippets"; then
            exported_any=true
        fi
    fi
    echo ""

    # Export data
    echo "--- Data ---"
    if copy_files "$EXPORT_DIR/data" "$REPORT_REPO/data" "data"; then
        if has_real_files "$EXPORT_DIR/data"; then
            exported_any=true
        fi
    fi
    echo ""

    # Summary and next steps
    echo "=========================================="
    if [ "$exported_any" = true ]; then
        print_success "Export complete!"
        echo ""
        echo "Next steps:"
        echo "  1. cd $REPORT_REPO"
        echo "  2. git status                    # Review changes"
        echo "  3. git add -A                    # Stage all changes"
        echo "  4. git commit -m 'Update assets from main project'"
        echo "  5. git push                      # Push to remote (syncs with Overleaf)"
    else
        print_warning "No files were exported"
        print_info "Add assets to Report_Export/ subfolders before running this script"
    fi
    echo "=========================================="
    echo ""
}

main "$@"
