PC-6001mk2_SD(https://github.com/yanataka60/PC-6001mk2_SD)互換機能のファームウェアです。

オリジナルのPC-6001mk2_SDとの違い
* SDカードの代わりにUSBメモリを使用します。(ホットスワップは出来ません)
* Fコマンドがワイルドカード(*と?)に対応。*は0文字以上の任意の文字、?は任意の1文字にマッチします。
例
a*	aから始まるファイル全て
*.cas	casファイル全て
a*.cas	aから始まるcasファイル全て
a*.???	aから始まる拡張子が3文字のファイル全て
(大文字小文字は区別しません)

使用法

1. mk2sd.binファイルの後ろにEXT_ROM.binを連結します。

EXT_ROM.binは添付のもの(キー入力絡みの不具合修正済)か以下のオリジナルのものが使用可能です。
https://github.com/yanataka60/PC-6001mk2_SD/blob/main/Z80/EXT_ROM.bin

unixのcatコマンドを使用する例
$ cat mk2sd.bin EXT_ROM.bin > firmware.bin

windowsのcopyコマンドを使用する例
>copy /b mk2sd.bin + EXT_ROM.bin firmware.bin

2. uf2conv.exeを用いて1で作成したfirmware.binをuf2ファイルに変換します。

例: windowsコマンドプロンプト上で
uf2conv.exe firmware.bin firmware.uf2

uf2conv.exeはhttps://github.com/piigaa-densetu-two-dai/SUMOにあります。

3. P6本体に刺さっていない状態の力士のカートリッジをブートモードでPCに接続します。

力士のカートリッジのBOOTSELボタンを押しながらPCとUSB接続して下さい。
接続が成功するとドライブが認識されます。

4. 力士のカートリッジに2で作成したfirmware.uf2ファイルを書き込みします。

3で認識されたドライブにfirmware.uf2をドラッグアンドドロップ(コピー)します。
コピーが完了するとドライブが見えなくなります。

5. 書込みが終わったらPCから外します。

注意：PCとの接続中はバスバッファの入力がフロート状態となります。
あまり良い状態ではないので長時間のPC接続は避けてください。

6. 力士のカートリッジにUSBメモリを取り付けて完了

USBメモリはFAT32でフォーマットして下さい。
ディスクラベルの無いUSBメモリ(直にFAT32フォーマットされた物)は問題があるかもしれません。
USB変換アダプター(OTGアダプター)が必要です。100円ショップ等で入手して下さい。

RESETスイッチ設定
ONにしてください(何か問題が起きる場合はOFFにして下さい。コールドスタート時に不具合が起きる場合はP6本体のリセットボタンを押してリセットして下さい)

RESETスイッチについて
P6本体のリセットと力士のカートリッジのリセットを連動させるか否かの設定スイッチです。
