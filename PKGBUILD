pkgname=plasma5-applets-cpufrequtility
pkgver=1.4.5
pkgrel=1
pkgdesc="CPU Frequency Utility plasmoid"
arch=('i686' 'x86_64')
url="https://github.com/F1ash/CPUFreqUtility/"
license=('GPL2')
depends=('qt5-base' 'kauth' 'knotifications' 'dbus' 'systemd' 'polkit')
makedepends=('cmake' 'desktop-file-utils' 'extra-cmake-modules')
source=("https://github.com/haasosaurus/CPUFreqUtility/archive/master.zip")
md5sums=('SKIP')

build() {
  mkdir build
  cd build
  cmake "../CPUFreqUtility-master" \
    -DCMAKE_INSTALL_PREFIX=$(kf5-config --prefix) \
    -DCMAKE_BUILD_TYPE=Release
  make
}

package() {
  cd build
  make DESTDIR="$pkgdir/" install
}
