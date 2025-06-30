# Electric Guitar Simulator

C++ ve SDL2 kullanılarak yapılmış elektro gitar simülatörü. Tıkladığınız notaları çalar ve gerçek gitar frekanslarını kullanır.

## Özellikler

- 6 telli gitar simülasyonu (standart akord: E-A-D-G-B-E)
- 12 perdeli fretboard
- Gerçek gitar frekansları
- Tıklayarak nota çalma
- Görsel fretboard ve perde işaretleri
- Harmoniklerle zenginleştirilmiş ses üretimi

## Gereksinimler

- C++17 uyumlu derleyici (GCC, Clang, MSVC)
- CMake 3.16+
- SDL2
- SDL2_mixer

### Windows'ta Kurulum

1. **VCPKG ile SDL2 kurulumu:**
```powershell
vcpkg install sdl2 sdl2-mixer
```

2. **Veya Manuel Kurulum:**
   - SDL2 Development Libraries'i indirin: https://www.libsdl.org/download-2.0.php
   - SDL2_mixer'i indirin: https://www.libsdl.org/projects/SDL_mixer/
   - Lib ve include klasörlerini projeye ekleyin

### Linux'ta Kurulum

```bash
# Ubuntu/Debian
sudo apt install libsdl2-dev libsdl2-mixer-dev cmake build-essential

# Fedora
sudo dnf install SDL2-devel SDL2_mixer-devel cmake gcc-c++

# Arch Linux
sudo pacman -S sdl2 sdl2_mixer cmake gcc
```

## Derleme

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Windows (Visual Studio ile)

```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Çalıştırma

```bash
# Linux/Mac
./ElectricGuitar

# Windows
./ElectricGuitar.exe
```

## Kullanım

1. Program açıldığında gitar fretboard'ını göreceksiniz
2. Herhangi bir perde üzerine tıklayın
3. O pozisyondaki nota çalacaktır
4. Konsol çıktısında hangi notanın çaldığını görebilirsiniz

## Gitar Akordları

- 1. Tel (En İnce): E4 (329.63 Hz)
- 2. Tel: B3 (246.94 Hz) 
- 3. Tel: G3 (196.00 Hz)
- 4. Tel: D3 (146.83 Hz)
- 5. Tel: A2 (110.00 Hz)
- 6. Tel (En Kalın): E2 (82.41 Hz)

## Yapı

```
src/
├── main.cpp         # Ana program ve SDL initialization
├── Guitar.h/cpp     # Gitar sınıfı ve fretboard rendering
└── AudioManager.h/cpp # Ses üretimi ve yönetimi
```

## Geliştirme Notları

- Ses üretimi gerçek zamanlı sine wave synthesis ile yapılır
- Her nota için harmonikler eklenerek daha gerçekçi gitar sesi elde edilir
- Frekanslar matematik formülle hesaplanır: f = f₀ × 2^(n/12)
- SDL2_mixer kullanılarak multiple ses kanalları desteklenir

## Lisans

Bu proje eğitim amaçlı geliştirilmiştir. Özgürce kullanabilir ve geliştirebilirsiniz. 