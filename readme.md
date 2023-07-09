# comp

コンパイラ実装の素振り。というよりかは、C言語のことやアセンブリのことが知りたくて書いている。
[低レイヤを知りたい人のための C コンパイラ作成入門](https://www.sigbus.info/compilerbook)に沿って実装をしてみる。

## 今できること

`test.sh` を参照。

`return` を実装したけど関数を実装していない。
とりあえず、入力したプログラムはmain関数の本体とみなすような感じでコンパイルしていると思えば良い。

## ズレるところ

- x86-64 ではなくて、arm64 をターゲットにする
  - mac で開発しているので。
  - コンテナとか使うの面倒だし
  - あとは純粋な興味
- pprint を作ってパーサが合ってるか確認
  - 割と実装の細かいところは追っていなくて、自分で思いついたのをノリで採用している。
    困ったり迷ったら参考にみに行くけど、デバッグは自力でしないといけない。
  - `make print ARGS="program"` で program にカッコをつけて表示する
  - lexer 向けにも作りかけているけど、デバッグに欲しくならなかったので中途半端
- 連結リストの作り方が違う。書籍では先頭要素をグローバル変数に持って、末尾を伸ばすのに対して、
  この実装ではグローバル変数を持たずに関数に既存のリストを渡して拡張したリストを返すようにする。
    - 環境っぽさはこっちの方がありそう 
- このくらいのマイナーな違いがゴロゴロある。

## issues

- 汎用レジスタをXnで固定している。それに依存する形でスタックフレームの確保を行うようにハードコードしている。
