# KriTap – Mobile-Friendly Krita Fork

**English**

**KriTap** is a fork of [Krita](https://krita.org/) designed to make the full power of Krita accessible on smartphones and tablets.  
While Krita offers professional-grade digital painting tools, the original mobile builds are essentially desktop versions squeezed onto a small screen, making them hard to use.

**Goal:**  
Bring **all the high-end functionality of Krita** to mobile devices, with a **UI and input system optimized for touch**, while keeping every feature intact.

**Key Features & Concepts:**
- **Touch-Optimized UI:** Replace desktop-oriented widgets with **Qt Quick/QML components** for natural finger interactions.
- **Smart Tool Access:** Circular or sliding tool menus accessible via tap and swipe, minimizing on-screen clutter.
- **Full Krita Engine:** Retains the original C++ rendering engine, brushes, layers, filters, and file format compatibility.
- **Mobile-Friendly Workflow:** Modes for one-handed operation and pro mode for advanced users.

**Why KriTap?**
- Makes Krita **usable on smartphones** without sacrificing features.
- Leverages existing Krita engine—no compromise on professional capabilities.
- Introduces a new **gesture- and touch-oriented interface**, designed for efficient mobile drawing.

**Installation & Build**
This fork is intended for **Android (via Qt for Android)**. Refer to the build instructions in the repository for setting up the environment and compiling the app.  

---

**日本語**

**KriTap** は [Krita](https://krita.org/) をフォークしたもので、スマートフォンやタブレットでもKritaの全機能を快適に使えるように設計されています。  
Kritaはプロ仕様のデジタルペイントツールですが、既存のモバイル版はデスクトップ版をそのまま小さい画面に押し込んだ状態で、使いにくいのが現状です。

**目標:**  
Kritaの**高機能をそのまま保持**しつつ、**タッチ操作に最適化されたUIと入力システム**を実装し、スマホでも快適に使えるようにすること。

**主な特徴とコンセプト:**
- **タッチ最適化UI:** デスクトップ向けのウィジェットを **Qt Quick/QML コンポーネント** に置き換え、指で自然に操作可能。
- **スマートツールアクセス:** 円形またはスライド式のツールメニューで、画面を整理しながら簡単に操作可能。
- **Kritaエンジンをそのまま:** C++描画エンジン、ブラシ、レイヤー、フィルター、ファイル形式の互換性を保持。
- **モバイル向けワークフロー:** 片手操作モードとプロモードを用意し、用途に応じて切替可能。

**なぜKriTapか？**
- 高機能Kritaをスマホでも**快適に利用可能**。
- 既存のKritaエンジンをそのまま利用、プロ仕様の性能を損なわない。
- **ジェスチャー・タッチ操作中心のUI** により、効率的なモバイル描画体験を提供。

**インストールとビルド**
このフォークは **Android（Qt for Android 経由）向け** を想定しています。  
環境構築とビルド方法はリポジトリ内のドキュメントを参照してください。


## It's original readme from Krita!! Thanks Krita!
![Picture](https://krita.org/images/krita-logo-light.svg)

| CI Name     | Master | Stable | Release |
| ------------------- | ---------------- | ------ | ------- |
| Pipeline | [![pipeline status](https://invent.kde.org/graphics/krita/badges/master/pipeline.svg)](https://invent.kde.org/graphics/krita/-/commits/master) | [![pipeline status](https://invent.kde.org/graphics/krita/badges/krita/5.2/pipeline.svg)](https://invent.kde.org/graphics/krita/-/commits/krita/5.2) | [![Latest Release](https://invent.kde.org/graphics/krita/-/badges/release.svg)](https://invent.kde.org/graphics/krita/-/releases) |

Note: Nightly builds are not covered by this table atm

Krita is a free and open source digital painting application. It is for artists who want to create professional work from start to end. Krita is used by comic book artists, illustrators, concept artists, matte and texture painters and in the digital VFX industry.

If you are reading this on GitHub, be aware that this is just a mirror. Our real code repository is provided by KDE: https://invent.kde.org/graphics/krita.git

![Picture](https://krita.org/images/hero-image-50.webp)

### Repository Status

For branch: `master`

| Freeze type    | Status                      |
|----------------|-----------------------------|
| Feature Freeze | no freeze, features allowed |
| String Freeze  | no freeze, strings allowed  |


### User Manual
https://docs.krita.org/en/user_manual.html

### Development Notes and Build Instructions

Please follow [the online documentation](https://docs.krita.org/en/untranslatable_pages/building_krita.html).

Other developer guides, notes and wiki:

https://docs.krita.org/en/untranslatable_pages.html

Apidox:

https://api.kde.org/legacy/krita/html/index.html

### Bugs and Wishes

https://bugs.kde.org/buglist.cgi?bug_status=UNCONFIRMED&bug_status=CONFIRMED&bug_status=ASSIGNED&bug_status=REOPENED&list_id=1315444&product=krita&query_format=advanced

### Discussion Forum

* https://krita-artists.org/

### IRC channel

Most of the developers hang out here. If you are interested in helping with the project this is a great place to start.

libera.chat, #krita

### Project Website

https://www.krita.org

### Nightly builds

#### Unstable

* https://cdn.kde.org/ci-builds/graphics/krita/master/

#### Stable

* https://cdn.kde.org/ci-builds/graphics/krita/krita-5.2/

#### Developers builds

##### Linux build with debug symbols in Qt and Krita

1) Go to Jobs section of Krita's CI: https://invent.kde.org/graphics/krita/-/jobs
2) Search for the latest `linux-debug-weekly` job
3) Enter the job and click on Artifacts->Browse
4) Download the AppImage

##### Linux build with ASAN in Qt and Krita

1) Go to Jobs section of Krita's CI: https://invent.kde.org/graphics/krita/-/jobs
2) Search for the latest `linux-asan-weekly` job
3) Enter the job and click on Artifacts->Browse
4) Download the AppImage
5) Set up environment variable for ASAN:
    ```bash
        export ASAN_OPTIONS=new_delete_type_mismatch=0:detect_leaks=0
    ```
6) Run the AppImage in the modified environment

##### Windows build with ASAN in Qt and Krita

1) Go to Jobs section of Krita's CI: https://invent.kde.org/graphics/krita/-/jobs
2) Search for the latest `windows-asan-weekly` job
3) Enter the job and click on Artifacts->Browse
4) Download the .zip file
5) Open terminal
6) Set up environment variable for ASAN:
    ```
        set ASAN_OPTIONS=new_delete_type_mismatch=0:detect_leaks=0
    ```
7) Change working directory to `c:\path\where\you\downloaded\krita-5.3.0-prealpha-git12345\bin`.
   That is important, otherwise ASAN will not be able to locate llvm-symbolizer.exe and the
   backtraces generated by ASAN will not contain proper symbols.
    ```
        cd c:\path\where\you\downloaded\krita-5.3.0-prealpha-git12345\bin
    ```
8) Run krita
    ```
        krita.com
    ```

### License

Krita as a whole is licensed under the GNU Public License, Version 3. Individual files may have a different, but compatible license.
