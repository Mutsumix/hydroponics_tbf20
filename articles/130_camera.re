= 定点カメラで自動撮影しよう

この章では、植物の成長をカメラで記録する方法について説明します。

ネットワークカメラを使った監視の方法については@<chapref>{110_start} で触れました。
しかし、毎日定刻に写真を撮影することはできませんし、その写真をどこかのサーバーにアップロードすることもできませんでした。

あくまでもメーカーであるSwitchBotのアプリ上で、現在の様子を見たり、誰かがカメラの近くに来た時の様子を録画して後から確認することがメインです。
これは監視を目的としたネットワークカメラである以上、求めること自体が無理があると言えます。

そこで、市販のWebカメラを小型のPCに接続して、定点カメラの撮影を自動化する方法についてこの章では紹介していきたいと思います。

== Raspberry Piについて

手のひらに収まる小さなパソコン、Raspberry Pi（ラズベリーパイ） をご存じでしょうか。
一見するとおもちゃのような小型PCですが、その自由度と拡張性は驚くほど高く、ちょっとした工夫で本格的な撮影システムを構築することができます。

たとえば、USB接続のWebカメラを取り付ければ、ラズベリーパイを「自動撮影カメラ」として動かすことができます。
プログラムを組み合わせることで、指定した時間に自動で撮影を行い、撮った写真をGoogleフォトへ自動アップロードすることも可能です。
さらに、一定期間の写真をつなぎ合わせれば、まるでプロが作ったようなタイムラプス動画を生成することもできます。
数値だけではなく、成長の様子を画像で記録することで、より多くの発見や気づきを得ることができます。

「植物の成長記録を自動で残したい」、その思いをラズベリーパイで実現しましょう。

今回用意するものはこちらです。

== 用意するもの

@<b>{ハードウェア}

必ずしもこれが良い、というわけではありませんが、以下のもので動作検証をしています。

 * Raspberry Pi 4 Model B（4GB以上推奨）@<fn>{raspberry_pi}
 * microSDカード（32GB以上）
 * 市販のUSBウェブカメラ
  ** EMEET S600（画質を良くしたい場合）@<fn>{web_camera_4k}
  ** Logicool C270（安く済ませたい場合）@<fn>{web_camera_C270}
 * USB接続のキーボード・マウス
 * microHDMIと接続可能なHDMIケーブルとモニター
 * 電源アダプタ（USB Type-C、5V/3A以上）

//image[C270][Logicool C270][scale=0.25]
//image[EMMET_S600][EMMET S600][scale=0.25]

//footnote[raspberry_pi][@<href>{https://www.switch-science.com/products/5680} Raspberry Pi 4 Model B 4GB が初心者向けに使いやすくオススメです。]
//footnote[web_camera_4k][@<href>{https://amzn.asia/d/8sYe7tf} EMEET 4K Webカメラ]
//footnote[web_camera_C270][@<href>{https://amzn.asia/d/bam9mhN} Logitech C270 なんの変哲もない一般的なWebカメラ]

@<b>{アカウント}

 * Google アカウント（Google Photos用）
 * Google Cloud Platform（GCP）アカウント（無料枠で使用可能）

== 仕組みの説明

最初に全体像を図示します。

//image[camera_system_overview.drawio][全体の構成図][scale=0.75]

カメラと繋がったRaspberry Piが設定ファイルに従って撮影を行い、撮影した画像をクラウド（GCP）にアップロードします。
Google Photos Library APIと認可方式にはOAuth2.0を使用しています。

プログラムを実行すると、ループ処理が実行されます。
あらかじめ設定した時刻になると、接続したWebカメラでの撮影を行います。
撮影した画像はラズベリーパイ上には保存されず、直接クラウドにアップロードされます。
また次の設定した時刻に達するまで待機のループが始まるというのがプログラムの一連の流れとなります。

== ハードウェア側の設定

すでに動作するRaspberry Piが手元にある場合はこの手順はスキップしてください。

=== Raspberry Pi の初期設定（インストール〜起動）

==== Raspberry Pi Imagerのダウンロード

公式サイトから使用しているOSにあったものをダウンロード：@<href>{https://www.raspberrypi.com/software/}

//image[imager-download][Raspberry Pi Imagerのダウンロード画面][scale=0.5]

==== OSの書き込み

 * microSDカードをPCに接続（カードリーダー経由）
 * Raspberry Pi Imagerを起動

==== OSの書き込み設定

 * 「デバイスを選択」→「Raspberry Pi 4」
 * 「OSを選択」→「Raspberry Pi OS (64-bit)」
 * 「ストレージを選択」→ 接続したmicroSDカードを選択
 * 「次へ」をクリック
 * 「OSカスタマイズ設定を使いますか？」→「設定を編集する」

==== 詳細設定

@<b>{全般タブ：}

 * ホスト名: raspi-camera（任意、好きな名前で大丈夫）
 * ユーザー名: pi（任意ですが、以降の手順ではpiを前提にしています）
 * パスワード: 任意（忘れないように必ず記録しておきましょう）
 * Wi-Fi設定:@<fn>{wifi_setting}
 ** SSID: 自宅のWi-Fi名
 ** パスワード: Wi-Fiのパスワード
 ** 国: JP
 * ロケール設定:
 ** タイムゾーン: Asia/Tokyo
 ** キーボードレイアウト: jp

//footnote[wifi_setting][Wi-Fi設定をここで行うと、初回起動時から自動接続されます。モニターとキーボードだけで作業したい場合はここで設定しておくと便利です。]

@<b>{サービスタブ：}

 * SSHを有効化:@<fn>{ssh_setting}
 * 認証方法: 「パスワード認証を使う」を選択

「保存」→「はい」→「はい」（データが消去される警告）→書き込み開始

//footnote[ssh_setting][SSHを有効にすると、別のPCからリモート操作できるようになります。後述のVNC Viewerと組み合わせると、コピー＆ペーストが簡単になります。]

==== 初回起動

 * 書き込みが完了したら、microSDカードを安全に取り外し、Raspberry Piに挿入してください
 * キーボード・マウス・モニターを接続
 * 電源を接続して起動@<fn>{first_boot}

//footnote[first_boot][初回起動は2-3分かかります。デスクトップ画面が表示されれば成功です。]

=== Raspberry Pi の初期設定（Wi-Fi接続と外部操作の設定）

==== Wi-Fi接続の確認

画面右上のWi-Fiアイコンで接続状況を確認してください。
もしWi-Fiに接続されていない場合は次の作業を行なってください

 1. 右上のWi-Fiアイコンをクリック
 2. 接続先SSIDを選択
 3. パスワードを入力

==== Raspberry Pi の外部からの操作設定

@<b>{方法1. VNC Serverの有効化（推奨）}

ラズベリーパイに毎度キーボードやマウス、モニターを繋いでいるとせっかくの小型PCであることのメリットが薄れてしまいますし、扱いにくいです。
そこで別のPCからラズベリーパイに接続する方法を二つ紹介します。
まず紹介するのはVNCというツールを使って別PCからRaspberry Piの画面を見て操作する方法です。

@<b>{Raspberry Pi側}

 1. デスクトップ左上のRaspberry Piアイコン→「設定」→「Raspberry Piの設定」
 2. 「インターフェイス」タブに移動
 3. 「VNC」を「有効」に設定
 4. 「OK」を選択

@<b>{操作用PC側}

操作したいPC側にVNC Viewerをインストールします。

 * @<href>{https://www.realvnc.com/en/connect/download/viewer/}からダウンロードしてインストール
 * インストール後、Raspberry PiのIPアドレスを入力して接続
 * ユーザー名とパスワードでログイン

VNC Viewerを使うと、操作用PCからコピー＆ペーストができるため、長いコマンドやJSONファイルの転送が楽になります。
以降は、VNC Viewerを使う前提で、操作の説明を行なっていきます。

@<b>{方法2. SSH接続（慣れている人向け）}

画面を経由せずに、別PCのターミナルからRaspberry Piを操作することも可能です。それがSSH接続をする方法です。

操作用PCのターミナルから次のコマンドを実行します。

//cmd{
ssh pi@192.168.1.10
# IPアドレスはご自身のRaspberry Piのものに置き換えてください
//}

パスワードを入力してログインします。

==== IPアドレスの確認

VNC Serverを有効化すると、画面右上にVNCアイコンが表示されます。
このアイコンをクリックすると、Raspberry PiのIPアドレスが表示されます。
このアドレスをメモしておいてください。操作用PCからVNC接続やSSH接続に使用できます。

//image[ip_address][IPアドレスの確認][scale=0.75]

コマンドを打つことで確認も可能です。
デスクトップ上部のメニューバーから「アクセサリ」→「ターミナル」を開き、以下を実行：

//cmd{
hostname -I
//}

表示されたIPアドレス@<fn>{ip_address}（例: 192.168.1.10）をメモしておいてください。VNC接続やSSH接続に使用します。

//footnote[ip_address][IPアドレスは、ネットワーク上でRaspberry Piを識別するための番号です。]

==== システムの更新

ターミナルで以下を実行：

//cmd{
sudo apt update && sudo apt upgrade -y
//}

sudoは管理者権限で実行するコマンド、aptはソフトウェアをインストール・更新するためのツールです。
ラズベリーパイを触っていると、そんなつもりじゃないのにlinuxのコマンドをどんどん覚えていきます。
初回は実行に10-20分かかる場合があります。

=== プログラムの用意と撮影までの流れ

==== 必要なツールのインストール

「v4l-utils」はカメラの認識確認に使うツールです。カメラの操作に必要なので次のコマンドでインストールします。@<fn>{v4l_utils}

//cmd{
sudo apt install -y v4l-utils
//}

//footnote[v4l_utils][OSがWindowsやMacだと、何かしらのソフトはインストーラーをダウンロードして実行して、開かれたウィンドウをOKしていって...といった手順でインストールをしますが、linuxの派生OSであるRaspberry Pi OSでは、コマンドを打つことで必要なツールをインストールしていきます。]

==== カメラの接続と確認

カメラの設定をしていきます。本書ではLogicool C270を使う想定で説明をしていきます。

 1. WebカメラをRaspberry PiのUSBポートに接続
 2. ターミナルで認識確認

//cmd{
# USB機器を一覧表示するコマンド
lsusb
 * カメラデバイスの確認
v4l2-ctl --list-devices
//}

//image[lsusb_list][lsusbの実行結果の例][scale=0.75]

通常、最初に接続したUSBカメラが@<code>{/dev/video0}を含むデバイスとして認識されます。
複数のカメラを接続した場合はvideo1、video2と番号が増えます。再起動やUSBの差し直しでこの番号はリセットされます。

次にこのコマンドでカメラの対応解像度を確認してください。

//cmd{
# カメラの対応解像度とフォーマットの確認
v4l2-ctl -d /dev/video0 --list-formats-ext
//}

//image[v4l2-ctl_list-devices][v4l2-ctl --list-devicesの実行結果の例][scale=0.5]

このコマンドの実行結果から、カメラの対応解像度とフォーマットを確認できます。

==== デスクトップに作業フォルダを作成

デスクトップ上にcamera-projectという名前でフォルダを作り、そこでプログラムに関する作業をします。
コマンドでやってみたい方は、次のコマンドターミナル上で実行してください。

//cmd{
cd ~/Desktop
mkdir camera-project
cd camera-project
//}

//image[mkdir_camera-project][デスクトップに作業フォルダを作成][scale=0.5]

==== GitHubからプログラムを取得

GitHubから著者の作成したプログラムをダウンロードします。

@<href>{https://github.com/Mutsumix/google-photo-uploader}

次のコマンドを実行してください。

//cmd{
# 今からターミナルを開く場合はcdコマンドで、実行場所を先ほど作成したフォルダに移動します
cd ~/Desktop/camera-project
git clone https://github.com/Mutsumix/google-photo-uploader.git
cd google-photo-uploader
//}

//image[git_clone][git cloneの実行結果][scale=0.5]

git cloneはGitHub上のプログラムをダウンロードするコマンドです。実行後、camera-projectフォルダの中にgoogle-photo-uploaderフォルダが作成されます。

==== 必要なPythonパッケージのインストール

//cmd{
pip install -r requirements.txt
//}

requirements.txtには、プログラムの動作に必要な部品（ライブラリ）のリストが記載されています。一つひとつインストールすることもできますが、このコマンドで一括インストールされます。
インストールには数分から数時間かかる場合があります。気長にお待ちください。

==== 設定ファイルの準備

設定ファイルを作成します。

//cmd{
cp config.sample.yaml config.yaml
//}

ファイルマネージャーまたはテキストエディタでconfig.yamlを開いて編集

//cmd{
nano config.yaml
//}

以下のように設定します。

//emlist[config.yaml]{
camera:
  use: true
  photo_dir: "photos"
  settings:
    camera_model: null  # "EMEET"でEMEET専用設定、nullでデフォルト（C270等）
    width: 1920  # C270最大:1280x960、EMEET最大:1920x1080
    height: 1080
    fourcc: "MJPG"  # MJPG:圧縮形式（推奨）、YUYV:非圧縮
    fps: 30
    focus: 500  # EMEETのみ有効。範囲0-1023: 0=最近距離、400-600=室内3-5m、1023=無限遠
  scheduler:
    day_of_week: ["monday", "tuesday", "wednesday", "thursday", "friday"]
    at_time: ["06:00:00", "12:00:00", "18:00:00"]
//}

 * album_titleは任意の名前に変更可能です。Google Photosに自動作成されるアルバム名になります
 * day_of_weekで撮影曜日を指定できます。
 * at_timeで撮影時刻を設定できます（24時間表記）
 * `width`と`height`はお使いのカメラの対応解像度に合わせてください（C270の場合は1280x960、EMEET S600の場合は3840x2160）
 * notificationsは通知設定です。今回は使用しませんので、falseにしてください。

編集後、@<code>{Ctrl + O}→@<code>{Enter}（保存）→@<code>{Ctrl + X}（終了）の順で操作をしてファイルを更新します。

==== テスト撮影の実行

次のコマンドを実行して、カメラが正常に動作するか確認します。

//cmd{
cd ~/Desktop/camera-project/google-photo-uploader
python camera_module.py
//}

このコマンドが成功すれば、カメラで撮影した画像が`photos/`フォルダに保存されます。


==== 撮影結果の確認

ファイルマネージャーで@<code>{~/Desktop/camera-project/google-photo-uploader/photos/}を開き、撮影された画像を確認してください。

画像が正常に保存されていれば、カメラの設定は完了です。

//image[camera_module_py][camera_module.pyの実行結果][scale=0.75]

==== 撮影のトラブルシューティング

@<b>{EMEETカメラで画像がぼやけている場合：}

config.yamlのfocus値を調整してください：

//emlist[config.yaml]{
camera:
  settings:
    camera_model: "EMEET"  # EMEET使用時は必須
    focus: 500  # 0-1023の範囲で調整: 0=最近距離、400-600=室内3-5m、1023=無限遠
//}

調整後、再度@<code>{python camera_module.py}で撮影して確認してください。
50-100ずつ変更して最適な値を見つけてください。

@<b>{画像が暗い場合：}

camera_module.pyのBRIGHTNESS値を直接変更する必要があります：

//emlist[camera_module.py]{
cap.set(cv2.CAP_PROP_BRIGHTNESS, 128)  # デフォルトは128、暗い場合は150-200に増やす
//}

@<b>{C270でピントが合わない場合：}

C270は固定フォーカスのため、ソフトウェアから調整できません。
カメラを分解してレンズのフォーカスリングを物理的に調整する必要があります（自己責任で行なってください）。

== クラウド側（Google Cloud Platform）の設定

ここからはクラウド（GCP）側の設定をしていきます。
手順が多いですが、順番に沿って進めれば問題ありません。

=== GCPプロジェクトの作成

 1. 操作用PCのブラウザでGoogle Cloud Console @<href>{https://console.cloud.google.com/, https://console.cloud.google.com} にアクセス

//image[gcp_project_create][GCPプロジェクトの作成画面][scale=0.5]

 2. Googleアカウントでログイン（初回は利用規約に同意）

//image[gcp_project_create_2][GCPプロジェクトの作成画面2][scale=0.5]
//image[gcp_project_create_3][GCPプロジェクトの作成画面3][scale=0.5]

 3. 画面上部の「プロジェクトを選択」（または「プロジェクト名」）をクリック
 1. 右上の「新しいプロジェクト」をクリック

//image[gcp_project_create_4][GCPプロジェクトの作成画面4][scale=0.5]

 1.  プロジェクト名を入力（例: raspi-camera-project）

//image[gcp_project_create_5][GCPプロジェクトの作成画面5][scale=0.5]

 1. 「作成」をクリック（作成に30秒ほどかかります）
 1. 通知領域の「プロジェクトを選択」から作成したプロジェクトを選択

GCPアカウントがGoogleから割り当てられた「家」だとすると、GCPプロジェクトは、Googleのクラウドサービスを利用するための「部屋」のようなものです。一つのプロジェクトで複数のAPI（例えるなら「家具」）を管理できます。

GCPでは2025年5月13日から多要素認証（MFA）とも呼ばれる 2 段階認証プロセス（2SV）の適用を開始しました。Googleアカウントのセキュリティ設定に移動して、2 段階認証プロセスを有効にしてください。

=== Photos Library APIの有効化

ラズパイで撮影した画像を自動的にGoogle PhotoにアップロードするためにAPIという機能を使用します。API@<fn>{api}を使用するには有効化の設定が必要なので、行なっていきましょう。

//footnote[api][APIについては@<chapref>{120_dashboard}で解説しています]

 1. 左上のハンバーガーメニュー（三本線）→「APIとサービス」→「ライブラリ」

//image[gcp_project_create_6][APIとサービスの設定メニュー][scale=0.5]

 2. 検索窓に「Photos Library API」と入力
 3. 「Photos Library API」を選択
 4. 「有効にする」をクリック

//image[gcp_project_create_7][APIとサービスの有効化][scale=0.5]

=== OAuth同意画面の設定

OAuthの設定をしていきます。@<fn>{oauth}

//footnote[oauth][OAuth（オーオース）は、アプリがあなたの代わりにGoogle Photosにアクセスするための安全な認証の仕組みです。ここでは、ユーザーに表示される許可画面（「このアプリにGoogle Photosへのアクセスを許可しますか？」など）の設定を行います。]

 * 左メニュー「APIとサービス」→「OAuth同意画面」→ 開始ボタンをクリック


 * アプリ情報入力：
 ** アプリ名: 任意（例: Raspi Camera Uploader）
 ** ユーザーサポートメール: 自分のメールアドレスを選択
 ** 対象：外部
 ** 連絡先情報: 自分のメールアドレスを入力
 * 「保存して次へ」
//image[gcp_project_create_8][0Auth同意画面の設定画面][scale=0.5]
//image[gcp_project_create_9][0Auth同意画面の設定画面2][scale=0.5] 

==== スコープの追加

スコープは「このアプリがGoogle Photosのどの機能を使うか」を指定するものです。
アルバムに画像を保存する、保存された画像を読み取る、不要な画像は削除するなど、プログラムの機能を増やそうとすればするほど、スコープは増えます。
今回はアルバムに画像が保存できればそれで良いので、次のような設定をします。

 * 「スコープを追加または削除」をクリック

//image[gcp_project_create_10][スコープの追加画面][scale=0.5]

 * フィルタに「photos」と入力
 * 以下の2つにチェック：
 ** https://www.googleapis.com/auth/photoslibrary
 ** https://www.googleapis.com/auth/photoslibrary.appendonly

//image[gcp_project_create_11][追加するスコープ][scale=0.5]
  * 「更新」→「Save」
//image[gcp_project_create_12][スコープの保存][scale=0.5]

==== テストユーザーの追加

テストユーザーに登録したアカウントだけがこのアプリを使用できます。自分だけを追加すれば、他の人は使用できません。
今回は自分だけが実行できればいいので、自分だけを追加します。

 * 「ADD USERS」をクリック
//image[gcp_project_create_13][テストユーザーの追加画面][scale=0.5]
 * 自分のGoogleアカウントのメールアドレスを入力
//image[gcp_project_create_14][テストユーザーの追加画面2][scale=0.5]
 * 「保存」
 * ダッシュボードに戻る

==== OAuth認証情報の作成

ここでは、アプリがGoogleと通信するための「鍵」を作成します。先ほど設定した「OAuth同意画面」はユーザーに見せる画面の設定で、こちらは実際の通信に使う認証情報（client_secrets.json）を作成します。

 * 左メニュー「認証情報」
//image[gcp_project_create_15][認証情報の作成メニュー][scale=0.5]
 * 上部の「＋認証情報を作成」→「OAuthクライアントID」
//image[gcp_project_create_16][OAuthクライアントIDの作成画面][scale=0.5]
 * アプリケーションの種類: 「デスクトップアプリ」を選択
 * 名前: 任意（例: Raspi Camera Client）
//image[gcp_project_create_17][OAuthクライアントIDの作成画面2][scale=0.5]
 * 「作成」を選択

==== 認証情報のダウンロード

 * 「OAuth 2.0 クライアントID」の一覧に作成した認証情報が表示される
 * 右端の「ダウンロード」アイコン（↓矢印）をクリック
//image[gcp_project_create_18][認証情報のダウンロード画面][scale=0.5]
 * JSONファイルがダウンロードされる（ファイル名はclient_secret_xxxxx.jsonのような形式）

== Raspberry PiからGCPへの認証と撮影画像のアップロード

=== ファイルの配置

ダウンロードしたファイルをラズパイ上に配置しましょう

VNC Viewerを使った方法で解説します。

 1. VNC ViewerでRaspberry Piに接続
 2. 操作用PCでダウンロードしたJSONファイルの内容をテキストエディタで開く
 3. 全選択してコピー
 4. Raspberry Pi側でターミナルを開き、次のコマンドを実行@<fn>{nano}

//footnote[nano][nanoコマンドではなく、右クリックからclient_secrets.jsonファイルを新規作成し、貼り付ける方法でも構いません。]

//cmd{
cd ~/Desktop/camera-project/google-photo-uploader
nano client_secrets.json
//}

//image[client_secrets_json][client_secrets.jsonの内容][scale=0.75]

 1. 右クリックで貼り付け
 2. @<code>{Ctrl + O}→@<code>{Enter}（保存）→@<code>{Ctrl + X}（終了）

//image[nano_client_secrets_json][nanoコマンドでclient_secrets.jsonを作成し貼り付け][scale=0.5]

慣れている方はSSHを使用してください。
うまくできなかった方はUSB経由で`client_secrets.json`というファイル名にして、ラズパイのデスクトップ上の`google-photo-uploader`に保存しましょう。

== 初回認証とテスト実行

=== 認証セットアップの実行

ターミナルで次のコマンドを実行しましょう

//cmd{
cd ~/Desktop/camera-project/google-photo-uploader
python setup_auth.py --test-camera --test-upload
//}

=== ブラウザでの認証

ターミナルに認証用のURLが表示されます：


//image[setup_auth_py][setup_auth.pyの実行結果][scale=0.75]

//cmd{
Please visit this URL to authorize this application: https://accounts.google...
//}

 * このURLをコピー（VNC Viewerならマウスでドラッグして右クリック→URLをコピー→ブラウザに貼り付け、もしくはOpen URL）

//image[open_url][API認証用のURLが表示される][scale=0.5]

 * ブラウザで開く（Raspberry Pi上またはVNC接続元のPC）
 * Googleアカウントでログイン
//image[login_google_account][Googleアカウントでログイン][scale=0.5]
 * 「このアプリはGoogleで確認されていません」という警告が表示されます@<fn>{oauth_warning}
//image[oauth_warning][OAuth警告画面][scale=0.5]

//footnote[oauth_warning][なぜこの警告が出るのかというと、Googleの審査を受けていない個人開発アプリだからです。審査は申請に時間がかかります。商用アプリでなければ審査をせずにテストモードとして使用し続けることも可能です。]

 * 一瞬ギョッとしてしまいますが、落ち着いて「続行」をクリック
 * 権限の許可画面で「すべて選択」→「続行」を選択

//image[select_all_permissions][すべて選択して続行][scale=0.5]

 * 「The authentication flow has completed. You may close this window.」というあり得ないくらい素っ気ない画面が表示されれば成功です。
//image[authentication_flow_completed][認証成功画面][scale=0.5]

 * ターミナル上でプログラムが続行されていることを確認してください。

//image[terminal_running][プログラムが続行され完了した画面][scale=0.75]

認証が成功すると`photo_token.json`という名前でトークンが保存され、次回以降は自動的に認証されます。@<fn>{photo_token_json}

//image[photo_token_json][photo_token.jsonの内容][scale=0.5]

//footnote[photo_token_json][ただし、テストモードだと、1週間で認証が切れてしまいます。そのため、photo_token.jsonを削除し、再度認証の作業をしなくてはなりませんので注意しましょう。]

=== 動作確認

認証が成功すると、自動的に次のことが起こります

 * 撮影した画像がGoogle Photosへアップロードされる
 * `raspi-camera`アルバムの作成（初回のみ）

ターミナルに以下のようなメッセージが表示されれば成功：

//cmd{
カメラテスト撮影を実行します...
2025-11-05 01:51:48,883 - camera_module - INFO - camera module is starting...
2025-11-05 01:51:49,277 - camera_module - INFO - Applying EMEET-specific settings
2025-11-05 01:51:50,030 - camera_module - INFO - Settings: MJPG, 3840x2160, 30fps
2025-11-05 01:51:52,784 - camera_module - INFO - Photo saved: photos/setup_test.jpg
OK: カメラテスト撮影完了: photos/setup_test.jpg
テスト画像をGoogle Photosにアップロードします...
2025-11-05 01:51:59,103 - google_photos - INFO - Succeeded upload of image to photo library. status: {'message': 'Success'}
OK: テスト画像アップロード完了
INFO: テスト画像を削除しました

セットアップ完了
   これで 'python main.py' を実行できます。

次のステップ:
   1. 設定確認: vi config.yaml
   2. メイン実行: python main.py
   3. ログ確認: tail -f logs/main.log
//}

ブラウザでGoogleフォト（@<href>{https://photos.google.com/}）を開いて、raspi-cameraアルバムに写真がアップロードされているか確認してください。

//image[google_photos_album][Googleフォトのアルバム画面][scale=0.5]

== 定期実行の設定

撮影とアップロードのスケジュールはconfig.ymlを編集することで変更ができます。

//emlist[config.yml]{
scheduler:
  day_of_week: ["monday", "tuesday", "wednesday", "thursday", "friday"]  # 月〜金の場合
  at_time: ["06:00:00", "12:00:00", "18:00:00"]        # 毎日6時、12時、18時に撮影
//}

 * at_timeの指定方法：24時間表記で指定します。朝6時を指定する場合は、"6:00:00"ではなく"06:00:00"と指定しないとエラーになりますのでご注意ください。

=== プログラムの起動

ここまでの作業が完了したら、本番のプログラムを実行しましょう。

プログラムが起動し、設定した時刻になると自動的に撮影・アップロードが実行されます。ターミナルを閉じるとプログラムも終了します。

//cmd{
cd ~/Desktop/camera-project/google-photo-uploader
python main.py
//}

=== 自動起動について

再起動後も自動的にプログラムを起動したい場合は、systemdサービスとして登録する方法があります。本書では紹介しませんが、詳しくは「raspberry pi systemd 自動起動」などで検索してください。

== トラブルシューティング

想定されるエラーとその対策を記します。

@<b>{カメラが認識されない}

//cmd{
# USB機器を再確認
lsusb
# カメラを別のUSBポートに差し替えてみる
# 特にUSB3.0ポート（青いポート）を試す

# OpenCVでの動作確認
python -c "import cv2; print(cv2.VideoCapture(0).isOpened())"
# Trueが返ってくればOK
//}

@<b>{640x480の低解像度でしか撮影できない}

原因：カメラがMJPG形式に対応していない、または設定ミス

//cmd{
# カメラの対応フォーマットを確認
v4l2-ctl -d /dev/video0 --list-formats-ext | grep MJPG
//}

お使いのカメラの対応解像度を確認し、`config.yaml`で設定してください：

 * Logicool C270: 最大1280x720（または1280x960）
 * EMEET S600: 最大3840x2160

//emlist[config.yml]{
camera:
  settings:
    width: 1280
    height: 720
    fourcc: "MJPG"  # または "YUYV"
//}

@<b>{Google Photos認証エラー}

「Token has been expired or revoked」などのエラーが出る場合：

//cmd{
cd ~/Desktop/camera-project/google-photo-uploader
rm photo_token.json
# 手動でphoto_token.jsonしても構いません
# 再認証
python google_photos.py
//}

ブラウザで再度認証フローを実行してphoto_token.jsonを再作成してください。
その後、@<code>{python main.py} を実行してください。

== Googleフォトでタイムラプス動画を作る

ここまでの手順で、Raspberry Piが自動的に撮影・アップロードした写真は、Googleフォト上のアルバム（例：「raspi-camera」）に定期的に保存されています。
最後に、これらの画像を使ってタイムラプス動画を作成してみましょう。

 * Googleフォト（@<href>{https://photos.google.com/}）にアクセスします。
 * 自動アップロードされた画像が保存されているアルバムを開きます。
 * タイムラプスに使いたい写真を複数選択します。
  ** 撮影日時が連続しているものを選ぶと自然な動画になります
 * 右上の「＋作成」ボタンをクリックします。
 * 表示されるメニューから「アニメーション」を選択します。

//image[google_photos_album_create_animation_menu][写真を選択し、アニメーションを作成][scale=0.5]
//image[google_photos_album_create_animation_menu_animation][アニメーション作成中][scale=0.5]

数秒ほど待つと、選択した画像をもとにアニメーション動画が自動生成されます。@<fn>{google_photos_album_create_animation_menu_animation}
この動画はGoogleフォト上で再生・共有できるほか、必要に応じてダウンロードして編集ソフトで再加工することも可能です。


連日撮影した植物の成長をこうして動画にまとめると、成長の経緯が一目でわかります。@<fn>{google_photos_album_create_animation_menu_animation}
純粋に見ていて楽しいですし、植物のコンテナ毎に生育条件を分けていれば、「葉が大きく成長するタイミング」や「日照時間や温度・湿度による葉の動きの違い」など連続的な成長の様子から多くの気づきが得られるのではないでしょうか。

ラズベリーパイとGoogleフォトの連携により、手間をかけずにタイムラプス動画を作成することができました。

//footnote[google_photos_album_create_animation_menu_animation][書籍では動画をお見せできないのが残念です]

== 参考資料・ソース

この記事で紹介した、ラズベリーパイに市販のWebカメラを接続してGoogle フォトにアップロードするアイディアは、Udemyで公開されている「ラズベリーパイ（Raspberry Pi）と3Dプリンターで作って学ぶIoT～DIY型IoTキット作成講座～」(@<href>{https://t.co/hYOPOvTUa9}）という講座を大いに参考にさせていただきました。
講師である佐々木 健介(@kensukesasawood)さんは、他にも農業とITを掛け合わせたユニークな講座を公開していてとても学びが多いです。本章の内容が面白いと感じた方がいましたら、ぜひ本記事のネタ元である講座やその他の講座を視聴することをお勧めいたします。

//image[udemy-sasaki][Udemy講座はこちらから][scale=0.5]