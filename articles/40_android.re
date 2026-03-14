= Androidアプリ開発の基礎

第3章までで、3つのデバイスとの通信がどのように行われるかをログとともに追いかけてきました。ここからは、いよいよ自分で作る側に回ります。

この章では、Androidアプリ開発の環境構築からプロジェクトの作成、UIの基本、アーキテクチャの考え方までを駆け足で紹介します。Android開発の教科書を書くつもりはありません。「本書のコードを読むために最低限知っておくべきこと」に絞った内容です。KotlinやAndroidに馴染みのある方は、この章は読み飛ばしてもらって構いません。

逆に、これまでAndroidアプリを作ったことがない方は、この章を手を動かしながら読むことをおすすめします。コードの意味がわからないまま次章に進んでも、おそらく途中で迷子になります。


== Android Studioのインストール

Android開発を始めるにあたって最初にやることは、Android Studioのインストールです。

Android Studioは、Googleが公式に提供しているAndroidアプリ開発用のIDE（統合開発環境）です。JetBrains社のIntelliJ IDEAをベースに作られており、コードの編集、ビルド、デバッグ、エミュレータの実行まで、開発に必要なほぼすべてがこの1つのアプリに詰まっています。

ダウンロードは公式サイト（https://developer.android.com/studio）から行えます。Windows、macOS、Linux、ChromeOSに対応しています。第1章で「え、Android開発ってMacでできるの？」と驚いた話を書きましたが、逆に言えばWindowsでもLinuxでも問題なく開発できるということです。

TODO: Android Studio公式ダウンロードページのキャプチャ（対応OS一覧が見える状態）

ここでiOS開発と比較してみましょう。iOSアプリの開発にはXcodeが必要で、XcodeはmacOSでしか動作しません。つまりiOSアプリを作りたければ、Macを買うところから始まります。Androidにはそうした制約がありません。これは第1章で触れた「オープンなプラットフォーム」という思想が、開発環境にも一貫して現れているところです。

さらに言えば、Android開発は必ずしもAndroid Studioでなければならないわけでもありません。ビルドシステムであるGradleとAndroid SDKはAndroid Studioとは独立したツールなので、VSCodeとコマンドラインだけでもアプリをビルドして実機にインストールすることは可能です。FlutterやReact NativeによるAndroid開発では、むしろVSCodeの方が主流のエディタです。

「Android StudioじゃなくてCursorやClineを使いたい」という声も聞こえてきそうです。AIアシスタントが統合された開発環境に慣れていると、それなしのIDEに戻るのは抵抗があるかもしれません。実際、CursorでKotlinのコードを編集し、ビルドと実行はAndroid Studioで行うという「デュアルIDE」のワークフローを実践している開発者もいます。ただし、Kotlin LSP（言語サーバー）はAndroidの複雑なGradleプロジェクトへの対応がまだ十分ではなく、Cursor単体でAndroid開発を完結させるのは現時点では難しいのが実情です。

なお、Android Studio自体もAI支援の方向に進化しています。GoogleのGeminiが統合されており、コード補完やチャットに加え、Agent Modeではマルチファイルのリファクタリングやテスト生成を自然言語で指示できます。ワイヤーフレームの画像からComposeのコードを生成する機能まであり、「Android開発に特化したAI支援」として独自の立ち位置を築きつつあります。

そして、本書で使うClaude CodeもAndroid Studioの中で動かせます。方法は2つあり、併用するのが効果的です。

1つ目は、Android Studio下部のTerminalタブで@<tt>{claude}コマンドを実行する方法です。プロジェクトのルートディレクトリがそのまま作業ディレクトリになるので、Claude Codeはプロジェクト構造を即座に把握できます。Gradleのビルドコマンドも同じターミナルから実行可能です。新しい機能をまるごと追加したり、複数ファイルにまたがるリファクタリングを指示したりと、プロジェクト全体を見渡す大きな作業に向いています。

TODO: Android StudioのTerminalタブでClaude Codeを起動した画面のキャプチャ

2つ目は、JetBrains MarketplaceからインストールできるClaude Codeプラグインです。こちらはCursorやGitHub Copilotに近い「エディタ補助型」のツールで、今開いているファイルや選択中のコードに対する修正提案、IDE上での差分表示とワンクリックでの適用、ショートカットキーでの呼び出しといった機能が使えます。ターミナルに移動せずに済むので、開発フローの中断が少ないのが利点です。

TODO: Claude Codeプラグインの差分ビュー（Accept/Rejectボタンが見える状態）のキャプチャ

つまり、大きな方針はターミナル版のClaude Codeに任せ、手元のコードの微調整はプラグイン版で素早くこなす。そしてComposeプレビューや実機確認はAndroid Studioの本来の機能で行う。すべてがAndroid Studioの1つのウィンドウの中で完結します。CursorとAndroid Studioを行き来するデュアルIDE運用に比べると、コンテキストスイッチが少ないのが大きな強みです。次章で、このワークフローの実際を詳しく見ていきます。

=== バージョンの話

本書の執筆時点（2026年3月）での安定版はAndroid Studio Panda 2です。Android Studioのバージョン名は動物の名前がアルファベット順に付けられており、過去にはGiraffe、Hedgehog、Iguana、Jellyfish、Koala、Ladybug、Meerkatなどがありました。本書のスクリーンショットはPanda 2に基づいていますが、基本的な操作は多少バージョンが違っても大きく変わりません。

=== インストール手順

インストール自体は特に複雑ではありません。ダウンロードしたインストーラを実行し、ウィザードに従って進めるだけです。途中でAndroid SDKのインストール先を聞かれますが、特別な理由がなければデフォルトのままで問題ありません。

初回起動時にAndroid SDKやエミュレータのイメージのダウンロードが始まります。ここでそれなりの時間がかかるので、安定したネットワーク環境で作業することをおすすめします。


== プロジェクトの作成

Android Studioを起動すると、ウェルカム画面が表示されます。「New Project」を選択してプロジェクトを作成しましょう。

TODO: Android Studioウェルカム画面のキャプチャ（New Projectボタンが見える状態）

=== テンプレートの選択

プロジェクト作成画面では、いくつかのテンプレートが並んでいます。ここで選ぶのは「Empty Activity」です。

TODO: テンプレート選択画面のキャプチャ（Empty ActivityとEmpty Views Activityが両方見える状態）

かつて、ここには「Empty Activity」と「Empty Compose Activity」の2つがありました。前者はXMLレイアウト、後者はJetpack Composeを使うテンプレートです。しかし現在は、Empty ActivityがデフォルトでJetpack Composeを使う構成になっています。XMLレイアウトは「Empty Views Activity」として別に用意されるようになりました。

この変化自体が、AndroidのUI開発がどちらに向かっているかを端的に示しています。

=== プロジェクトの設定

テンプレートを選んだら、いくつかの設定を入力します。

TODO: プロジェクト設定画面のキャプチャ（Name、Package name、Minimum SDKなどの入力欄が見える状態）

//table[project_settings][プロジェクト作成時の設定例]{
項目	説明	本書での設定例
---------------------------------------------------------
Name	アプリの表示名	HydroLog
Package name	アプリの一意な識別子	com.example.hydrolog
Save location	プロジェクトの保存先	任意
Minimum SDK	サポートする最低Androidバージョン	API 26（Android 8.0）
Build configuration language	ビルドスクリプトの言語	Kotlin DSL
//}

Minimum SDKの設定は少し考えどころです。低く設定すれば多くの端末で動作しますが、古いAPIの互換性コードが増えます。高く設定すれば新しいAPIが使えますが、対象端末が減ります。API 26（Android 8.0）を選ぶと、2026年時点で世界のAndroid端末の約95%以上をカバーできます。本書のBLE関連の実装で必要なAPIもこの範囲に収まります。

=== 生成されるファイル

「Finish」を押すと、プロジェクトが生成されます。初回はGradleの同期が走るので少し待ちましょう。

TODO: プロジェクト生成直後のAndroid Studio画面のキャプチャ（MainActivity.ktが開かれ、左側にプロジェクトツリーが見える状態）

生成されるファイルの中で、最初に目を通すべきものを整理しておきます。

//table[generated_files][生成される主要ファイル]{
ファイル	役割
---------------------------------------------------------
@<tt>{app/src/main/java/.../MainActivity.kt}	アプリのエントリーポイント
@<tt>{app/src/main/AndroidManifest.xml}	アプリの設定ファイル（権限、画面構成など）
@<tt>{app/build.gradle.kts}	アプリのビルド設定（依存関係、コンパイル設定など）
@<tt>{gradle/libs.versions.toml}	依存ライブラリのバージョンを一元管理するカタログ
@<tt>{settings.gradle.kts}	プロジェクト全体の設定
//}

@<tt>{libs.versions.toml}は比較的新しい仕組みで、以前は@<tt>{build.gradle.kts}に依存関係のバージョンを直接書いていました。ライブラリの数が増えると同じバージョン番号があちこちに散らばって管理が煩雑になるため、バージョンカタログとして一箇所にまとめる方式が推奨されるようになりました。

Web開発の経験がある方は、Node.jsの@<tt>{package.json}やPythonの@<tt>{requirements.txt}に近い役割だと思えばイメージしやすいでしょう。


== ビルドとエミュレータでの実行

プロジェクトが作成できたら、まずは何も変更せずにビルドして動かしてみましょう。「動くこと」を確認してから手を加えるのが、環境構築のトラブルシューティングを楽にするコツです。

=== エミュレータの準備

Android Studioにはエミュレータ（仮想デバイス）が付属しています。実機がなくてもアプリの動作を確認できます。

ツールバーのデバイス選択ドロップダウンから「Device Manager」を開き、仮想デバイスを作成します。機種やAndroidバージョンを選べますが、ここではデフォルトで提案されるものをそのまま使えば十分です。

TODO: Device Manager画面のキャプチャ（仮想デバイス一覧が見える状態）

エミュレータのイメージを初めてダウンロードする際にはそれなりの容量（数GB）が必要です。また、エミュレータの動作にはPCのメモリとCPUをかなり使います。開発マシンのRAMが8GB未満だと動作が重くなる可能性があります。

=== 実行

ツールバーの緑色の再生ボタン（Run）を押すと、ビルドが始まり、成功するとエミュレータが起動してアプリが表示されます。

テンプレートから生成されたアプリは、画面の中央に「Hello Android!」と表示されるだけのシンプルなものです。しかし、この1行の表示に至るまでに、Kotlinのコンパイル、リソースの処理、APKの生成、エミュレータへのインストールという一連のビルドプロセスが走っています。

TODO: エミュレータで「Hello Android!」が表示されている画面のキャプチャ

=== 実機での実行

本書では最終的にBluetooth機器と接続するため、実機での動作確認が不可欠です。エミュレータにはBluetooth機能がないからです。

Androidの実機テストはシンプルです。端末の「設定」から「開発者向けオプション」を有効にし、「USBデバッグ」をオンにして、USBケーブルでPCに接続します。Android Studioのデバイス選択に端末名が表示されたら、あとはエミュレータと同じようにRunボタンを押すだけです。

TODO: デバイス選択ドロップダウンに実機の端末名が表示されている状態のキャプチャ

ここでもiOSとの違いが出ます。iOSの場合、実機テストにはApple Developer Programへの登録（個人利用なら無料のApple IDでも可能ですが、制約があります）やプロビジョニングプロファイルの設定が必要です。Androidにはそうした手続きがなく、ケーブルを繋いですぐにテストできます。ただし、Google Play Storeへのアプリ公開にはGoogle Playデベロッパーアカウント（登録料$25、一回のみ）が必要です。


== Jetpack Composeによる宣言的UI

テンプレートで生成された@<tt>{MainActivity.kt}を開くと、見慣れないコードが目に入るかもしれません。

//emlist{
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MyAppTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        name = "Android",
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}
//}

これがJetpack Composeのコードです。

=== XMLレイアウトからの脱却

Androidアプリの画面を作る方法は、2つの時代に分けられます。

2008年のAndroid誕生から2021年頃まで、画面のレイアウトはXMLで記述するのが標準でした。UIの見た目をXMLファイルで定義し、画面の動きをKotlin（またはJava）で書く。「何がどこに表示されるか」と「それがどう動くか」が別々のファイルに分かれている構造です。Web開発でいえば、HTMLとJavaScriptを分離する考え方に近いものがありました。

しかし、アプリのUIが複雑になるにつれて、この分離が逆に足かせになってきました。XMLファイルとKotlinファイルの間を行き来する手間、@<tt>{findViewById}で画面要素を取得するボイラープレートコード、UIの状態管理の煩雑さ。こうした課題を解決するために、GoogleはJetpack Composeを開発しました。

=== 宣言的UIとは

Jetpack Composeは「宣言的UI」のフレームワークです。

「宣言的」とは、「UIがどう見えるべきか」を記述するアプローチです。対して従来のXMLレイアウト方式は「命令的」で、「UIをどう組み立てるか」を記述します。

たとえば、名前の一覧を表示したいとします。

命令的アプローチ（従来のXML + Kotlin）では、こう考えます。「まずRecyclerViewを配置して、Adapterクラスを作り、ViewHolderを定義して、onBindViewHolderでデータをビューにバインドして...」。手順を逐一指示する料理レシピのようなものです。

宣言的アプローチ（Compose）では、こう書きます。

//emlist{
@Composable
fun NameList(names: List<String>) {
    LazyColumn {
        items(names) { name ->
            Text(text = name)
        }
    }
}
//}

「名前のリストがあるので、それぞれをTextとして縦に並べて」。完成形を宣言しているだけです。リストの生成や更新のタイミングはフレームワークが面倒を見てくれます。

この「宣言的UI」というパラダイムは、Androidに限った動きではありません。iOSではSwiftUI（2019年発表）、WebではReact（2013年）やFlutter（2018年）など、プラットフォームを問わず同じ方向に進化しています。「UIの状態管理はフレームワークに任せて、開発者は見た目の定義に集中する」という考え方が、業界全体のトレンドになっています。

=== Composable関数

Composeの基本単位は@<tt>{@Composable}アノテーションが付いた関数です。先ほどの@<tt>{Greeting}もComposable関数です。

//emlist{
@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}
//}

Composable関数は通常の関数と見た目は似ていますが、いくつかの特別なルールがあります。

1つ目は、Composable関数の中からしか他のComposable関数を呼べないこと。@<tt>{Text}も@<tt>{Column}も@<tt>{Button}もすべてComposable関数なので、UIの記述は常にComposable関数の中で行います。

2つ目は、Composable関数は「再コンポジション」される可能性があること。状態（State）が変わると、Composeはその状態に依存するComposable関数を再度呼び出してUIを更新します。この「必要な部分だけを再描画する」仕組みが、Composeの効率的なUI更新を支えています。

=== 状態（State）

UIが変化するためには「状態」が必要です。Composeでは@<tt>{remember}と@<tt>{mutableStateOf}を使って状態を管理します。

//emlist{
@Composable
fun Counter() {
    var count by remember { mutableStateOf(0) }

    Column {
        Text(text = "Count: $count")
        Button(onClick = { count++ }) {
            Text("Increment")
        }
    }
}
//}

@<tt>{remember}は「再コンポジションをまたいでこの値を覚えておいて」という指示です。@<tt>{mutableStateOf}は「この値が変わったらUIを更新して」という宣言です。@<tt>{count}の値が変わるとComposable関数が再実行され、画面に表示される数字が更新されます。

開発者が「値が変わったからUIを更新する」コードを書く必要はありません。状態とUIの紐付けをComposeが自動で管理してくれます。

=== プレビュー

Composable関数にはプレビュー機能があります。@<tt>{@Preview}アノテーションを付けた関数を書くと、Android Studioのエディタ上でUIのプレビューが確認できます。

//emlist{
@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    MyAppTheme {
        Greeting("Android")
    }
}
//}

アプリ全体をビルドしてエミュレータで確認する必要がなく、個々のUIパーツを素早く確認・調整できます。第3章で紹介したように、Androidのビルドにはそれなりの時間がかかるため、このプレビュー機能は開発効率に大きく貢献します。

TODO: Android Studioのエディタ右側にComposeプレビューが表示されている画面のキャプチャ（コードとプレビューが左右に並んでいる状態）

=== Jetpack ComposeとSwiftUI

ここまで読んで、iOSのSwiftUIに似ていると感じた方もいるかもしれません。実際、両者は驚くほど似ています。

どちらも宣言的UIフレームワークであり、状態管理の考え方も近く、コードの見た目すら似通っています。Composeは@<tt>{Column}と@<tt>{Row}でレイアウトし、SwiftUIは@<tt>{VStack}と@<tt>{HStack}でレイアウトする。Composeは@<tt>{remember}で状態を保持し、SwiftUIは@<tt>{@State}で状態を保持する。

しかし、設計思想には重要な違いがあります。SwiftUIはOSに組み込まれているため、新機能を使うにはユーザーがOSをアップデートする必要があります。一方、Composeは通常のライブラリとして配布されるため、アプリのビルド時にバージョンを指定するだけで最新の機能が使えます。古いAndroid端末であっても、Composeのライブラリを同梱すれば新しいUIの仕組みを利用できるのです。

第1章で見たフラグメンテーション（バージョンの断片化）を思い出してください。Androidは複数のOSバージョンが並存する世界です。UIフレームワークをOSから切り離してライブラリ化したのは、この断片化への回答でもあります。


== MVVMとディレクトリ構造

ここまでで、画面にUIを表示する基本がわかりました。次は、アプリ全体をどう設計するかという話です。

=== なぜアーキテクチャが必要なのか

小さなアプリであれば、1つのファイルにすべてのコードを書いても動きます。しかし、本書のアプリのように「BLEでセンサーデータを受信し、画面に表示し、データベースに保存し、プリンターに送信する」といった処理が絡み合うと、コードの見通しが急速に悪くなります。

画面の表示ロジックと通信処理とデータ保存が1つのファイルに混在していると、「はかりの値を表示するコードを修正したいのに、プリンターの接続処理が邪魔で見つからない」という事態になります。コードの修正がどこに影響するかも予測しづらくなります。

こうした問題に対処するための設計パターンがアーキテクチャです。本書では、Androidの公式ガイドラインで推奨されている @<b>{MVVM（Model-View-ViewModel）} を採用しています。

=== MVVMの3つの層

MVVMは、アプリのコードを3つの役割に分離する設計パターンです。

//table[mvvm_layers][MVVMの3層]{
層	役割	本書での対応
---------------------------------------------------------
View	UIの表示とユーザー操作の受け取り	Composable関数（@<tt>{XXXScreen}）
ViewModel	UIの状態管理とビジネスロジック	@<tt>{XXXViewModel}
Model	データの取得・保存	Repository、Room Database
//}

@<b>{View}（本書ではScreen）は、画面に何を表示するかだけを担当します。「はかりの値が変わったからテキストを更新する」「ボタンが押されたからViewModelに伝える」。それ以上のことはしません。

@<b>{ViewModel}は、ViewとModelの間に立って、UIに必要な状態を管理します。「はかりから受信した生のバイト列を、画面に表示できる形式に変換する」「ユーザーが収穫ボタンを押したら、データベースに保存する指示を出す」。Viewが直接Modelにアクセスすることはなく、必ずViewModelを経由します。

@<b>{Model}は、データの実体です。本書ではRoomデータベースへの保存や、BLEデバイスとの通信クライアントがここに該当します。

=== 単方向データフロー

MVVMで重要なのは、データの流れが一方向であることです。

//emlist{
ユーザー操作 → View → ViewModel → Model
                ↑                    |
                |     状態の更新      |
                ← ← ← ← ← ← ← ← ←
//}

ユーザーがボタンを押すと、ViewはViewModelのメソッドを呼びます。ViewModelはModelからデータを取得・更新し、自身が持つ状態（UiState）を更新します。Viewはその状態の変化を検知して画面を再描画します。

データが常に同じ方向に流れるので、「この画面の表示がおかしい」と思ったときに、追うべき経路が明確になります。

=== UiState

本書のアプリでは、各画面の状態を@<tt>{UiState}というデータクラスで表現しています。

//emlist{
data class ScaleUiState(
    val weight: Float = 0.0f,
    val isStable: Boolean = false,
    val isConnected: Boolean = false,
    val logs: List<String> = emptyList()
)
//}

ViewModelがこの@<tt>{UiState}を更新し、Viewがそれを観察（collect）してUIに反映する。この構造が、すべての画面で統一されています。

=== ディレクトリ構造

本書のアプリのディレクトリ構造を見てみましょう。

//emlist{
app/src/
├── main/java/com/example/hydrolog/    ← アプリ本体のコード
│   ├── ui/
│   │   ├── home/
│   │   │   ├── HomeScreen.kt
│   │   │   └── HomeViewModel.kt
│   │   ├── scale/
│   │   │   ├── ScaleScreen.kt
│   │   │   ├── ScaleViewModel.kt
│   │   │   └── ScaleUiState.kt
│   │   ├── printer/
│   │   │   ├── PrinterScreen.kt
│   │   │   └── PrinterViewModel.kt
│   │   └── navigation/
│   │       └── NavGraph.kt
│   ├── data/
│   │   ├── database/
│   │   │   ├── AppDatabase.kt
│   │   │   └── CultivationDao.kt
│   │   └── model/
│   │       └── CultivationRecord.kt
│   ├── bluetooth/
│   │   ├── ScaleClient.kt
│   │   └── PrinterClient.kt
│   └── MainActivity.kt
├── test/java/com/example/hydrolog/    ← 単体テスト（JVM上で実行）
│   └── ui/scale/
│       └── ScaleViewModelTest.kt
└── androidTest/java/com/example/hydrolog/  ← UIテスト（エミュレータ/実機で実行）
    └── ui/scale/
        └── ScaleScreenTest.kt
//}

@<tt>{main/}がアプリ本体、@<tt>{test/}が単体テスト、@<tt>{androidTest/}がUIテストです。この3つのディレクトリが並列に存在するのがAndroidプロジェクトの基本構造です。@<tt>{test/}のコードは開発マシンのJVM上で高速に実行され、@<tt>{androidTest/}のコードはエミュレータや実機の上で動きます。

@<tt>{main/}の中を見ると、@<tt>{ui/}以下が画面ごとにパッケージ分けされたViewとViewModel、@<tt>{data/}がModel層、@<tt>{bluetooth/}がデバイス通信のクライアントです。ファイルを探すときに「これはどの層の話だろう」と考えれば、目的のファイルにたどり着けるようになっています。

=== State Hoisting

Composeの設計パターンとして知っておきたいのが「State Hoisting（状態の巻き上げ）」です。

本書のコードでは、各画面が@<tt>{XXXScreen}と@<tt>{XXXScreenContent}の2つのComposable関数に分かれています。

//emlist{
// ScaleScreen.kt

@Composable
fun ScaleScreen(viewModel: ScaleViewModel = viewModel()) {
    val uiState by viewModel.uiState.collectAsState()
    ScaleScreenContent(
        uiState = uiState,
        onTareClick = { viewModel.sendTare() }
    )
}

@Composable
fun ScaleScreenContent(
    uiState: ScaleUiState,
    onTareClick: () -> Unit
) {
    // UIの描画のみ。状態の管理はしない
    Column {
        Text("Weight: ${uiState.weight}g")
        Button(onClick = onTareClick) {
            Text("Tare")
        }
    }
}
//}

@<tt>{ScaleScreen}はViewModelから状態を取得し、@<tt>{ScaleScreenContent}に渡す役割だけを持ちます。@<tt>{ScaleScreenContent}は渡された状態をもとにUIを描画するだけで、ViewModelのことは知りません。

この分離により、@<tt>{ScaleScreenContent}はプレビューやテストで単独で使えるようになります。ViewModelに依存しないので、任意の状態を渡してUIの見た目を確認できるのです。


== テストの基本

アプリのコードが書けたら、それが正しく動くことを確認する必要があります。本書では深入りしませんが、Androidアプリのテストの全体像を把握しておきましょう。

=== テストの種類

Androidのテストは大きく3つに分かれます。

//table[test_types][Androidテストの種類]{
種類	実行場所	速度	何を確認するか
---------------------------------------------------------
単体テスト（Unit Test）	開発マシン上のJVM	速い	ロジックの正しさ
UIテスト	エミュレータまたは実機	遅い	画面の表示と操作
プレビューテスト	Android Studio上	中間	UIの見た目
//}

=== 単体テスト

ViewModelのロジックをテストするには、@<tt>{test/}ディレクトリに通常のKotlinのテストを書きます。先ほどのディレクトリ構造で見たように、テストファイルは@<tt>{app/src/test/}以下に、テスト対象と同じパッケージ構造で配置します。

//emlist{
class ScaleViewModelTest {
    @Test
    fun `parseWeightData returns correct weight`() {
        val viewModel = ScaleViewModel()
        val rawData = byteArrayOf(0x03, 0xCE.toByte(), 0x00, 0x64, 0x00, 0x00, 0xAD.toByte())

        val result = viewModel.parseWeightData(rawData)

        assertEquals(10.0f, result.weight)
        assertTrue(result.isStable)
    }
}
//}

第3章で見たDecent Scaleの7バイトプロトコルのパースが正しいかどうかを、実機もBLEも使わずに検証できます。MVVMでViewModelにロジックを集約しているからこそ、こうしたテストが書きやすくなっています。

=== Composeのプレビューをテストとして使う

先ほど紹介した@<tt>{@Preview}は、手動での目視確認だけでなく、スクリーンショットテストとしても活用できます。Android Studio Pandaでは、Compose Preview Screenshot Testingが導入され、プレビューのスクリーンショットを自動で撮影・比較し、UIの意図しない変更を検出できるようになりました。

State Hoistingでリフトアップされた@<tt>{XXXScreenContent}は、任意の@<tt>{UiState}を渡せるため、さまざまな状態のプレビューを用意しておけば、それがそのままビジュアルテストになります。

//emlist{
@Preview(showBackground = true)
@Composable
fun ScaleScreenPreview_Connected() {
    ScaleScreenContent(
        uiState = ScaleUiState(weight = 10.0f, isStable = true, isConnected = true),
        onTareClick = {}
    )
}

@Preview(showBackground = true)
@Composable
fun ScaleScreenPreview_Disconnected() {
    ScaleScreenContent(
        uiState = ScaleUiState(isConnected = false),
        onTareClick = {}
    )
}
//}

接続時と未接続時の画面を並べて確認できます。

TODO: ScaleScreenPreview_ConnectedとScaleScreenPreview_Disconnectedのプレビューが上下に並んでいるキャプチャ（異なるUiStateで表示が変わることが視覚的にわかる状態）

=== テストに深入りしない理由

テストの手法は本1冊書けるほど奥が深いテーマです。本書の主題はBluetooth機器との連携にあるので、テストについてはここまでとします。ただ、MVVMとState Hoistingという設計パターンを採用したことで「テストが書きやすい構造」にはなっている、ということだけ覚えておいてください。


== この章のまとめ

この章では、Android Studioのインストールから、プロジェクトの作成、Jetpack Composeによる宣言的UI、MVVMアーキテクチャ、テストの基本までを駆け足で見てきました。

振り返ると、いくつかの場面でiOSとの比較が出てきました。開発環境の自由度、UIフレームワークのライブラリ化、実機テストの手軽さ。これらはすべて、第1章で見た「オープンなプラットフォーム」というAndroidの思想から一貫して導かれるものです。

一方で、宣言的UIへの移行はiOSのSwiftUIとまったく同じ方向を向いており、プラットフォームが違っても「良いUI開発とは何か」についての答えは収斂しつつあることもわかりました。

次章では、これらの基礎知識を前提に、Claude Codeを使ってアプリの実装を進めていきます。