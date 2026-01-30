= ダッシュボードを作ってみよう

@<chapref>{110_start} では、SwtichBotの製品とアプリを使ったセンサーデータの確認を行いました。

この章では、センサーデータをThingSpeak（シングスピーク）というクラウドサービスに保存し、Grafana（グラファナ）というツールを使って自分好みのダッシュボードを作成する方法について紹介します。

自分好みのダッシュボードを作成する理由ですが、自分が知りたいデータを自分の知りたいレイアウトや表示する期間に設定することで、育成している植物の生育状況の把握がしやすくなります。

まずは概要を説明します。用意する機器は、@<chap>{110_start} で紹介したSwitchBotのCO2センサーとハブミニです。追加で必要な機材はありません。

強いて言えば、これから説明する操作を実行するためのPCが必要です。WindowsでもMacでもLinuxでも、OSは問いません。

センサーの値を取得するために実行するPythonのプログラムは、AWSというクラウドベンダが提供しているLambdaというサービスを使って動かします。

全体の構成図を以下に示します。

//image[system_overview.drawio][全体の構成図]

それでは見ていきましょう。

== API連携の準備、各種アカウントの作成

プログラムを使ってセンサーの値をモニタリングするためには、APIとの連携準備が必要です。@<fn>{api}

次の作業を行なっていきます。

//footnote[api][APIとは、アプリケーションプログラミングインターフェースの略で、アプリケーションやプログラム同士を接続・連携させるためのコンセントのようなものです。このコンセントの穴があることでアプリケーションやプログラムは、お互いにデータを送り合うことができるのです。このAPIを作るかどうかはサービス提供者側が第三者の利用を期待するかどうかで変わります。当然APIを作っておらず、自分の世界に閉じたプログラムはありますし、むしろそういったプログラムの方が圧倒的多数だと思います。しかし、このAPIの仕組みを作成し・公開することで、アプリケーションは世界中のユーザーに開かれ、これから生まれるアプリの可能性を拡張することができるのです。]

 1. SwitchBotの認証用トークンの取得
 1. AWSのアカウント作成
 1. ThingSpeakのアカウント作成
 1. ThingSpeakのAPIキーの取得

まずはSwitchBotをAPIから操作する方法を説明します。

SwitchBotは、SwitchBot ハブを介してCO2センサーがインターネットに接続されていることを前提としています。

=== SwitchBot 認証用トークンの取得

API認証用のトークンを取得します。
SwitchBotアプリの 設定 > 開発者向けオプション から確認可能です。
開発者向けオプション は、設定 > 基本データ の アプリバージョン を10回ほどタップすると表示されるようになります。

//image[app_version][アプリバージョンを10回ほどタップ][scale=0.5]
//image[app_version_detail][開発者向けオプションが表示される][scale=0.25]
//image[app_version_detail_2][開発者向けオプションからトークン情報が確認可能][scale=0.25]

==== SwitchBot クラウドサービス設定（現在は不要な設定です）

APIから操作するために、SwitchBotの「クラウドサービス」をオンにしてください。（SwitchBotアプリV9.0から、クラウドサービスの項目自体が削除されていて、手動でオンにする必要がなくなりました）@<fn>{switchbot_cloud_service}

==== SwitchBot デバイスIDの取得

続いて、SwitchBotの deviceId を取得します。
コマンド内のAuthorization には、先ほど開発者向けオプションで確認した認証用トークンを指定してください。

//emlist[curlコマンドでデバイス一覧を取得する][curlコマンドでデバイス一覧を取得する]{
curl --request GET 'https://api.switch-bot.com/v1.0/devices' \
  --header 'Authorization: 認証用トークン' \
  --header 'Content-Type: application/json; charset=utf8'
//}

下記のようなレスポンスが返ってきます。

//emlist[curlコマンドで取得したデバイス一覧の例][curlコマンドで取得したデバイス一覧]{
{
    "statusCode": 100,
    "body": {
        "deviceList": [
            {
                "deviceId": "A1234567890Z",
                "deviceName": "ハブ",
                "deviceType": "Hub Mini",
                "enableCloudService": true,
                "hubDeviceId": "000000000000"
            },
            {
                "deviceId": "A9876543210Z",
                "deviceName": "CO2センサー",
                "deviceType": "MeterPro(CO2)",
                "enableCloudService": true,
                "hubDeviceId": "000000000000"
            }
        ],
        "infraredRemoteList": []
    },
    "message": "success"
}
//}

ここで取得したデバイス一覧の中から、CO2センサーの deviceId を確認してください。この deviceId を後ほどLambdaの環境変数に設定します。

SwitchBotのAPIドキュメントの詳細はGitHubから確認できます。@<fn>{switchbot_api}

//footnote[switchbot_cloud_service][@<href>{https://blog.switchbot.jp/announcement/switchbot-app-9-0/}]
//footnote[switchbot_api][@<href>{https://github.com/OpenWonderLabs/SwitchBotAPI#switchbotapi} SwitchBot API]

=== プログラム実行前の準備（ThingSpeak側）

プログラムを実行するには、ThingSpeakのチャンネルIDの設定とAPIキーの取得が必要になるので、ThingSpeakアカウントの作成手順からチャンネルの設定までを説明します。

データの受け手となるThingSpeak（@<href>{https://thingspeak.mathworks.com/}）のアカウントを作成します。

//image[thingspeak][ThingSpeakのトップ画面][scale=0.75]

登録にはメールアドレスが必要なので、登録と認証を済ませます。

//image[thingspeak_signup][アカウント登録画面][scale=0.75]

登録が完了すると、マイページが表示されるので「New Channel」をクリックします。

//image[thingspeak_new_channel][チャンネル作成ボタン][scale=0.75]

チャンネル名と、どういった値を受信するのかを設定する必要があるので、次のように入力します。

//image[thingspeak_channel_settings][チャンネル設定画面][scale=0.75]

最後に、「Save Channel」をクリックします。

//image[thingspeak_channel_save][チャンネル保存ボタン][scale=0.75]

チャンネルが作成されると、各種タブが表示されるので、「API Keys」タブをクリックし、Write API Key とRead API Key が存在するのを確認します。

//image[thingspeak_api_keys][API Keysタブ][scale=0.75]
//image[thingspeak_api_keys_values][Read API KeysとWrite API Keyの値][scale=0.75]

これで、AWSのLambdaでプログラムを実行し、ThingSpeakにデータを送信する準備が整いました。


=== プログラム実行前の準備（AWS側）

AWSアカウントの作成ですが、こちらについては詳細には説明しません。@<fn>{aws_no_detail}公式のドキュメントを参照してください。@<fn>{aws_account_setup}

//footnote[aws_no_detail][AWSに限らずですが、クラウドサービスはしょっちゅうUIやボタン位置が変わって、画面のキャプチャを掲載しても数年後には役に立たない、ということが主な理由です。]
//footnote[aws_account_setup][@<href>{https://aws.amazon.com/jp/register-flow/} AWSアカウントの作成方法]

途中カード情報の入力が求められます。本書の使用範囲であれば、無料枠で収まるので特に心配はいりません。ただし、アカウント情報の漏洩には注意してください。

コンソールと呼ばれる設定画面が表示されれば、アカウントの作成は完了です。

//image[aws_console][コンソールの画面][scale=0.75]



== センサー値取得プログラムの作成

それでは、CO2センサーから一定間隔でセンター値を取得するプログラムを作成していきます。

手順は次のとおりです。

 1. GitHubから監視プログラムのソースコードを取得する
 1. ソースコード内のzipファイルをAWSのLambdaにアップロードする
 1. 環境変数を設定する
 1. テストを実行し、結果を確認する
 1. AWSのEventBridgeで定期的に実行するように設定する
 1. Grafanaのダッシュボードを作成する

それでは順を追って説明します。

=== GitHubからソースコードの取得

GitHubからプログラムのソースコードを取得します。

著者のリポジトリにアクセスしてください。@<href>{https://github.com/Mutsumix/switchbot-co2-thingspeak} 

アクセスした先の画面で、「Code」ボタンを選択し「Download ZIP」を選択して、zipファイルをダウンロードして、解凍します。

//image[github_download_zip][GitHubからzipファイルをダウンロードする][scale=0.75]
//image[unzip_lambda][zipファイルを解凍する][scale=0.75]

zipファイルを解凍すると、中にlambda.zipというファイルがあるので、これをAWSのLambdaにアップロードします。

使うのはこのzipファイルだけですが、もし内部に興味のある人がいたら、ソースコードも中に入っていますので覗いてみてください。

GitやGitHubに慣れている人はクローンしてください、慣れているあなたには手順なんて不要です。


=== Lambdaの設定

続いてAWSの画面に戻ってLambdaの設定をしていきます。@<fn>{aws_lambda}

//footnote[aws_lambda][@<href>{Lambda（ラムダ）とはAWSのサービスの一つで、サーバーレスコンピューティングサービスです。簡単に言うと、サーバーの管理や設定をすることなく、プログラムコードを実行できるサービスです。必要な時だけ動作し、使った分だけ料金が発生するので、定期的なデータ取得のような用途に最適です。}]

検索画面に「Lambda」と入れて、トップに表示される「Lambda」を選択します。
//image[aws_lambda_search][AWSのLambdaの検索画面][scale=0.75]

==== 関数の作成

「関数の作成」ボタンを選択します。

//image[aws_lambda_create_function][関数の作成画面][scale=0.75]

関数の作成画面で次のように各種設定します。

 * 一から作成を選択
 * 関数名（任意）：switchbot-co2-monitor
 * ランタイム：Pythonの最新版を選択してください（3.9, 3.11で動作確認しています）

 「関数を作成」ボタンを選択します。

//image[aws_lambda_create_function_2][関数の設定画面][scale=0.75]

==== パッケージのアップロード

問題なく関数が作成されると、Lambda関数の設定画面に遷移します。

画面内のコードソースのエリアの右上にある「アップロード元」ボタンを選択し、「.zipファイル」を選択します。

//image[aws_lambda_upload_package][パッケージのアップロード][scale=0.75]

アップロードのポップアップが表示されるので、GitHubからダウンロードしたzipファイルをアップロードし、「保存」ボタンを選択します。

//image[aws_lambda_upload_package_2][GitHubからダウンロードしたzipファイルをアップロードする][scale=0.75]

==== 環境変数の設定

次に、このアップロードしたプログラムが、あなたのSwitchBotの情報を参照し、ThingSpeakにデータを追加できるように環境変数を設定します。

Lambda関数の設定画面の「環境変数」タブを選択し、「環境変数」の項目を選択し、「編集」ボタンを選択します。

//image[aws_lambda_environment_variables][環境変数の設定][scale=0.75]

環境変数の設定画面で、次のように設定し、「保存」ボタンを選択します。

 * SWITCHBOT_TOKEN：SwitchBotの開発者向けオプションで「トークン」の項目で確認可能
 * SWITCHBOT_SECRET：SwitchBotの開発者向けオプションで「クライアントシークレット」の項目で確認可能
 * SWITCHBOT_DEVICE_ID：Curlコマンドで取得したCO2センサーの deviceId
 * THINGSPEAK_API_KEY：ThingSpeakのWrite API Key

//image[aws_lambda_environment_variables_2][環境変数の設定値][scale=0.75]

==== テストの実行

ここまでできたらテストを実行しましょう

Lambda関数の設定画面の「テスト」タブを選択し、「テスト」ボタンを選択します。

//image[aws_lambda_test][テストの実行][scale=0.75]

画面に「成功」と出たらOKです。

もしそれ以外の結果が出たら、ここまでの手順を見直してください。

//image[aws_lambda_test_result][テストの実行結果（詳細を広げた状態）][scale=0.75]

結果の確認のためにThingSpeakの画面も見にいきます。

ダッシュボードから「SwitchBotSensor」を選択し、「サービスメトリック」のタブを選択します。

SwitchBotから取得したデータが送られていることを確認します。

// image[mackerel_service_metrics][サービスメトリックの画面][scale=0.75]

=== 定期実行の設定

SwitchBotのデータを取得できるようになりましたが、今のままだと、テストを実行した時にしかデータが送られません。

このプログラムが定期的に動くように設定しましょう。

==== AWSのEventBridgeの設定

AWSのEventBridgeを使って、Lambdaを定期的に実行するように設定します。

AWSの検索画面に「EventBridge」と入力して、トップに表示される「Amazon EventBridge」を選択します。

//image[aws_eventbridge_search][AWSのEventBridgeの検索画面][scale=0.75]

左側のメニューから「スケジュール」を選択し、「スケジュールを作成」ボタンを選択します。

//image[aws_eventbridge_create_schedule][スケジュールの作成][scale=0.75]

スケジュールの作成画面で次のように設定します。

 * スケジュール名：switchbot-co2-monitor
 * スケジュールのパターン：定期的なスケジュール
 * スケジュールの種類：rateベースのスケジュール

 30分ごとに実行するように、rate式には次のように設定します。

 * 値：30分
 * 単位：minutes

 フレックスタイムウィンドウは、「オフ」

//image[aws_eventbridge_create_schedule_3][フレックスタイムウィンドウの設定][scale=0.75]

 「次へ」ボタンを選択します。

 ターゲットの詳細を設定する必要があるので、次の通りに設定します。

 * ターゲットの種類：Lambda関数
 * 関数名：switchbot-co2-monitor

//image[aws_eventbridge_create_schedule_4][ターゲットの詳細の設定][scale=0.75]

「次へ」ボタンを選択します。

オプションの設定画面になりますが、特に設定する必要はありません。

再度「次へ」ボタンを選択し、確認画面に遷移し「スケジュールを作成」ボタンを選択します。

以上で、定期実行の設定は完了です。

これで30分に一回、このプログラムが実行される設定が完了しました。

=== Grafanaでダッシュボードを作成する
これでセンサーの値をWeb上で確認できるようになりましたが、筆者はGrafanaにダッシュボードを作成しています。
こうすれば、視覚的に今どうなっているのか把握しやすいですし、外出先でも簡単に生育環境の状況を確認することができます。

1. Grafana Cloudアカウントの作成:

Grafana Cloudのウェブサイト（@<href>{https://grafana.com/products/cloud/}）にアクセスします。

「Create free account」をクリックし、指示にしたがってアカウントを作成します。ソーシャルログインに対応しているので簡単にログインが可能です。

//image[grafana_cloud_signup][Grafana Cloudのアカウント登録画面][scale=0.75]

アカウント作成後、セットアップ画面が表示されます。右上の「I'm already familiar with Grafana〜」をクリックし、この画面はスキップします。

//image[grafana_cloud_setup][セットアップ画面][scale=0.75]

2. Connectionの追加

「Add new connection」と画面に表示されるのを確認します。もし表示されていなければ左側のメニューから「Connections」> 「add new connection」を選択します。

#@# 「Add data source」をクリックします。？

検索バーで 「JSON」 と検索し、「JSON API」を選択します。

//image[grafana_cloud_add_json_api][JSON APIの選択画面][scale=0.75]

「install」を選択します。

//image[grafana_cloud_install_json_api][JSON APIのインストール画面][scale=0.75]

3. Data Sourceの追加

左側のメニューから「Connections」> 「Data sources」を選択します。

//image[grafana_cloud_data_sources][Data sources画面][scale=0.75]

「Add new data source」をクリックし、先ほどインストールしたJSON APIを選択します。

//image[grafana_cloud_add_data_source][Data sourceの追加画面][scale=0.75]
//image[grafana_cloud_data_source_select][JSON APIの選択][scale=0.75]

次の設定を行います。

 * Name: ThingSpeak（任意の名前です）
 * URL: https://api.thingspeak.com/channels/@<b>{ID}/feeds.json?api_key=@<b>{API} @<fn>{thingspeak_api_url}

//image[grafana_cloud_data_source_settings][Data sourceの設定画面][scale=0.75]

 * ここまでの設定に問題がなければ「Save & Test」をクリックすると、画面上にSuccessと表示され、設定が保存されます。

//image[grafana_cloud_data_source_success][Data sourceの設定成功画面][scale=0.75]

//footnote[thingspeak_api_url][ID と API はThingSpeakのチャンネルIDとRead API Keyにそれぞれ置き換えてください]

4. ダッシュボードの作成:

左側のメニューから「Dashboards」を選択します。

//image[grafana_cloud_create_dashboard][Dashboardの作成画面][scale=0.75]

「New dashboard」をクリックします。

//image[grafana_cloud_add_dashboard][Dashboardの追加画面][scale=0.75]

「Add visualization」をクリックします。

//image[grafana_cloud_add_visualization][Visualizationの追加画面][scale=0.75]

データソースを聞かれるので、先ほど作成したThingSpeakを選択するために検索窓に「thing」と入力し、「ThingSpeak」を選択します。

//image[grafana_cloud_select_data_source][Data Sourceの選択画面][scale=0.75]

データソースの詳細設定を行う画面が表示されるので、設定を行います。

次の２つを設定します（@<list>{grafana_cloud_data_source_settings_fields}）。

//list[grafana_cloud_data_source_settings_fields][Data Sourceの設定画面][scale=0.75]{
$. feeds [*].created_at
$. feeds [*].field1
//}

また、型[Type]を正しく設定する必要があるため、次のように設定します（@<list>{grafana_cloud_data_source_settings_fields_type}）。

//list[grafana_cloud_data_source_settings_fields_type][Data Sourceの設定画面][scale=0.75]{
$. feeds [*].created_at -> Time
$. feeds [*].field1 -> Number
//}

//image[grafana_cloud_data_source_settings2][Data Sourceの設定画面][scale=0.75]

すると画面上に取得した数値が折れ線グラフで表示されます。

あとは、パネルのTitleを適切なものに変更します。Field1の場合は、気温のため「Temperature」もしくは「気温の推移」というように変更します。

Save Dashboardをクリックしてダッシュボードを保存します。

同様の手順で、他のセンサーデータについても取得できるようにFiels2, Filed3のパネルを作成します。

Grafanaでは表示形式について色々と設定できます。

参考までに筆者はこちらの図のようなダッシュボードを作成しています。

//image[grafana_cloud_dashboard][ダッシュボードの作成例][scale=0.75]

== まとめ

ここまでで、SwitchBotのCO2センサーの値をThingSpeakに送信し、Grafanaでダッシュボードを作成するところまでを説明しました。

このように、AWSのLambdaを使うことで簡単にセンサーの値を取得し、SwitchBotのアプリで見るよりも見やすく自由度の高いダッシュボードを作成することができました。


#@# ==== 思い通りのダッシュボードを作成できる
#@# アプリでの値の確認方法ですが、期間や表示形式はアプリ提供者の方法に従うしかありません。
#@# またアプリで確認すると若干遅い、という点がデメリットです。
#@# Mackerelでダッシュボードを自由に作成しブラウザにブックマークすることで、これらのデメリットを解消することができ、監視の自由度が高まります。

#@# == おまけ（カメラによる監視）

#@# 筆者はネットワークカメラにもSwitchBotの製品を使っていますが、ネットワークカメラを使うことで、リアルタイムの生育状況を出先から確認することができます。

#@# //image[switchbot_camera][SwitchBotのネットワークカメラで撮影した画像][scale=0.75]

#@# このカメラで撮った写真を定期的にGooglePhotoに送ることができれば、簡単にタイムラプス動画が作成できるのに、と思っているのですが、うまい方法をまだ見つけられていません。今後の課題です。
