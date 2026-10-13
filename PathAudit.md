# パス確認結果

2026-10-13。確認のみで、ソースコードやプロジェクト設定の修正は行っていません。

## 確認範囲

- BaseFramework.sln の4プロジェクトと、それぞれのプロジェクト内のファイル参照。
- インクルード・ライブラリの検索フォルダ。Debug/Release、x64/Win32へ置換して存在確認。プロジェクト独自の条件で有効になるかまでは評価していません。
- Project.vcxproj.filters のファイル参照190件。
- ローカル Src の引用符形式の include 269件。
- ローカル Src の Asset/、Save/、ドライブ指定で始まる固定文字列86件。
- Asset 内のglTFの外部バッファ・画像URI24件。
- Src/Pch.h の明示リンクライブラリ11件。外部ライブラリ7件は検索フォルダに存在し、4件はWindows SDK/MSVCの標準検索対象です。

各参照の場所、参照文字列、解決先、存在確認結果は PathAudit.csv に記録しています。繰り返し記述された参照も含む件数です。

## 見つかった不足・注意点

1. Src/Framework/Effekseer/KdEffekseerManager.h:180 の Asset/Data/Effect/ は存在しません。KdEffekseerManager.cpp:200 でエフェクト名を連結して読み込む際の基準フォルダです。現在のローカルSrcではエフェクトファイル名やゲーム側のPlay呼び出しが見つからず、直ちにゲームに影響するとは判断していません。
2. 外部の Library/DirectXTK/DirectXTK_Desktop_2022.vcxproj が参照する README.md は存在しません。ドキュメント項目であり、コンパイル対象ではありません。
3. Src/Application/GameObject/Stage/WarpStone.h は実在し、includeも解決しますが、Project.vcxproj / filters のファイル一覧には未登録です。ビルド可能ですが、Visual Studioのプロジェクトツリーには表示されません。
4. Src/Framework/Utility/KdDebug/KdDebugGUI.cpp:39 は c:\Windows\Fonts\msgothic.ttc を固定指定しています。このPCでは存在しますが、Windowsの場所やフォント構成に依存します。
5. 外部依存は ../Library と ../asio-1.38.2/include を参照しています。現在は存在しますが、ソリューションを別の場所へ移すときは隣接フォルダも必要です。
6. 実行時のAsset/とSave/は作業ディレクトリ基準です。Project.vcxproj.userでは $(ProjectDir) が指定されています。exeを直接起動する場合は作業ディレクトリによって解決先が変わります。

## 正常だった参照

ソリューションのプロジェクト参照、各プロジェクトのコンパイル対象・ヘッダー・シェーダー、ローカルSrcの引用符include、ゲーム側の固定アセット/保存先、glTF外部ファイル、確認した検索フォルダには、上記以外の欠落は見つかりませんでした。

## 確認の限界

MSBuildの環境マクロを含むImport 34件はCSVでDynamicとしています。すべての構成のMSBuild評価・ビルドや実行時ロードを行った結果ではありません。ソースで組み立てる動的パス、標準の山括弧include、外部Library/Asioのソース内部やアセットのバイナリ内部のパスまでは総当たり検証していません。既存のDebug/x64ビルド成功は他構成の保証ではありません。
