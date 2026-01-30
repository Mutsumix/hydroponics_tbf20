= すべてのデータをダッシュボードに反映させよう

@<chapref>{140_sensor}では、センサーの値を取得することができました。

ここまでの集大成として、Seeduinoから取得したデータもまとめてThingSpeakに送信し、Grafanaでダッシュボードを作成する手順を紹介します。

以下の順番で作業を行なっていきます。

 1. Seeeduino Lotusをラズベリーパイに接続する
 1. ThingSpeakのチャンネルを作成する
 1. ラズベリーパイで、Seeeduino Lotusから取得したセンサーデータをThingSpeakに送信するプログラムを実行する
 1. 結果をThingSpeakで確認する
 1. Grafanaのダッシュボードを更新する

== Raspberry Piでシリアルデータを受信する
Seeduinoから取得したセンサーデータをRaspberry Piで受信するために、双方を接続する必要があります。そのためRaspberry PiのUSBポートとSeeeduino LotusのMicro USBポートを接続するケーブルを用意します。

@<chapref>{140_sensor}でPCとSeeduino Lotusを接続する際に使用したケーブルをそのまま使うのが良いでしょう。
Seeduino Lotus への電力供給とデータの受け取りがこれ一本でできます。

//image[seeeduino_lotus_usb_cable][Seeduino LotusとRaspberry Piを接続した様子][scale=0.5]

接続後、Raspberry Pi側で、次のコマンドでデバイス名を確認します。

//cmd{
$ ls /dev/ttyACM*
//}

lsコマンドの結果、このように一覧が出力されます。

//image[ls_result][lsコマンドの結果][scale=0.75]

一般的には @<code>{/dev/ttyACM0} が対象のデバイスです。

次に、シリアル受信できるかを確認します。

//cmd{
$ screen /dev/ttyACM0 115200
//}

「コマンドが見つかりません」というエラーが出た場合は、次のコマンドでscreenをインストールします。

//cmd{
sudo apt update
sudo apt install screen
//}

シリアル受信ができていれば、結果がこのように表示されます。

//image[screen_result][screenの出力結果][scale=0.75]

単に接続しているだけですが、データが送られていることが確認できます。

screenコマンドを終了するには、@<code>{Ctrl+a} → @<code>{k} → @<code>{y} の順にキーを押します。

覚え方

 * Ctrl + A = screenのコマンドモード
 * K = Kill（終了）
 * y = Yes（確認）

次に、このデータをThingSpeakに送信するPythonスクリプトを用意します。

== ThingSpeakに送信する

@<chapref>{120_dashboard} でThingSpeakのアカウントを作成し、チャンネルを作成しました。
そこに新たにSeeeduinoから送信したデータ用のチャンネルを作成します。

//image[thingspeak_channel][Seeeduino用のThingSpeakチャンネルの作成][scale=0.75]

ThingSpeak側の準備ができたら、データ送信のPythonスクリプトを準備します。

こちらのリポジトリにあるPythonスクリプトを使用します。

@<href>{https://github.com/Mutsumix/Seeeduino}

ラズベリーパイ上の任意の場所でGitを使ってクローンします。
ここではデスクトップ上に「Seeeduino」というフォルダを作成して、そこにクローンする想定で進めていきます。

//cmd{
git clone https://github.com/Mutsumix/Seeeduino.git
//}

thingspeak-uploaderフォルダに移動します。

設定ファイル（example.config.yml）をコピーして、config.ymlという名前で保存します。

//cmd{
cp example.config.yml config.yml
//}

config.ymlを開いて、ThingSpeakのAPIキーと、@<code>{ $ ls /dev/ttyACM* } で確認したデバイス名を設定します。

//emlist[config.yml]{
thingspeak_api_key: "YOUR_THINGSPEAK_WRITE_API_KEY"   # ThingSpeakのWrite APIキーを設定
serial_port: "/dev/ttyACM0"  # シリアルポートのデバイス名を設定(ttyACM0でなければ変更)
baud_rate: 115200
send_interval_minutes: 0.25  # データ送信間隔を設定（0.25分 = 15秒）
//}

次に実行に必要なPythonのパッケージをインストールします。

//cmd{
pip install pyserial pyyaml requests
//}

最後に、実行します。

//cmd{
python uploader.py
//}

これでThingSpeakに設定した間隔でデータがアップロードされます。
デフォルトは0.25分（15秒）なので、好みの時間に設定すると良いでしょう。

//image[python_result][Pythonスクリプトの実行結果][scale=0.75]
//image[thingspeak_result][ThingSpeakに送信されたデータ][scale=0.75]

== Grafanaでダッシュボードを作る

ThingSpeakでデータが受信できているのを確認できたら、最後に、Grafanaのダッシュボードにデータを追加します。

@<chap>{120_dashboard} でGrafanaの設定をしていれば、データソースとしてThingSpeakがすでに設定されています。
同じ要領で、Seeeduinoから送信したデータを取得するデータソースを追加します。

最後に、温度・湿度・CO2を設定したときと同様に、各センサーのデータをグラフに設定していきます。

筆者はこのようにダッシュボードを作成しました。

//image[seeeduino-dashboad][Grafanaダッシュボードのサンプル][scale=0.75]

ここはセンスが問われるところですので、自分好みのダッシュボードを作成してください。

== まとめとさらなる活用について

この章では、SeeduinoからRaspberry Piにセンサーデータを送信し、最終的にGrafanaダッシュボードに反映させる手順を紹介しました。

ダッシュボードは作って終わりではなく、ここを起点にさまざまなアラートや自動化を行うことができます。
Grafanaでは閾値を設定した通知機能があります。
気温が一定の温度を超えたらメールやSlackへの通知を行う、といったことができます。

さらに、SwitchBot APIを使えば指ロボットや、電球などの家電製品を自動で操作することができます。
また、上級者向けですが、赤外線コマンドをAPIで操作することで、赤外線リモコンの機能を再現することもできます。
それを活かして、例えば次のようなことができます。

 * 照度が下がったら、LEDの電球をつける
 * 温度が30°Cを超えたら、ファンをつける

アイディア次第でさまざまな自動化が可能です。
また、Seeeduinoで今回紹介していないようなセンサー、土壌水分量やEC値、PC値を測定できるものを試すのも面白いと思います。