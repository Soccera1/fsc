# Maintainer: Your Name <youremail@domain.com>
pkgname=fsc
# pkgver is dynamically generated from git tags and commits
pkgrel=1
pkgdesc="A skeleton X compositor for fractional scaling"
arch=('x86_64')
url="https://github.com/soccera1/fsc"
license=('GPL3')
depends=(
  'libx11'
  'libxrandr'
  'libxrender'
  'libxcomposite'
)
optdepends=(
  'xorg-server-xephyr: for testing in a nested X server'
  'dwm: for testing with dwm'
)
makedepends=('git')

# Source from the Git repository, specifying the master branch
source=("git+${url}.git#branch=master")
# Skip md5sums for git sources as they change frequently
md5sums=('SKIP')

pkgver() {
  cd "$srcdir/$pkgname"
  # Generates a version string like 1.0.r123.abcdef
  git describe --long --tags --abbrev=7 | sed 's/^v//;s/\([^-]*-\)/\1r/;s/\([^-]*\)-\([^-]*\).*/\1\2/'
}

build() {
  cd "$srcdir/$pkgname"
  # Copy config.def.h to config.h if it doesn't exist
  [[ ! -f config.h ]] && cp config.def.h config.h
  make
}

package() {
  cd "$srcdir/$pkgname"
  # Install the fsc executable
  install -Dm755 fsc "$pkgdir/usr/bin/fsc"
  # Install documentation
  install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
  # Install license file
  install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
