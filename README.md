# Project Introduction

這是一個使用 C++ 實作的課程專案，目標是 1:1 完全復刻 Angry Birds 遊戲

# Notice:
Please change `PTSD/cmake/Dependencies.cmake` file in line 93 and 94 to turn on the sound:

```cmake
set(SDL2MIXER_OGG ON)
set(SDL2MIXER_VORBIS STB)
```

⚠️ 本專案的資源路徑（圖片、音效等）是在**編譯時**寫死的，因此**請務必在 clone 下來的同一個位置進行 build，build 完成後也不要搬動專案資料夾**，否則遊戲會讀不到資源。

完整的環境安裝、build 與執行步驟，請參考 [Instruction.md](./Instruction.md)（內含常見問題排解）。

## Recommendation Settings

In Windows 11, display scaling 150%

The game window size is fixed at compile time in `PTSD/include/config.hpp`:
```cpp
#define WINDOW_WIDTH 2400
#define WINDOW_HEIGHT 1350
```
If the window is too large for your screen (e.g. on a smaller display or 
without 150% scaling), you can lower these values and rebuild, for example:
```cpp
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
```

## Developing Time

2026 Department of Computer Science and Information Engineering, National Taipei University of Technology

OOPL 2026 Spring Semester

## Team Member

- 111310452 黃安華 (PM)
- 113590039 許兆雲 

## Game Introduction
Angry Birds 是一款 2D 多關卡益智遊戲，玩家透過彈弓發射不同種類的 Angry Bird 來攻擊豬的防禦堡壘。豬會用木頭、石頭、冰塊等不同材料建造防禦，而不同種類的 Angry Bird 也各有獨特的異能和特效，玩家需要策略性地選擇使用。

- [遊戲連結](https://www.angrybirds.com/)
- [遊玩畫面](https://www.youtube.com/watch?v=tmcG-vbI6DQ)

## Milestones
- [期中成果](https://youtu.be/V9pe8lg6LLU)
- [期末成果](https://youtu.be/aVP6Vbt74fo) (cc subtitles included)
- [Final Report](./2026OOPL_Final.md)
- [Source Code Release (v1.1)](../../releases/tag/v1.1)

## Developing Tool

- AI Assisted IDE: Copilot, Antigravity
- Game Engine: 基於 [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design) framework 修改，原始碼已直接整合進本 repo（非 submodule，clone 時不需要 `--recursive`）

## [Development Timeline Planning](./Proposal.md)