= 簡単な監視を始めよう

この章では手始めに簡単に購入できる機器を使ったデータ収集と監視を紹介します。

プログラミングは必要ありません。そもそもパソコンすら不要です。

概要を図で示すと次のようになります。

//image[switchbot_monitoring_system.drawio][手軽な監視の概要図][scale=0.75]

早速、購入品一式と設置方法を見ていきましょう。

== 購入品一式

//table[start_device][購入品一式]{
品目	個数	購入先	目安の価格
------------------
SwitchBot CO2センサー（温湿度計）@<fn>{switchbot_co2_sensor}	1つ	公式ネットショップ	¥7,980
SwitchBot 屋内カメラ@<fn>{switchbot_indoor_camera}	1つ	公式ネットショップ	¥3,780
SwitchBot ハブミニ@<fn>{switchbot_hub_mini}	1つ	公式ネットショップ	¥5,480
水温計@<fn>{water_temperature_sensor}	2つ	Amazon	¥585
//}

//image[switchbot_co2_sensor][CO2センサー][scale=0.25]
//image[switchbot_camera][屋内カメラ][scale=0.25]
//image[switchbot_hub_mini][ハブミニ][scale=0.25]

//footnote[switchbot_co2_sensor][@<href>{https://www.switchbot.jp/products/switchbot-co2-meter} SwitchBot CO2センサー（温湿度計）]
//footnote[switchbot_indoor_camera][@<href>{https://www.switchbot.jp/products/switchbot-indoor-cam} SwitchBot 屋内カメラ]
//footnote[switchbot_hub_mini][@<href>{https://www.switchbot.jp/products/switchbot-hub-mini} SwitchBot ハブミニ]
//footnote[water_temperature_sensor][@<href>{https://amzn.asia/d/3HV4weq} 水温計 購入できる最小単位が2個セットでした]


=== SwitchBot製品の選定理由について

==== 安さ
まず屋内カメラについては、同機能を持つ屋内用ネットワークカメラの中では比較的安い部類に入ります。

==== 調達の容易さ
紹介したSwitchBot の製品はどれも公式サイトやAmazonなどネット上で簡単に購入ができます。
スマートホーム機器を取り揃えているメーカーのうち、SwithBotは有名どころでラインナップも豊富なため、ここで紹介した製品が長い期間にわたって調達が可能という点でオススメしています。

==== 拡張性
@<chap>{120_dashboard} @<title>{120_dashboard} の章で後述しますが、CO2センサー（温湿度計）はAPIを使ってプログラムからセンサーデータを取得できるので、様々な形でのデータ利活用が可能です。

==== 注意点
気をつけておきたいのが、経験上、SwitchBotの製品は初期不良や故障が割とあります。
保証期間内であればしっかり対応が受けられます。
何か動きがおかしいな、と思ったらメールからでもアプリ上からでも問い合わせができるようになっていますので、迷わずカスタマーサポートに連絡することをお勧めします。
人によってはサポート窓口に問い合わせをするための心理的ハードルが高いかもしれません。
筆者もそうでしたが、SwitchBotの製品を家に導入し始めてから、サポートに何度も問い合わせをするようになり、そのうちに心理的ハードルがかなり下がりました。

=== ハブについて

ハブに関しては、ハブミニ以外にも、ハブ2、ハブ3、さらに温湿度計やスマートリモコン、あるいは照明と一体になった製品も販売されています。

どの製品でも大丈夫なので、ご自宅に必要だと思ったハブ機能を持つ製品を選んでください。ハブに接続することで、遠隔で計測データの確認ができ、後の章で触れるAPI経由での制御も可能になります。

== 設置

必要な機器が揃ったら、設置をしていきましょう。

CO2センサー（温湿度計）、屋内カメラ、ハブミニのケーブルを電源に繋ぎます。

SwtichBotのマニュアルに従ってネット接続の初期設定を行い、ハブに接続します（ハブミニはWi-Fiに接続させる必要があります）。@<fn>{hub_connection}

詳細な手順は、機器のバージョンによって変わるので、機器購入時のマニュアルやSwitchBotの公式サイトを参照してください。@<fn>{switchbot_manual}

//footnote[switchbot_manual][@<href>{https://support.switch-bot.com/hc/ja/articles/20426539496727-%E6%B8%A9%E6%B9%BF%E5%BA%A6%E8%A8%88%E6%B8%A9%E6%B9%BF%E5%BA%A6%E3%83%87%E3%83%BC%E3%82%BF%E3%81%AE%E9%81%A0%E9%9A%94%E7%A2%BA%E8%AA%8D%E6%96%B9%E6%B3%95-%E3%83%8F%E3%83%96%E3%81%AB%E6%8E%A5%E7%B6%9A%E3%81%99%E3%82%8B%E6%96%B9%E6%B3%95} SwitchBot ハブミニ マニュアル]
//footnote[hub_connection][2025年11月現在の手順では、アプリ→温湿度計グラフページ下にある「データをクラウドに保存」ボタンを手動でタップする必要があります。そうすれば、ハブに接続できが可能になります。]

温度測定のための水温計ですが、外気温を測るのにも使えますし、土の中に入れて使用することもできます。

先端部は防水仕様になっているので、水の中に入れたり、土の中に入れて温度を測ることができます。

//image[water_temperature_sensor][水温計][scale=0.75]

温度表示の液晶部分がカメラの画角に収まるようにしましょう。

設置については以上です。

== 観察

SwitchBotのアプリ上から、温度・湿度・CO2濃度のグラフを確認でき、またカメラの映像から植物の様子や水温計に表示された温度もリアルタイムでわかります。

//image[app_graph][アプリ上でのグラフ表示][scale=0.55]

//image[camera_view][カメラの映像][scale=0.75]

ハブに接続していれば、外出先からでもリアルタイムのセンサやカメラの情報を得ることができます。

ここまでは、手軽な監視を行う方法について紹介しました。次章から、徐々に発展的な監視を行なっていきます。
