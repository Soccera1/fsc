# Copyright 2024 The Gentoo Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v3.0

EAPI=8

DESCRIPTION="A skeleton X compositor for fractional scaling"
HOMEPAGE="https://github.com/soccera1/fsc"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64"
IUSE="xephyr"

# Build dependencies
DEPEND="
	x11-libs/libX11
	x11-libs/libXrandr
	x11-libs/libXrender
	x11-libs/libXcomposite
"
# Runtime dependencies (optional, for testing with Xephyr and dwm)
RDEPEND="
	>=x11-base/xorg-server-1.10.0:0/1.10.0[xephyr]
	x11-wm/dwm? ( x11-wm/dwm )
"

# Live ebuild specific variables
GIT_URI="git://github.com/soccera1/fsc.git"
GIT_BRANCH="master"

# Do not mirror git repositories
RESTRICT="mirror"

src_unpack() {
	git_src_unpack
}

src_prepare() {
	default_src_prepare

	# Copy config.def.h to config.h if it doesn't exist
	if [[ ! -f config.h ]]; then
		cp config.def.h config.h
	fi
}

src_compile() {
	local emake_args=()
	if use xephyr; then
		emake_args+=( XEPHYR_SUPPORT=1 )
	fi
	emake "${emake_args[@]}"
}

src_install() {
	default_src_install

	dobuild
	dobins ${PN}

	doins README.md
	doins LICENSE
}
