= デバイスとの通信

前章までで、Androidというプラットフォームの特徴と、Bluetoothという通信技術の仕組みを学びました。この章では、いよいよ実際のデバイスをAndroidアプリから操作し、その通信ログを読み解いていきます。

本書のアプリは3つのデバイスと通信します。電子はかり（BLE）、モバイルプリンター（Bluetooth Classic）、電子ペーパータグ（HTTP経由）。それぞれ通信方式が異なるため、前章で学んだBluetooth ClassicとBLEの違いが、実際のコードとログにどう現れるかを体験できます。


== アプリの概要

=== 技術スタック

本書のアプリはKotlinで記述し、Jetpack Compose（Material 3）でUIを構成しています。アーキテクチャにはMVVM（Model-View-ViewModel）を採用し、各画面がScreen（View）、ViewModel、UiStateの3層で構成されています。

//table[tech_stack][アプリの技術スタック]{
技術	用途
---------------------------------------------------------
Kotlin	開発言語
Jetpack Compose	UI構築（Material 3）
Kotlin Coroutines / Flow	非同期処理・状態管理
Android BLE API	電子はかりとのBLE通信
StarXpand SDK	モバイルプリンターとのBluetooth Classic通信
OkHttp	電子ペーパータグへのHTTP通信
Room	計量データのローカル保存
//}

=== ログの仕組み

通信の流れを理解するために、本書ではアプリが出力するログを活用します。各デバイスとの通信を担当するクライアントクラスがKotlinのStateFlowでログを蓄積し、画面にリアルタイムで表示します。

ログの形式は以下のようになっています。

//emlist{
[14:23:45.001][SCAN] Starting BLE scan (filter: "Decent Scale")
[14:23:46.234][SCAN] Found: Decent Scale (XX:XX:XX:XX:XX:XX) RSSI=-42dBm
[14:23:47.123][GATT] Connected (status=0)
//}

角括弧の1つ目がタイムスタンプ（ミリ秒精度）、2つ目が操作の種別です。@<tt>{SCAN}はBLEスキャン、@<tt>{GATT}はGATT接続・サービス発見、@<tt>{NOTIFY}はNotify受信、@<tt>{WRITE}はWrite送信を表します。

この形式により、前章で学んだBLE通信の各段階（スキャン→接続→サービス発見→サブスクライブ→データ送受信）が、ログのどこに対応するかが一目でわかります。


== 電子はかり：BLE通信の実践

この章の主役は電子はかりです。BLEのGATT通信（スキャン、接続、サービス発見、Notify、Write）のすべてを、実際のログを追いながら体験していきます。

//image[scale_ble.drawio][電子はかりとのBLE（GATT）通信の概要]

=== 製品紹介：Decent Scale

TODO: ここに電子はかりの写真を挿入

使用するのは、香港のエスプレッソマシンメーカーDecentが製造する電子はかりです。この会社の特徴的な点は、製品の通信仕様をGitHubで公開し、サードパーティからの接続を歓迎していることです@<fn>{decent_github}。BLEのサービスUUID、キャラクタリスティック、データフォーマットがすべてドキュメント化されており@<fn>{decent_api}、3Dモデルまで公開する徹底ぶりです。

//footnote[decent_github][@<href>{https://github.com/decentespresso/openscale}]
//footnote[decent_api][@<href>{https://decentespresso.com/decentscale_api}]

製品を開封してもマニュアル類は一切入っていません。ネット上で公開されている仕様書がすべてです。ファームウェアのアップデートが前提の、現代的な製品らしい割り切りです。

=== ステップ1：スキャンとアドバタイズの発見

BLE通信の最初のステップは、周囲のデバイスをスキャンすることです。電子はかりの電源を入れると、はかり（ペリフェラル）は「Decent Scale」というデバイス名を含むアドバタイズパケットを周囲に発信し続けます。スマートフォン（セントラル）は、このパケットを受信してデバイスを発見します。

アプリ側のコードでは、Android標準のBLE API（@<tt>{BluetoothLeScanner}）を使い、デバイス名「Decent Scale」でフィルタリングしたスキャンを実行します。

//emlist{
[14:23:45.001][SCAN] Starting BLE scan (filter: "Decent Scale")
[14:23:46.234][SCAN] Found: Decent Scale (C4:DE:E2:XX:XX:XX) RSSI=-42dBm
[14:23:46.235][SCAN] Scan stopped
//}

1行目でスキャンが開始され、約1.2秒後にデバイスが見つかりました。@<tt>{RSSI=-42dBm}は受信信号強度で、値が0に近いほど電波が強い（＝デバイスが近い）ことを意味します。一般的にRSSIが-50dBm以上であれば良好な通信が期待できます。

=== ステップ2：GATT接続とサービス発見

デバイスが見つかったら、GATT接続@<fn>{gatt_spec}を確立します。

//footnote[gatt_spec][GATTの技術仕様はBluetooth SIGが公開しています（@<href>{https://www.bluetooth.com/specifications/specs/}）。ただしCore Specificationは3000ページ超と聖書より多いので、通読して理解するのは現実的ではありません。実務では、Nordic SemiconductorやAndroid Developersの解説記事から入る方がおすすめです。]

//emlist{
[14:23:46.500][GATT] Connecting to C4:DE:E2:XX:XX:XX (transport=LE)...
[14:23:47.123][GATT] Connected (status=0)
[14:23:47.124][GATT] Discovering services...
//}

@<tt>{transport=LE}は、Bluetooth Classic（BR/EDR）ではなくBLEトランスポートで接続していることを示します。@<tt>{status=0}はGATT_SUCCESS、つまり接続成功です。

接続が確立すると、すぐにサービス発見（Service Discovery）が始まります。第2章で説明したように、BLEデバイスはサービスとキャラクタリスティックという階層構造でデータを公開しています。セントラルは接続後にこの構造を問い合わせ、「このデバイスは何ができるのか」を把握します。

//emlist{
[14:23:47.456][GATT] Services discovered: 3 service(s)
[14:23:47.457][GATT]   Service: FFF0 (2 characteristic(s))
[14:23:47.458][GATT]     └ FFF4 [Notify]
[14:23:47.459][GATT]     └ 36F5 [Write]
[14:23:47.460][GATT]   Service: 180A (3 characteristic(s))
[14:23:47.461][GATT]     └ 2A29 [Read]
[14:23:47.462][GATT]     └ 2A24 [Read]
[14:23:47.463][GATT]     └ 2A26 [Read]
//}

3つのサービスが発見されました。ここで重要なのは@<tt>{FFF0}サービスです@<fn>{fff0_uuid}。この中に2つのキャラクタリスティックがあります。@<tt>{FFF4}や@<tt>{36F5}も同様に、Decent Scale固有の番号です。つまり、Decent Scaleと通信する限り、これらの番号は常に登場します。

//footnote[fff0_uuid][@<tt>{FFF0}という値自体に特別な意味はありません。Bluetooth SIGが「ベンダーが自由に使ってよい」と定めた範囲（@<tt>{0xFFF0}〜@<tt>{0xFFFF}）に含まれる番号で、Decent社が自社のサービスに割り当てたものです。一方、後述の@<tt>{180A}（Device Information Service）のように、Bluetooth SIGが用途ごとに定義した標準UUIDも存在します。]

//table[decent_chars][Decent Scaleのキャラクタリスティック]{
UUID	プロパティ	役割
---------------------------------------------------------
FFF4	Notify	重量データのストリーミング（はかり → スマホ）
36F5	Write	コマンド送信（スマホ → はかり）
//}

もう1つの@<tt>{180A}サービスは「Device Information Service」というBLE標準のサービスで、製造者名やモデル番号、ファームウェアバージョンなどの情報を提供します。今回の通信には直接使いませんが、BLEデバイスが自身の情報を公開する仕組みとして覚えておくと良いでしょう。

=== ステップ3：Notifyのサブスクライブ

サービス構造がわかったら、重量データを受信するためにNotifyをサブスクライブ（購読登録）します。

//emlist{
[14:23:47.500][NOTIFY] Subscribing to FFF4...
[14:23:47.501][NOTIFY] Writing CCCD descriptor (0x0001 = ENABLE_NOTIFICATION)
[14:23:47.600][NOTIFY] Enabled on FFF4 (CCCD=0x0001)
[14:23:47.601][NOTIFY] Receiving weight data at ~10Hz
//}

ここで登場する@<b>{CCCD（Client Characteristic Configuration Descriptor）}は、Notifyを有効にするための特別なディスクリプタ（UUID: 2902）です。ディスクリプタとは、キャラクタリスティックに付属する設定値や補足情報のことです。前章の図書館のたとえでいえば、サービスが「棚」、キャラクタリスティックが「本」、ディスクリプタはその本に貼る「付箋」にあたります。CCCDは「この本が更新されたら教えて」と書き込むための付箋です。セントラルがこのディスクリプタに@<tt>{0x0001}を書き込むことで、「このキャラクタリスティックの変化を通知してほしい」とペリフェラルに伝えます。

これは第2章で説明した「サブスクライブ（購読登録）」の実際の仕組みです。Notifyは「ペリフェラルが勝手に送ってくる」ように見えますが、実際にはセントラル側からCCCDへの書き込みという明示的な許可が必要なのです。

=== ステップ4：重量データの受信（Notify）

サブスクライブが完了すると、電子はかりは毎秒約10回（10Hz）の頻度で重量データを送信し始めます。何も載せていなくても、0gのデータが送られ続けます。

//emlist{
[14:23:47.700][NOTIFY] RX: 03 CE 00 00 00 00 CD → 0.0g (stable)
[14:23:47.800][NOTIFY] RX: 03 CE 00 00 00 00 CD → 0.0g (stable)
[14:23:47.900][NOTIFY] RX: 03 CE 00 00 00 00 CD → 0.0g (stable)
//}

@<tt>{RX}はReceive（受信）を意味します。16進数で表示されているのが、Notifyで届いた生のバイト列です。このDecent Scaleは1回のNotifyで7バイトのデータを送信します。

==== 7バイトプロトコルの解読

7バイトの各フィールドの意味を見ていきましょう。

//table[decent_protocol][Decent Scale 重量データフォーマット（7バイト）]{
バイト位置	名前	値の例	意味
---------------------------------------------------------
0	モデル	0x03	Decent Scale固有の識別子
1	タイプ	0xCE / 0xCA	0xCE=安定、0xCA=不安定
2-3	重量	0x00 0x64	符号付き16bit整数（÷10で実重量）
4-5	（予約）	0x00 0x00	将来の拡張用
6	XOR	0xAD	バイト0〜5のXORチェックサム
//}

たとえば@<tt>{03 CE 00 64 00 00 AD}というデータを解読すると、以下のようになります。

 * バイト0: @<tt>{0x03} → Decent Scale
 * バイト1: @<tt>{0xCE} → 安定（stable）
 * バイト2-3: @<tt>{0x00 0x64} → 10進数で100 → 100 ÷ 10 = @<b>{10.0g}
 * バイト6: @<tt>{0xAD} → 0x03 XOR 0xCE XOR 0x00 XOR 0x64 XOR 0x00 XOR 0x00 = 0xAD ✓

@<b>{XORチェックサム}は、データが通信途中で壊れていないかを検証する仕組みです。バイト0から5までの値を順にXOR演算し、その結果がバイト6と一致すればデータは正常です。一致しなければ通信エラーが発生しているため、そのデータは破棄します。身近な例でいえば、QRコードにもデータの正しさを守る仕組みがありますが、あちらは壊れたデータを復元までできる高度な方式です。XORチェックサムは壊れたかどうかだけを判定するシンプルな手法で、壊れていたら捨てるしかありません。電子はかりのように毎秒10回もデータが届く用途では、1回分を捨てても次がすぐ届くので、この割り切りで十分なのです。

安定（stable）と不安定（unstable）の区別も興味深い点です。はかりに物を載せた直後は重量が変動するため「不安定」状態になり、値が安定すると「安定」に切り替わります。食品製造の現場では、安定状態になるまで待ってから計量値を確定する、という運用が一般的です。

//emlist{
[14:24:05.100][NOTIFY] RX: 03 CA 00 64 00 00 A9 → 10.0g (unstable)
[14:24:05.200][NOTIFY] RX: 03 CA 00 65 00 00 A8 → 10.1g (unstable)
[14:24:05.300][NOTIFY] RX: 03 CA 00 64 00 00 A9 → 10.0g (unstable)
[14:24:05.800][NOTIFY] RX: 03 CE 00 64 00 00 AD → 10.0g (stable)
//}

2バイト目が@<tt>{0xCA}（不安定）から@<tt>{0xCE}（安定）に変わったのがわかります。重量値も微妙に揺れた後、10.0gで安定しました。

=== ステップ5：風袋引き（Tare）によるWrite操作

Notifyがペリフェラルからセントラルへの方向だったのに対し、Writeはセントラルからペリフェラルへの方向です。電子はかりの場合、風袋引き（重量のゼロリセット）の指示をWriteで送ります。

風袋引きとは、容器の重量を差し引いて中身だけの重量を測るための操作です。たとえば、植物の鉢ごとの重量を測った後に風袋引きを行えば、次に水を注いだ時に「水の量」だけを正確に計れます。

//emlist{
[14:24:12.345][WRITE] TX: 03 0F 00 00 00 01 0E → 36F5 (Tare)
[14:24:12.456][WRITE] Success on 36F5
[14:24:12.500][NOTIFY] RX: 03 CE 00 00 00 00 CD → 0.0g (stable)
//}

@<tt>{TX}はTransmit（送信）です。7バイトのTareコマンド（@<tt>{03 0F 00 00 00 01 0E}）をキャラクタリスティック@<tt>{36F5}に書き込みます。書き込みが成功すると、はかりの表示がゼロにリセットされ、次のNotifyで0.0gが返ってきました。

=== BLE通信の全体像

ここまでのステップを整理すると、BLE通信の全体像が見えてきます。

//table[ble_flow][電子はかりとのBLE通信フロー]{
段階	ログラベル	操作	方向
---------------------------------------------------------
1. スキャン	SCAN	アドバタイズの発見	はかり → スマホ
2. 接続	GATT	GATT接続の確立	スマホ → はかり
3. サービス発見	GATT	サービス/キャラクタリスティックの列挙	スマホ ← はかり
4. サブスクライブ	NOTIFY	CCCDへの書き込み（0x0001）	スマホ → はかり
5. データ受信	NOTIFY	重量データの通知（10Hz）	はかり → スマホ
6. コマンド送信	WRITE	Tareコマンドの書き込み	スマホ → はかり
//}

この一連の流れが、第2章で学んだBLEの仕組み（セントラルとペリフェラル、GATT、サービスとキャラクタリスティック、Read/Write/Notify）の実際の姿です。

ここまで読むと複雑に感じるかもしれませんが、実際のKotlinコードでは、AndroidのBLE APIがこれらの手順を大部分隠蔽してくれます。たとえば、スキャンからNotifyの受信開始までは、おおむね次のような流れになります。

//emlist{
// 1. スキャン：デバイス名でフィルタリングして探す
val filter = ScanFilter.Builder().setDeviceName("Decent Scale").build()
bluetoothLeScanner.startScan(listOf(filter), scanSettings, scanCallback)

// 2. 接続：見つかったデバイスにGATT接続する
val gatt = device.connectGatt(context, false, gattCallback, TRANSPORT_LE)

// 3. サービス発見：接続成功後に呼び出す
gatt.discoverServices()

// 4. Notifyのサブスクライブ：CCCDへの書き込みもヘルパーで1行
gatt.setCharacteristicNotification(characteristic, true)
descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
gatt.writeDescriptor(descriptor)
//}

スキャン、接続、サービス発見、サブスクライブという4つのステップが、それぞれ数行のコードで表現できています。ログで追いかけた各段階が、コード上ではAPIの呼び出しとして対応していることがわかります。本書のアプリでは、これらをさらに@<tt>{ScaleClient}というクラスにまとめて、画面側からは「接続して」「切断して」といった単純な操作だけで済むようにしています。


//embed[latex]{
\clearpage
//}

== モバイルプリンター：Bluetooth Classicの通信

//image[printer_classic.drawio][モバイルプリンターとのBluetooth Classic（SPP）通信の概要]

=== 製品紹介：スター精密 SM-S210i

モバイルプリンターとは、小型でバッテリー駆動の持ち運び可能なプリンターです。インクは使わず、感熱紙に熱で印字するため「サーマルプリンター」とも呼ばれます。

使用するのはスター精密のSM-S210iです。主に業務用として設計されたモデルですが、2026年4月時点でAmazonからも購入できます。感熱紙も一般的な規格の58mmロール紙で、モノタロウやアスクルなどの通販サイトで手に入ります。

TODO: プリンターの写真

=== SDKによるBluetooth Classicの隠蔽

電子はかりとの通信では、BLE APIを直接操作し、GATT接続からNotifyのサブスクライブまですべてのステップをアプリ側で制御しました。しかし、モバイルプリンターとの通信はまったく様相が異なります。

スター精密はStarXpand SDKというKotlin/Swift向けのSDKを提供しており、開発者はこのSDKを通じてプリンターを操作します。第2章で説明したように、この製品はBluetooth ClassicのSPP（Serial Port Profile）を内部的に使っていますが、SDKがその詳細をすべて隠蔽しています。

以下はプリンターの発見から印刷までのログです。

//emlist{
[14:25:01.001] > Searching for printers...
[14:25:03.234] > Found device: SM-S210i
[14:25:05.456] > Discovery finished
[14:25:05.500] > Connecting to SM-S210i...
[14:25:05.501] > Testing connection...
[14:25:06.789] > Connection successful
[14:25:06.790] > Ready to print
//}

電子はかりのログと比べてみてください。BLEの場合はサービス発見やキャラクタリスティックのUUID、CCCDへの書き込みなど、通信の各段階が細かく見えていました。一方、プリンターのログでは「発見→接続→準備完了」という抽象的な情報しかありません。

=== SDKの裏で起きていること

SDKの裏側では、以下のような処理が行われていると推測できます。

//table[sdk_hidden][SDKが隠蔽しているBluetooth Classic通信]{
SDKの操作	裏で起きていること
---------------------------------------------------------
StarDeviceDiscoveryManager.startDiscovery()	Bluetooth Classicのデバイス検索（Inquiry）
StarPrinter.openAsync()	SPPチャンネルの確立（RFCOMM接続）
StarPrinter.printAsync(commands)	シリアルデータとしてのコマンド送信
StarPrinter.closeAsync()	RFCOMM切断
//}

第2章で説明したSPP（Serial Port Profile）を思い出してください。SPPはシリアル通信をBluetooth上でエミュレートするプロファイルです。SDKは内部的にRFCOMM（Radio Frequency Communication）という仕組みでSPP接続を確立し、プリンター固有のコマンド体系に従って印刷データを送信しています。

開発者がSPPやRFCOMMを直接意識する必要がないのは、SDKが設計として意図した結果です。
Bluetooth Classicのペアリング、チャンネルの確立、エラーハンドリングといった低レベル（ハードウェアに近い階層）の処理を開発者に任せると、実装の品質がバラつき、サポートコストが増大することが想定されます。
産業用機器のメーカーがSDKを提供する大きな理由がここにあります。

=== 印刷コマンドの構築

印刷時のコードを見てみましょう。StarXpand SDKではビルダーパターンでコマンドを組み立てます。

//emlist{
[14:25:10.001] > Printing: "植物の生育記録"
[14:25:10.002] > Opening printer...
[14:25:11.234] > Sending print command...
[14:25:12.456] > Print completed
[14:25:12.457] > Closing printer...
//}

ここでは、印刷のたびに接続と切断を行っています。常時接続を維持する方法もありますが、本書のアプリでは以下の理由から逐次接続方式を採用しています。

 * 親機（スマートフォン）を変更した際に、前の親機との接続解除を忘れるリスクを防ぐ
 * プリンターのバッテリー消費を抑える
 * Bluetooth Classicは接続数の上限が厳しいため、不要な接続を保持しない

=== BLEとBluetooth Classicの開発体験の違い

電子はかりとモバイルプリンターの開発を通じて、BLEとBluetooth Classicの開発体験の違いが見えてきました。

//table[dev_experience][BLEとBluetooth Classicの開発体験の比較]{
項目	電子はかり（BLE）	モバイルプリンター（Classic）
---------------------------------------------------------
使用API	Android BLE API（直接操作）	StarXpand SDK（抽象化）
通信の可視性	高い（UUIDやデータが全て見える）	低い（SDKが隠蔽）
ペアリング	不要	必要（事前にOS設定で実施）
接続方式	ペアリングなしで直接接続	ペアリング後にSPP接続
デバッグ	ログで各段階を追跡可能	SDK側のエラーメッセージに依存
//}

BLEはプロトコルが標準化されており、Android APIで直接操作できるため、通信の各段階を細かく制御・観察できます。一方、Bluetooth ClassicはSDKが提供されている場合、通信の詳細は隠蔽されますが、その分、開発者は印刷ロジックに集中できます。


//embed[latex]{
\clearpage
//}

== 電子ペーパー：ESP32経由のHTTP通信

//image[epaper_http.drawio][電子ペーパーとのESP32経由HTTP通信の概要]

=== 製品紹介：電子ペーパータグ

最後に紹介するのは電子ペーパータグです。正確にはGiciskyが販売する2.9インチの小型電子ペーパーディスプレイで、家電量販店やスーパーマーケットの棚に取り付けられている電子棚札と同じ技術です。

電子ペーパーの最大の特徴は省電力性です。表示の書き換え時にのみ電力を消費し、表示を維持すること自体には電力を必要としません。そのため、バッテリー1つで数年間の運用が可能です。Kindle Paperwhiteのような電子書籍リーダーにも使われているこの技術は、近年ではインテリア用の大型ディスプレイとしても一般消費者に浸透しています。

=== 構成：Android → ESP32 → 電子ペーパー

他の2つのデバイスとの大きな違いは、Androidと電子ペーパータグの間に @<b>{中継機（ESP32）} が存在する点です。

//table[epaper_architecture][電子ペーパー通信の構成]{
区間	通信方式	内容
---------------------------------------------------------
Android → ESP32	HTTP（Wi-Fi）	画像データのPOST送信
ESP32 → 電子ペーパータグ	BLE（OpenEPaperLink）	画像データの書き込み
//}

なぜAndroidから直接BLEで書き込まないのか。
それは、電子ペーパータグへの書き込みにOpenEPaperLinkという特定のファームウェアプロトコルが必要で、このプロトコルを実装したAndroid向けのライブラリが存在しないためです。
OpenEPaperLinkはESP32をはじめとする複数のマイコン基板に対応したファームウェアで、本書ではESP32を使用します。ESP32がアクセスポイント（AP）として動作し、
Wi-Fi経由でHTTPリクエストを受け付けます。

つまり、実際のBluetooth通信はESP32に委譲し、Android側はHTTPで画像データを送信するという構成です。

=== 画像として送信する理由

アプリの特徴的な設計として、表示させる内容をテキストではなく画像として送信しています。テキストとして送信することも技術的には可能ですが、その場合はESP32側でフォントのレンダリングが必要になります。

特に日本語を表示したい場合、フォントの問題は深刻です。ESP32のメモリに日本語フォントを格納する容量は限られており、意図しない文字化けや改行が発生する可能性があります。画像であれば、Android側でレンダリングした結果をそのまま送信するため、プレビュー通りの表示が保証されます。

=== HTTPリクエストのログ

通信の流れをログで確認します。

//emlist{
[14:26:01.001] > Connecting to AP...
[14:26:01.002] > URL: http://192.168.1.100/imgupload
[14:26:01.003] > POST /imgupload
[14:26:01.004] > Content-Type: multipart/form-data
[14:26:01.005] > file: image.jpg (12345 bytes)
[14:26:01.006] > mac: 'XX:XX:XX:XX:XX:XX:XX:XX'
[14:26:02.789] > Response: 200 OK
//}

OkHttp（Android/Kotlin向けの定番HTTPクライアントライブラリ）を使ったmultipart/form-data形式のPOSTリクエストです。
電子ペーパータグのMACアドレスと、JPEG画像データを送信しています。ESP32がこのリクエストを受け取ると、内部でBLEを使って電子ペーパータグに画像データを書き込みます。

Bluetooth通信の詳細を追うことはできませんが、「Bluetoothを直接使えない場面で、中継機を介してHTTPに変換する」というアーキテクチャは、IoTシステムの設計パターンとして参考になるでしょう。


== まとめ

この章では、3つのデバイスとの通信を通じて、Bluetoothの仕組みが実際のアプリでどのように現れるかを見てきました。

//table[chapter_summary][3つのデバイスの通信方式の比較]{
デバイス	通信方式	特徴
---------------------------------------------------------
電子はかり	BLE（GATT）	Notify/Writeを直接操作。プロトコルを解読可能
モバイルプリンター	Bluetooth Classic（SPP）	SDKが通信を隠蔽。印刷ロジックに集中可能
電子ペーパー	HTTP（ESP32経由）	中継機にBluetooth通信を委譲
//}

電子はかりでは、BLEのスキャン→GATT接続→サービス発見→Notifyサブスクライブ→データ受信→Writeコマンド送信という一連の流れを、ログの1行1行で追うことができました。
第2章で理論として学んだGATT通信の仕組みを、
実際のバイト列をログから直接読み取る体験は、仕様書を読むだけでは得られないものです。

モバイルプリンターでは、SDKが通信の複雑さを隠蔽することで開発者体験を向上させている実例を見ました。
Bluetooth ClassicのSPPが裏側で動いていることを知りつつも、開発者はそれを意識せずに印刷機能を実装できます。

電子ペーパーでは、Bluetoothを直接使えない制約を、HTTP通信と中継機で解決するアーキテクチャを紹介しました。

3つのデバイスがそれぞれ異なるアプローチを取っているのは、デバイスの特性や既存のエコシステムに応じた最適解を選んだ結果です。Bluetoothという1つの技術が、実際の開発においてはこれほど多様な姿で現れるということが、本章で最も伝えたかったことです。
