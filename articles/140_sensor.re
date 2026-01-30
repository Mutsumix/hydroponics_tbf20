= センサーを使ってモニタリングしよう

ここまで、SwitchBotやスマートフォンを使って環境データを集め、ダッシュボードでの可視化、定点観測カメラで記録する仕組みを作ってきました。
クラウドを中心とした観測のシステムはひと通り構築してきたことになります。

この章の目的は、データの源そのものを自分で扱うことです。
「データを使う」から「データを生み出す」段階に進みます。

ソフトウェアの世界では、監視（モニタリング）は「ツールの運用」ではなく、「どの情報を、どの精度で観測するかを設計すること」だとよく言われます。@<fn>{monitoring}
//footnote[monitoring][Mike Julian 著『入門 監視』でも、この観点が強調されています。]

それはハードウェアを扱う場合でも同じです。
観測の設計をサーバーの外である現実世界に拡張すると、データの意味や精度、そしてそのデータの変化のきっかけまで意識するようになります。
それはより深いレベルで、生育する植物を理解することにつながります。

この章では、Arduino（アルデュイーノ）互換機である Seeeduino Lotus（シーデュイーノ・ロータス） を使い、水位・水温・照度・騒音といった環境センサーを自分で接続し、実際にデータを取得していきます。
これまで扱ってきた監視の仕組みの基盤となる部分、つまり「データが生まれる層」を、自分の手で組み上げていきましょう。

全体像を把握しておきましょう。

//image[sensor_system_overview.drawio][センサーデータ取得システムの全体像][scale=0.75]

Raspberry Piへの送信とGrafanaへの反映は、@<chapref>{145_finishing} で説明します。

まずはこの章で扱う機器についての理解を深めることにしましょう。

== あなたにArduinoを愛していないとは言わせない
Raspberry PiとArduino（およびその互換機であるSeeeduino）は、どちらも小型のコンピュータです。
見た目は似ていますが、内部の仕組みと得意分野はまったく異なります。
この違いを理解しておくと、「なぜArduino系がセンサーに向いているのか」「なぜラズベリーパイが手元にあるのにそれで全部済ませないのか」が理解できます。

//image[arduino_uno][Arduinoシリーズ、Unoの外観][scale=0.5]

=== Arduino系は電圧と信号の扱いがシンプル

Arduino系ボードは、センサーを直接つなぐことを前提に設計されています。
多くのセンサーは 3.3V または 5V で動作しますが、Seeeduino Lotus Cortex M0+ は 3.3V 動作のマイコンです。
Grove システム対応のセンサーであれば、ケーブルを差し込むだけで安全に接続できるように設計されています。

アナログ出力のセンサーは 0〜3.3V の範囲で信号を出します。
Arduino 側ではこの電圧をそのまま扱うことができ、@<code>{analogRead()} で数値化できます。
5Vセンサーをそのまま接続できるわけではありませんが、3.3Vセンサーであれば追加の回路は不要です。

一方で、Raspberry Pi の GPIO ピンも 3.3V 専用ですが、アナログ入力機能を持っていません。
そのため、水位センサーや温度センサーのようにアナログ信号を扱うには、別途A/D変換ICを接続する必要があります。
信号の種類や電圧を意識せず扱えるという点で、Arduino系の方がセンサー接続には適しています。

今回扱う、水位・水温・照度・騒音といったセンサーはすべて Grove 規格に対応しており、
Seeeduino Lotus Cortex M0+ であれば、追加部品なしで最短の配線で動作します。

=== Arduino系は構造が単純

Arduino（Seeeduino）とRaspberry Piの構造の最も大きな違いは、OSの有無です。
Arduino系ボードは、マイクロコントローラ（MCU：Micro Controller Unit） が直接動作する構造になっています。
簡単にいうと、電源を入れると、ただひとつのプログラムが即座に動き出し、一定間隔でセンサーを読み取るといった処理を確実に実行します。
逆にいうと、それ以外はできません。Raspberry Piのことを知っている人にとっては、あまりの単機能っぷりに驚くかもしれません。

「電源を入れてただ一つのプログラムを動かす」

同じことをRaspberry Piでやる場合、OSがあるため、OSのセットアップやプログラムの書き込み、自動実行の設定、必要なソフトのインストールなど、いろいろな準備が必要になります。
Arduino系は電源を入れるだけですぐに書き込んだプログラムが動きますが、ラズベリーパイはパソコンに近いので、手間がかかります。

どちらが優れているという話ではなく、センサーを扱うにはArduino系が向いているということです。
そのため、Raspberry Piはデータを集めて解析したり、クラウドに送信したりする側、Arduinoは現場でセンサーを扱う側として使い分けるのが良いでしょう。

=== Grove規格で配線トラブルを減らせる


センサーを複数扱うと、電源（VCC・GND）や信号線の取り回しで配線が複雑になりがちです。
マイコンを使った指南書や解説ブログには必ず、「ブレッドボード」、「ジャンパーワイヤ」というものが出てきます。

//image[board_and_wire][ブレッドボードとジャンパーワイヤ][scale=0.75]

ジャンパーワイヤを使ったブレッドボード配線は学習には良いのですが、接触不良や挿し間違いが起きやすく、初心者にとってはトラブルの原因になりやすい部分です。
また、ものによっては半田付けの必要があったりします。初心者がはんだ付けや配線の間違いで詰まってしまった場合、周りに電子工作に詳しい人がいないと、解決ができず学習が止まってしまい、徒労感と挫折感だけが残ってしまうことがあります。
本書ではそういったことが起きないように、できるだけシンプルな構成でセンサーデータを取得できるような機器、Seeeduino Lotus を選定しました。@<fn>{soldering}

//image[seeeduino_lotus][Seeeduino Lotus][scale=0.5]

Seeeduinoシリーズでは、配線問題を解決するためにGroveコネクタが採用されています。
Groveは4ピンの専用ケーブルで、センサーを差し込むだけで接続できる仕組みです。
電源と信号線の配線を自動で正しくまとめてくれるため、間違いを最小限に抑えながら安全に実験できます。

//image[grove_cable][Groveケーブル][scale=0.5]

//footnote[soldering][もっと正直に言うと、文系ソフトウェアエンジニアの自分にとって回路図や配線図がマニュアルに出てきた瞬間「自分とは関わりのない世界だ！」と脳が拒絶反応を示して激萎えしてしまいます。そんな自分でもセンサーでデータを取得したい！と言う願望を叶えてくれるのがSeeeduinoとSeeed社のセンサーたちだったんです。]

=== 互換機 Seeeduinoとは

Arduino互換機のSeeeduinoについて説明します。

まず、Arduinoは、イタリアのArduino社が開発したオープンソースのマイコンボードです。「オープンソース」であるため、設計図が公開されており、誰でも互換機を作ることができます。
Seeeduinoは、中国のSeeed Studio社が製造するArduino互換機です。Arduino公式ボードとピン配置や動作は完全に同じなので、Arduino用のスケッチ@<fn>{arduino_sketch}やライブラリがそのまま使えます。

//footnote[arduino_sketch][Arduinoのプラグラムのこと、拡張子は.inoで言語はC++がベース]

@<b>{なぜArduino互換機を選ぶのか}

Arduino公式のUno R3よりもSeeeduinoは安価に購入できます。
互換機を使っても機能的なデメリットはほぼありません。むしろ、Seeeduinoには後述するGroveコネクタという独自の利点があります。

@<b>{Groveコネクタという福音}

前述の通り、Arduino系ボードにセンサーを接続する場合、ブレッドボードとジャンパーワイヤーを使った配線作業が必要です。

@<code>{センサー → ジャンパーワイヤー数本 → ブレッドボード → Arduino}

この方法だと前述の通り、初心者にとって、配線ミスが多発しやすく、接触不良も起きやすくなります。

@<b>{Groveの仕組み}

Groveは、Seeed Studio社が開発した統一規格のコネクタシステムです。

@<code>{センサー → Groveケーブル1本 → Arduino}

この形状であれば間違った挿し方はできません（挿しの甘さには注意）。
さらに見た目がすっきりして取り回しがしやすいので、栽培用の容器やプランターの近くに置いたとしても違和感が少なくなります。

また、@<href>{https://jp.seeedstudio.com/, Seeed Studio社のECサイト}では、Groveコネクタ対応のセンサーモジュールを数多く販売しています。
たとえば次のような豊富なセンサーの中から、栽培に必要なものを選ぶことができます。

 * 温湿度センサー
 * 土壌水分センサー
 * 照度センサー
 * CO2センサー
 * 気圧センサー
 * 水位センサー
 * etc...

すべて同じGroveケーブルで繋げるため、接続方式の違いを意識する必要がありません。
これはマイコンの配線に悩まされた人間にとって、朗報です。

Seeeduino Lotus はこのGroveコネクタを12個搭載しており、簡易さと拡張性の両立を実現しています。
前置きがとても長くなりましたが、ここまでが、Seeeduino Lotusをセンサーの取得に活用する理由です。

次に購入品を紹介します。
ほとんどの部品はSeeed StudioのECサイト（@<href>{https://jp.seeedstudio.com/, https://jp.seeedstudio.com/}）で購入できます。

== 購入品一式

//table[sensor_list][購入品一式]{
品目	個数	購入先	目安の価格
------------------
Seeeduino Lotus Cortex M0+@<fn>{seeeduino_lotus}	1つ	Seeed Studio	¥1,509
ワイヤー温度センサー@<fn>{temperature_sensor}	1つ	Seeed Studio	¥1,266
照度センサー@<fn>{light_sensor}	1つ	Seeed Studio	¥991
騒音センサー@<fn>{volume_sensor}	1つ	Seeed Studio	¥824
水位センサー@<fn>{water_level_sensor}	1つ	Seeed Studio	¥1,159
LCD液晶ディスプレイ（オプション）@<fn>{lcd_display}	1つ	Seeed Studio	¥1,080
USBケーブル（USB2.0 Aオス-マイクロBオス）	1本	Amazon	¥500
//}

Seeeduino LotusはSeeeduino Lotus V1.1とSeeeduino Lotus Cortex M0+ の2種類が販売されていますが、 Cortex M0+ が本書で使用するボードです。
公式のWikiに正確なスペック@<fn>{seeeduino_lotus_wiki}は記載されていますが、 Cortex M0+ は V1.1 よりも性能が向上しており、本書で使用するボードはこちらになります。

//image[seeeduino_lotus_two_types][2種類のSeeduino Lotus][scale=0.75]

また価格はすべて2025年11月1日時点の価格です。為替の影響を大きく受けるので、あくまで目安としてご覧ください。
もし、海外のECサイトで購入することに抵抗がある方は、国内のショップであるスイッチサイエンス、マルツオンライン、秋月電子、またはAmazonで一部製品は購入することが可能です。

部品によっては公式サイトで入荷待ちとなっている場合もあるので、別のショップも組み合わせて購入することをおすすめします。
また、これらの部品は@<b>{すべて揃える必要はありません}。
あとで紹介するセンサー取得のプログラムは、接続されていないセンサーのデータは無視してくれますので、必要なものだけを購入することもおススメします。

//footnote[seeeduino_lotus][@<href>{https://jp.seeedstudio.com/Seeeduino-Lotus-Cortex-M0-p-2896.html} Seeeduino Lotus]
//footnote[temperature_sensor][@<href>{https://jp.seeedstudio.com/One-Wire-Temperature-Sensor-p-1235.html} ワイヤー温度センサー]
//footnote[light_sensor][@<href>{https://jp.seeedstudio.com/Grove-Digital-Light-Sensor-TSL2561.html} 照度センサー]
//footnote[volume_sensor][@<href>{https://jp.seeedstudio.com/Grove-Sound-Sensor-Based-on-LM358-amplifier-Arduino-Compatible.html} 騒音センサー]
//footnote[water_level_sensor][@<href>{https://jp.seeedstudio.com/Grove-Water-Level-Sensor-10CM-p-4443.html} 水位センサー]
//footnote[lcd_display][@<href>{https://jp.seeedstudio.com/Grove-16-x-2-LCD-Black-on-Red.html} LCD液晶ディスプレイ]
//footnote[seeeduino_lotus_wiki][@<href>{https://wiki.seeedstudio.com/ja/Seeeduino_Lotus_Cortex-M0-/} Seeeduino Lotus Cortex-M0+ 公式Wiki]

== センサーの接続方法

センサーの接続方法は解説がいるのか？と疑問に浮かぶくらい非常に簡単です。
ただし、Groveコネクタはセンサーの種類によって、差し込むコネクタの位置が異なります。

 * デジタルコネクタ × 7個 (D2〜D8)
 * アナログコネクタ × 3個 (A0〜A2)
 * I2C（アイ・スクエアド・シー）コネクタ × 2個

そのため、今回は次の図を参考に配線を行ってください。

//image[sensor_connection][センサー接続図][scale=0.75]

 * 温度センサー     D3
 * 騒音センサー     A2
 * 水位センサー     I2C
 * 照度センサー     I2C
 * (LCDディスプレイ  I2C)

== センサーデータ取得のプログラムのダウンロードと実行方法

それでは、センサーデータを取得するプログラムを作成していきます。
プログラムのボードへの反映には、専用のエディタであるArduino IDEを使用します。

手順は次のとおりです。一つ一つ順を追って見ていきましょう。

 1. Arduino IDEの設定をする
 1. GitHubからセンサーデータを取得するプログラムのソースコードを取得する
 1. プログラムをSeeeduino Lotusにアップロードする
 1. 出力結果を確認する

=== Arduino IDEをインストールする

Seeeduino Lotusにプログラムを書き込むためのツールとして、Arduino IDEを使用します。

Arduino公式サイト(@<href>{https://www.arduino.cc/en/software, https://www.arduino.cc/en/software})からIDEをダウンロードしてインストールします。

Windows、macOSいずれでも構いません。OSに応じたインストーラをダウンロードして、特別な理由がなければデフォルトの設定でインストールします。
起動して、初期画面（空のスケッチ画面）が表示されれば成功です。

//image[arduino_ide_initial_screen][Arduino IDEの初期画面][scale=0.75]

=== Seeeduino用ボードパッケージを追加する

Seeeduino Lotus Cortex M0+ は Arduino 互換機ですが、Arduino Uno とは製造元・チップ構成が異なります。
そのため、Arduino IDE に Seeed 製ボードパッケージを追加する必要があります。

 * Windows: メニューから [ファイル] → [環境設定] を開く
 * macOS: メニューから [Arduino IDE] → [Preferences] を開く
 * 「追加のボードマネージャのURL」欄に次を入力

    https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json@<fn>{seeeduino_board_manager_url}
 * ［OK］を押して閉じる

//footnote[seeeduino_board_manager_url][本書のサポートページ（@<href>{https://mutsumikajihara.com/data-garden/}）にも記載していますので、そちらからコピペすることも可能です]

//image[additional_board_manager_url][追加のボードマネージャの設定][scale=0.75]

準備が整ったので、Seeeduino Lotus Cortex M0+ 用のボードパッケージをインストールします。

  * [ツール] → [ボード] → [ボードマネージャ…] を開く
  * 検索欄に「seeed samd」と入力
  * 「Seeed SAMD Boards」が候補に出てきたら「Install」をクリック

//image[board_manager][ボードマネージャ][scale=0.5]

これで Seeeduino Cortex M0+ 系のボードパッケージが Arduino IDE に追加されました。

=== Seeeduino Lotus Cortex M0+ をPCに接続する

Seeeduino Lotus Cortex M0+ を Micro USB ケーブルで PC に接続します。
ケーブルは@<b>{「データ通信対応」}のものを必ず使用してください。
100円ショップでよく売られている「充電専用」と書かれたケーブルではスケッチの書き込みができません。

接続が完了すると、Arduino IDE のポートリストに新しいポートが表示されます。
Windows では「COMx」（x は数字）、macOSやLinuxでは「/dev/cu.usbmodemXXXX」といった形式になります。

//image[port_list][ポートリスト][scale=0.5]

ポートが分からなくなった場合は、一度 USB を抜き差しして、新たに現れたポートを選びます。

=== ボードとポートを設定する

接続したSeeeduinoとボードの種類を設定します。
「ボ」ードと「ポ」ート、紛らわしいですが別物です。

ここでいう@<b>{ボード}とは、Seeeduino基盤のことです。

@<b>{ポート}とは、PCとボードを接続するための通信線のことです。

 * @<code>{Select other board and port}を選択

//image[select_other_board_and_port][Select other board and port メニュー][scale=0.5]

 * 右側のPORTSからSeeeduino が接続されたポートを選択して「OK」をクリック

//image[select_port][Select port メニュー][scale=0.5]

 * 左側のBOARDSからSeeeduino Zero を選択して「OK」をクリック

 * また、ボーレート@<fn>{baud_rate}は 115200bps に設定しておきます

画面下部に、「Serial Monitor（シリアルモニタ）」が表示されていますか？表示されていない場合は、「Tools」メニューから「Serial Monitor」を選択してください。

//image[boud_rate_setting][ボーレートの設定][scale=0.75]

//footnote[baud_rate][ボーレートは、通信速度のことで、1秒間に何回信号状態が変化するかを表します。115200bpsは、1秒間に115200ビットのデータを送信する速度です。ここの値が異なっていると、文字化けが発生します。]

これでボードとポートの設定が完了しました。

=== 接続確認（Lチカ）

動作確認のため、用意されているLチカ（LEDの点滅）のサンプルプログラムを書き込んでみます。

 * Lチカのサンプルプログラムを選択
 ** Windows: [ファイル] → [スケッチ例] → [01.Basics] → [Blink] 
 ** macOS: [File] → [Examples] → [01.Basics] → [Blink] 

//image[select_blink_program][Lチカのサンプルプログラムの選択][scale=0.75]

ここで、Arduino IDEでのプログラムの実行の方法について確認しておきましょう。

左上のチェックマークアイコンは「コンパイル」ボタンです。プログラムに間違いがないかどうかを検証します。

同じく左上にある右矢印アイコンが、「アップロード」ボタンです。
コンパイル作業を行ったのちに、プログラムをボードに書き込みます。

//image[upload_program][Arduino IDEにおけるプログラムの実行方法][scale=0.75]

それでは実際にアップロードボタンを選択して、プログラムを書き込んでみましょう。

基板上の LED が 1 秒間隔で点滅すれば成功です。

//image[blink_program_result][Lチカの結果][scale=0.75]
地味ですがまたたいています

=== センサーを接続する

ここから、Seeeduino Lotusに複数の環境センサーを接続してデータを取得します。
紹介したセンサーすべてを接続したと想定して、センサーと接続ポートを表にまとめます。

//table[sensor_port][センサー接続ポート（Cortex M0+）]{
センサー	接続ポート
--------------------------
温度センサー	D4
水位センサー	I2C
照度センサー	I2C
騒音センサー	A1
LCDディスプレイ（オプション）	I2C
//}

//image[sensor_connection_diagram][センサー接続図][scale=0.75]

=== コードの紹介とライブラリのダウンロード

ここからは、センサーデータを取得するプログラムのソースコードを作成します。

Arduino IDEで新規プロジェクトを作成します。
左側のフォルダアイコン→「NEW SKETCH」の順で選択します。

//image[new_sketch][新規プロジェクト作成][scale=0.75]

続いてGitHubからセンサーデータを取得するプログラムのソースコードを取得します。

著者のリポジトリにアクセスしてください。

@<href>{https://github.com/Mutsumix/Seeeduino.git} 

必要なソースコードは

@<code>{seeduino-sensor-monitor/seeduino-sensor-monitor.ino}

のみです。

これをコピーしてArduino IDEに貼り付けます。

このプログラムですが、各センサーが接続されていない場合でも、エラーを出さずに動作します。
ただし、必要なライブラリがダウンロードされていない場合は、エラーになります。

そのため、次にセンサー関連のライブラリをダウンロードします。
このプログラムは4つのライブラリを必要とします。
@<table>{sensor_library} の表を参考に、検索キーワードをライブラリマネージャの検索窓に入力してダウンロードしていってください。

//image[download_sensor_library][センサーライブラリ ダウンロード][scale=0.75]

//table[sensor_library][センサーライブラリ]{
使用するセンサー	ライブラリ名	検索キーワード
--------------------------
温度センサー（DS18B20）	OneWire	OneWire
温度センサー（DS18B20）	DallasTemperature	DallasTemperature
照度センサー（TSL2561）	Adafruit TSL2561 Library	Adafruit TSL2561
LCDディスプレイ	Grove_LCD_RGB_Backlight	Grove LCD RGB
//}

これらのライブラリをダウンロードすると、コンパイルに成功するはずです。

照度センサーのライブラリ「Adafruit TSL2561」をインストールしようとすると、依存関係で他のライブラリもダウンロードするかを聞かれますので、
「INSTALL ALL」を選択してください。

//image[install_dependent_library][依存関係のライブラリのインストール][scale=0.75]


=== 動作確認

それでは書き込んでみましょう。
書き込み後、Arduino IDEのシリアルモニタにこのようなログが表示されれば成功です。@<fn>{serial_monitor}

//footnote[serial_monitor][Serial Monitorは、Arduino IDEのメニューから「Tools」→「Serial Monitor」で開くことができます。]

//list[sensor_monitor][シリアルモニタの出力例]{
Temp: 30.0C | Water: 0% | Sound: 16% | Light: 44 lux
//}

数値が周期的に更新されれば、すべてのセンサーが動作しています。
もし接続していないセンサーがあれば、「NA」と表示されるはずです。

画面が文字化けする場合は、ボーレートの設定が115200bps になっていない可能性があるので、確認しましょう。

//image[serial_monitor_result][シリアルモニタの出力結果][scale=0.75]

== 液晶モニタに表示も可能

オプションのLCD液晶ディスプレイを接続した場合は、書き込み前にSeeduinoに接続していれば、結果を液晶モニタに表示することもできます。
ただし、接続場所はI2Cポートです。照度と水位でI2Cポートをすでに二つ使用しているため、どちらかを割り当てる必要があります。

このようにLCDにセンサー値が順次更新表示されれば成功です。

//image[lcd_display_result][液晶モニタの出力結果][scale=0.75]

一度ボードにプログラムが書き込まれると、あとはPCに接続せずに、電力を供給し続けるだけでセンサー値を確認することができます。
モバイルバッテリーでの駆動も可能なので、一時的に屋外の植物の環境を確認したい場合にも活用ができます。

//image[outdoor_sensor_result][闇夜に美しく光るLCDディスプレイ][scale=0.5]
