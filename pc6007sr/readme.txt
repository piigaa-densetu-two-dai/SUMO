PC-6007SR(64K RAM + 拡張漢字ROM)のファームウェアです。
初代PC6001は非対応です使用できません

使用法

1. pc6007sr.binファイルの後ろに拡張漢字ROMファイル(128K)を連結します。

unixのcatコマンドを使用する例
$ cat pc6007sr.bin kanji1.rom > firmware.bin

windowsのcopyコマンドを使用する例
>copy /b pc6007sr.bin + kanji1.rom firmware.bin

拡張漢字ROMファイルについて
PC-8801mk2シリーズ(SR以降?)の第一水準漢字ROMファイルが流用出来るようです。
SHA1(kanji1.rom)= 82e11a177af6a5091dd67f50a2f4bafda84d6556

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
ONにして下さい(何か問題が起きる場合はOFFにして下さい。コールドスタート時に不具合が起きる場合はP6本体のリセットボタンを押してリセットして下さい)

RESETスイッチについて
P6本体のリセットと力士のカートリッジのリセットを連動させるか否かの設定スイッチです。
