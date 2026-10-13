# Spell Meadow 開発ガイド

このソリューションは、タイトル・草原・城を切り替えるDirect3D 11のゲームです。まずここから担当クラスと変更箇所を確認してください。

## プロジェクト構成

| 場所 | 役割 |
| --- | --- |
| `BaseFramework.sln` / `Project.vcxproj` | Visual Studioのソリューションとゲームプロジェクト |
| `Src/Application` | ゲーム固有のルール、シーン、キャラクター、UI |
| `Src/Framework` | 描画、入力、音声、モデル読み込みなどの共通基盤 |
| `Asset` | 実行時に読むモデル、画像、音声 |
| `Save` | 育成情報、城の配置、安全地帯の設定 |
| `../Library` / `../asio-1.38.2` | ソリューション外の依存。プロジェクトだけコピーすると不足する |

## 最初に読むクラス

1. `Src/Application/Scene/SceneManager.*`：タイトルとゲームの切り替え。光学迷彩の背景用レンダーターゲットも管理。
2. `Scene/GameScene/GameScene.*`：草原・城の所有者。ステージを越えて残るStatusと怨念の共有状態を保持。
3. `Scene/GameStageBase/GameStageBase.*`：プレイヤー・カメラ・杖の作成、BGM、一時停止、共通UI。
4. `GrassStage/GrassStage.*` / `CastleStage/CastleStage.*`：各ステージの配置、入退場の条件、敵の生成。
5. `GameObject/Character/Status/Status.*`：HP、育成状態、魔法選択とHUD。

Frameworkを変更するのは、複数のゲームオブジェクトに共通する機能の変更が必要な場合です。敵の強さや配置などはApplication側で調整します。

## ステージ切り替えと所有関係

ステージは移動条件が成立するとGameSceneのコールバックを呼び、次のステージを予約します。オブジェクトの更新中には旧ステージを破棄せず、次フレームの`PreUpdate`で切り替えます。

- `shared_ptr`：オブジェクトの寿命を所有する場所。
- `weak_ptr`：プレイヤーやカメラを参照する場所。使用前に`lock()`を確認します。
- GameSceneのコールバックは、そのGameSceneが所有するステージだけで使用します。別のシーンへ持ち出さないでください。

入場経路と到着点は`StageTransition.h`のenumで指定します。

| 操作 | 到着点 |
| --- | --- |
| 草原の城から入場 | 城の入口 |
| 村のポータルから入場 | 城のポータル |
| 城の通常出口 | 草原側の城入口 |
| 城のポータル | 村のポータル |
| 城で死亡 | 村の開始・復活地点 |

## よく変更する設定

| 変更したいもの | 編集箇所 |
| --- | --- |
| 敵の最大数、スポーン距離、怨念の能力値、倍率の増減 | `GameObject/Character/Status/GameBalance.h` |
| 村の開始・復活位置、村ポータル位置 | `GrassStage.cpp`の`spawnPos` / `VillagePortalPos` |
| 城の入口・ポータル位置 | `CastleStage.cpp`の`castleEntrancePos` / `CastlePortalPos` |
| 草原の城の配置距離、入口の判定 | `GameObject/Character/Castle/NoneCastle.cpp` |
| 怨念の出現地点 | `CastleStage::CreateGrudges`の`spawnPoints` |
| Fキー案内の画像・サイズ・位置 | `GameObject/UI/InteractionPrompt.h` |
| メニューの見た目・ボタン | `GameObject/UI/PauseMenu.h` |
| 各ステージのBGM | 各ステージの`StartStageBgm`呼び出し |
| 城の安全地帯 | ゲーム内編集画面と`Save/CastleSafeArea.txt` |

ポータルの判定座標と表示座標を別々に変更しないでください。Fキーの表示と操作は`GetInteractionAction()`を共有しています。新しい操作を増やす場合も、この判定へ追加します。

## 怨念の持ち越し

`GrudgeBattleState`はGameSceneが所有し、城ステージはそれを参照します。

- 残数は`TotalCount - DefeatedCount`。
- 草原の撃破数は城への入場時に合計へ追加して消費。
- 残った敵の復元分を合計へもう一度加算しないこと。
- 撃破時の強化と退城時の減衰は`StrengthBonus`を変更し、撃破累計を変更しないこと。
- 死亡・ポータル・出口の減衰は、実際の切り替え時に一度だけ行う。

現在の持ち越しは同じGameSceneの寿命内だけです。タイトルへの移動やアプリ終了を越える保存は実装されていません。再入場時の敵は残数・倍率から再生成し、個体ごとの位置や残りHPは保持しません。

## 更新・描画の注意点

メニュー表示中は全オブジェクトの通常更新とPostUpdateを止めます。魔法選択中はStatusだけ通常更新します。ステージのEvent内で操作を追加する際にも一時停止条件を確認してください。

ImGuiは`KdDebugGUI::GuiProcess()`がフレームを開始した後、オブジェクトの`DrawDebugGui()`から使用します。`Update()`から`ImGui::Begin()`を呼ばないでください。PauseMenuは画像を使ったスプライト描画です。通常HUDの後に暗い背景・枠・文字・カーソルの順で描画し、入力は一時停止中もステージのEventから処理します。

BatとGrudgeBatは共有モデル・共有羽ばたき姿勢を使います。個体別の被弾、ディゾルブ、光学迷彩設定は`KdModelVisualEffects`としてバッチへ渡します。共有マテリアルへ個体の色を書き込むと全員へ反映されます。

怨念は影を描画しません。光学迷彩では背景用の描画と最終描画があるため、大量の敵の影を再度有効にすると負荷が増えます。描画設定が異なる個体は別バッチになります。

## ビルドと動作確認

Visual Studioで`Debug | x64`を選んでビルドします。プロジェクト設定は`v145`です。この環境で確認してきたコマンドビルドはVisual Studio 2022のMSBuildを使用し、`PlatformToolset=v143`を指定しています。必要なツールセットが違う場合は実行環境に合わせてください。

実行時のAssetとSaveはプロジェクトのルートを基準にします。起動時にexeの親フォルダからAssetを探す処理があります。パスエラーは警告を無視して進めず、まず表示された読み込み先と実在ファイルを照合してください。

変更後は、関連する経路を選んで確認します。

- タイトル→草原→城。城入口とポータルで到着点が違うこと。
- F案内の表示と操作範囲。退出前の敵が残っている場合もポータルが使えること。
- 城で途中死亡→村→再入場。残数と退城時の倍率減少。
- Escメニュー開閉。敵・プレイヤーが停止し、復帰後のカメラが飛ばないこと。
- BGMの切り替え。旧曲が重ならないこと。

ビルド成功は、ゲーム内の位置・画像・音声・負荷の確認を代替しません。

## コードを追加するとき

- 設定値には名前と単位を付け、同じ値を複数箇所へ埋め込まない。
- 意味の異なる移動先をbooleanで表現せず、enumを使用する。
- コメントでは「なぜ必要か」「誰が所有するか」「変更時の制約」を説明する。
- アセットを追加したらコードの参照だけでなく、glTFのbin・画像URIも確認する。
- 新しいヘッダーをVisual Studioで編集できるよう、vcxprojとfiltersへ登録する。
- 第三者ライブラリは直接編集せず、ApplicationまたはFramework側で変更を完結させる。
