PC-6006(16K RAM + ROM)のファームウェアです。

使用法

1. pc6006.binファイルの後ろに使用するROMファイル(16K以下)を連結します。

unixのcatコマンドを使用する例
$ cat pc6006.bin game.rom > firmware.bin

windowsのcopyコマンドを使用する例
>copy /b pc6006.bin + game.rom firmware.bin

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

5. 書込みが終わったらPCから外して完了

注意：PCとの接続中はバスバッファの入力がフロート状態となります。
あまり良い状態ではないので長時間のPC接続は避けてください。

RESETスイッチ設定
初代PC6001: OFFにして下さい(コールドスタート時にメモリがBASICから認識されない場合はP6本体のリセットボタンを押してリセットして下さい)
それ以外の機種: ONにして下さい(何か問題が起きる場合は初代PC6001と同様の設定にしてみてください)

RESETスイッチについて
P6本体のリセットと力士のカートリッジのリセットを連動させるか否かの設定スイッチです。
