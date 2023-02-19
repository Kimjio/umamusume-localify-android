# umamusume-localify-android
Android port of [umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify)

[韓国語](README.ko-KR.md)

## 使用方法
1. [Magisk](https://github.com/topjohnwu/Magisk) v24またはそれ以降をインストールし、Zygiskを有効化してください。
2. [Shamiko](https://github.com/LSPosed/LSPosed.github.io/releases)をインストールし、SafetyNetによる対策を回避する設定をしてください。 (DenyListを有効化するとモジュールが動作しません)
3. Magiskにumamusume-localify-androidのモジュールをインストールしてください。
4. `config.json` ファイルを `/sdcard/android/data/jp.co.cygames.umamusume/` `dicts`に参照されている翻訳ファイルを配置します。
## 設定アプリ
jsonファイルを編集せずに設定可能なアプリを作成しました。

インストールは [GitHubのリリース](https://github.com/Kimjio/umamusume-localify-android/releases) または [Playストア](https://play.google.com/store/apps/details?id=com.kimjio.umamusumelocalify.settings)からダウンロードができます

## Note
Working directory is `/sdcard/Android/data/jp.co.cygames.umamusume/` 

(Korean version is `/sdcard/Android/data/com.kakaogames.umamusume/`)

## Config
- `enableLogger` Outputs an unoverwritten string to `data.txt` (`true` / `false`, Default: `false`)
- `dumpStaticEntries` Outputs a hard-coded string to `data.txt` (`true` / `false`, Default: `false`)
  - `enableLogger` must be enabled.
- `dumpDbEntries` Outputs DB data to `original_data/` (`true` / `false`, Default: `false`)
- `maxFps` Max FPS settings (`-1` = Default / `0` = Unlimited / `n > 0` = Limit to n, Default: `-1`)
- `uiAnimationScale` Change UI animation scale (0 < ~, Default: `1`)
  - Caution: Soft lock occurs when set to `0`.
- `uiUseSystemResolution` Renders UI, live subtitles, and more at system resolution. (`true` / `false`, Default: `false`)
- `replaceToBuiltinFont` (`replaceFont`) Use Unity builtin fonts instead of in-game fonts. (`true` / `false`, Default: `false`)
- `replaceToCustomFont` Use custom fonts instead of in-game fonts. (`true` / `false`, Default: `false`)
- `fontAssetBundlePath` Font asset bundle path (ex: `custom_font/GyeonggiTitle_Medium/font`)
- `fontAssetName` Font asset name (ex: `GyeonggiTitle_Medium.fontsettings`)
- `graphicsQuality` Change predefined graphics settings. (`-1` ~ `4`, Default: `-1`)
  - `-1`: Follow the App's choice
  - `0`: `Toon1280`, MSAA OFF
  - `1`: `Toon1280x2`, MSAA x2
  - `2`: `Toon1280x4`, MSAA x4
  - `3`: `ToonFull`, MSAA x8
- `antiAliasing` Change MSAA settings (`-1`, `0`, `2`, `4`, `8`, Default: `-1`)
  - `-1`: Follow the Graphics settings
  - `0`: MSAA OFF
  - `2`: MSAA x2
  - `4`: MSAA x4
  - `8`: MSAA x8
- `forceLandscape` Force all screens to landscape. (`true` / `false`, Default: `false`)
- `forceLandscapeUiScale` Sets the UI scale to use for forced landscape mode. (0 < ~, Default: `1.8`)
- `uiLoadingShowOrientationGuide` Set whether to display the loading guide when the screen is rotated (`true` / `false`, Default: `true`)
- `replaceAssetsPath` Set the path to the folder containing the assets to replace the downloaded assets in the game.
  - The name of the asset file to be replaced must be the same as the hash name of the original asset file.
  - ex: `2FOXNDZ5H52B3E4JMXVAJ4FRMDE3PX7Q` (Asset containing the Home footer texture and scripts (Android))
- `dicts` Translation file list (List of translation file paths since `/sdcard/Android/data/jp.co.cygames.umamusume/`)

## Known issue
When `maxFps` is set to unlimited, the frame of the game is played from 60 FPS to 30 FPS, which is halved. (Not a typical 30FPS)

## Build
1. Download the source code
2. Use Android Studio to run the gradle task `:module:assembleRelease` to compile, the zip package will be generated in the `out` folder
