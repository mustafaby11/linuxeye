#!/usr/bin/env bash
# ──────────────────────────────────────────────────────────
# linuxeye – setup.sh
# Installation script for linuxeye system analyzer
# https://github.com/mustafaby11/linuxeye
# ──────────────────────────────────────────────────────────
set -euo pipefail

# ── Colors ────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
success() { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

SRCDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ── Root check ────────────────────────────────────────────
need_root() {
    if [[ $EUID -ne 0 ]]; then
        if command -v sudo &>/dev/null; then
            SUDO="sudo"
        elif command -v sudo-rs &>/dev/null; then
            SUDO="sudo-rs"
        elif command -v doas &>/dev/null; then
            SUDO="doas"
        else
            error "This step requires root. Please install sudo, sudo-rs, or doas."
        fi
    else
        SUDO=""
    fi
}

# ── Distro detection ─────────────────────────────────────
detect_distro() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        DISTRO_ID="${ID:-unknown}"
        DISTRO_ID_LIKE="${ID_LIKE:-}"
    else
        error "Cannot detect distribution (/etc/os-release not found)"
    fi
}

# ── Install dependencies ─────────────────────────────────
cmd_installdep() {
    detect_distro
    need_root
    info "Detected distribution: ${BOLD}${DISTRO_ID}${NC}"

    case "$DISTRO_ID" in
        arch|cachyos|endeavouros|manjaro|garuda|arcolinux|artix)
            info "Installing dependencies (pacman)..."
            $SUDO pacman -S --needed --noconfirm \
                qt6-base qt6-svg cmake gcc ninja git
            ;;
        ubuntu|debian|linuxmint|pop|zorin|elementary)
            info "Installing dependencies (apt)..."
            $SUDO apt update -qq
            $SUDO apt install -y \
                qt6-base-dev qt6-svg-dev cmake g++ ninja-build git
            ;;
        fedora|nobara)
            info "Installing dependencies (dnf)..."
            $SUDO dnf install -y \
                qt6-qtbase-devel qt6-qtsvg-devel cmake gcc-c++ ninja-build git
            ;;
        bazzite|silverblue)
            info "Installing dependencies (rpm-ostree)..."
            rpm-ostree install --idempotent \
                qt6-qtbase-devel qt6-qtsvg-devel cmake gcc-c++ ninja-build git
            warn "A reboot may be required for rpm-ostree changes to take effect"
            ;;
        opensuse*|suse*)
            info "Installing dependencies (zypper)..."
            $SUDO zypper install -y \
                qt6-base-devel qt6-svg-devel cmake gcc-c++ ninja git
            ;;
        void)
            info "Installing dependencies (xbps)..."
            $SUDO xbps-install -Sy \
                qt6-base-devel qt6-svg-devel cmake gcc ninja git
            ;;
        gentoo)
            info "Installing dependencies (emerge)..."
            $SUDO emerge --noreplace \
                dev-qt/qtbase dev-qt/qtsvg dev-build/cmake sys-devel/gcc dev-build/ninja dev-vcs/git
            ;;
        *)
            if [[ "$DISTRO_ID_LIKE" == *"arch"* ]]; then
                info "Arch-based distro detected, using pacman..."
                $SUDO pacman -S --needed --noconfirm \
                    qt6-base qt6-svg cmake gcc ninja git
            elif [[ "$DISTRO_ID_LIKE" == *"debian"* || "$DISTRO_ID_LIKE" == *"ubuntu"* ]]; then
                info "Debian-based distro detected, using apt..."
                $SUDO apt update -qq
                $SUDO apt install -y \
                    qt6-base-dev qt6-svg-dev cmake g++ ninja-build git
            elif [[ "$DISTRO_ID_LIKE" == *"fedora"* ]]; then
                info "Fedora-based distro detected, using dnf..."
                $SUDO dnf install -y \
                    qt6-qtbase-devel qt6-qtsvg-devel cmake gcc-c++ ninja-build git
            else
                warn "Unknown distribution: ${DISTRO_ID}"
                warn "Please install manually: Qt6 (base + svg), CMake >= 3.20, GCC >= 11, Ninja"
                read -rp "Continue anyway? [y/N]: " ans
                [[ "$ans" =~ ^[Yy]$ ]] || exit 1
            fi
            ;;
    esac

    success "Dependencies installed"
}

# ── Build ─────────────────────────────────────────────────
cmd_build() {
    info "Building linuxeye..."

    mkdir -p "$SRCDIR/build"
    cd "$SRCDIR/build"

    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -G Ninja

    ninja -j"$(nproc)"

    success "Build complete! Binary at: ${BOLD}build/src/linuxeye${NC}"
}

# ── Install ───────────────────────────────────────────────
cmd_install() {
    need_root

    # Build first if not already built
    if [[ ! -f "$SRCDIR/build/src/linuxeye" ]]; then
        cmd_build
    fi

    cd "$SRCDIR/build"

    info "Installing linuxeye to /usr/local/bin..."
    $SUDO ninja install

    # Polkit policy
    if [[ -f "$SRCDIR/data/io.linuxeye.collect.policy" ]]; then
        info "Installing polkit policy..."
        $SUDO install -Dm644 \
            "$SRCDIR/data/io.linuxeye.collect.policy" \
            /usr/share/polkit-1/actions/io.linuxeye.collect.policy
        success "Polkit policy installed"
    fi

    # Desktop entry
    if [[ -f "$SRCDIR/data/linuxeye.desktop" ]]; then
        info "Installing desktop entry..."
        $SUDO install -Dm644 \
            "$SRCDIR/data/linuxeye.desktop" \
            /usr/share/applications/linuxeye.desktop
        success "Desktop entry installed"
    fi

    echo ""
    success "Installation complete! Run ${BOLD}linuxeye${NC} to start."
}

# ── Uninstall ─────────────────────────────────────────────
cmd_uninstall() {
    need_root
    info "Uninstalling linuxeye..."

    $SUDO rm -f /usr/local/bin/linuxeye
    $SUDO rm -f /usr/local/bin/linuxeye-helper
    $SUDO rm -f /usr/share/polkit-1/actions/io.linuxeye.collect.policy
    $SUDO rm -f /usr/share/applications/linuxeye.desktop

    success "linuxeye uninstalled"
}

# ── Remove build ──────────────────────────────────────────
cmd_rmbuild() {
    if [[ -d "$SRCDIR/build" ]]; then
        info "Removing build directory..."
        rm -rf "$SRCDIR/build"
        success "Build directory removed"
    else
        warn "No build directory found"
    fi
}

# ── Uninstall dependencies ────────────────────────────────
cmd_uninstalldep() {
    detect_distro
    need_root
    info "Removing build dependencies..."

    case "$DISTRO_ID" in
        arch|cachyos|endeavouros|manjaro|garuda|arcolinux|artix)
            $SUDO pacman -Rns --noconfirm \
                qt6-base qt6-svg cmake ninja 2>/dev/null || true
            ;;
        ubuntu|debian|linuxmint|pop|zorin|elementary)
            $SUDO apt remove -y \
                qt6-base-dev qt6-svg-dev cmake ninja-build 2>/dev/null || true
            $SUDO apt autoremove -y
            ;;
        fedora|nobara)
            $SUDO dnf remove -y \
                qt6-qtbase-devel qt6-qtsvg-devel cmake ninja-build 2>/dev/null || true
            ;;
        bazzite|silverblue)
            rpm-ostree uninstall \
                qt6-qtbase-devel qt6-qtsvg-devel cmake ninja-build 2>/dev/null || true
            warn "A reboot may be required for rpm-ostree changes to take effect"
            ;;
        opensuse*|suse*)
            $SUDO zypper remove -y \
                qt6-base-devel qt6-svg-devel cmake ninja 2>/dev/null || true
            ;;
        void)
            $SUDO xbps-remove -Ry \
                qt6-base-devel qt6-svg-devel cmake ninja 2>/dev/null || true
            ;;
        gentoo)
            $SUDO emerge --deselect \
                dev-qt/qtbase dev-qt/qtsvg dev-build/cmake dev-build/ninja 2>/dev/null || true
            ;;
        *)
            warn "Unknown distribution: cannot auto-remove dependencies"
            ;;
    esac

    success "Build dependencies removed"
}

# ── Help ──────────────────────────────────────────────────
cmd_help() {
    echo -e "${BOLD}${CYAN}"
    echo "  ╔══════════════════════════════════════╗"
    echo "  ║     linuxeye – Setup Script          ║"
    echo "  ║     System Analyzer for Linux        ║"
    echo "  ╚══════════════════════════════════════╝"
    echo -e "${NC}"
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  --install        Build and install linuxeye to system"
    echo "  --installdep     Install build dependencies only"
    echo "  --build          Build the project (no system install)"
    echo "  --uninstall      Remove linuxeye from system"
    echo "  --uninstalldep   Remove build dependencies"
    echo "  --rmbuild        Remove the build directory"
    echo "  --help           Show this help message"
    echo ""
}

# ── Main ──────────────────────────────────────────────────
main() {
    case "${1:-}" in
        --install)      cmd_install      ;;
        --installdep)   cmd_installdep   ;;
        --build)        cmd_build        ;;
        --uninstall)    cmd_uninstall    ;;
        --uninstalldep) cmd_uninstalldep ;;
        --rmbuild)      cmd_rmbuild      ;;
        --help)         cmd_help         ;;
        *)              cmd_help         ;;
    esac
}

main "$@"
