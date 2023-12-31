# comp

コンパイラ実装の素振り。というよりかは、C 言語のことやアセンブリのことが知りたくて書いているのかも。
[低レイヤを知りたい人のための C コンパイラ作成入門](https://www.sigbus.info/compilerbook)
に沿って実装をしてみる。

## 今できること

`test.sh` を参照。

- 2023-07-19: 関数を定義・呼び出しできるようになった
  - `gcc`など既存のコンパイラでビルドしたオブジェクトコードとリンクすれば、
    その関数を呼び出すプログラムをコンパイルできる
  - 自分で定義した関数も呼べる
- 2023-07-22: ポインタをlvalueとして使えるようになった
  - calleeにポインタを渡してそこに計算結果を書き込んでもらう、みたいなことができる
- 2023-07-22: 型検査を実装した
  - 演算子オーバーロドのために実装した
  - ポインタ算術で欲しくなる

## 参考ドキュメントからズレるところ

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

## レジスタサイズをまじめに指定する

式や変数の方に応じて適切なレジスタやスタック領域を使用したい。

### 現状

現状では全て8byteのレジスタを使っているし、一時的な値はスタック領域の16byte境界に積んでいる。
ローカル変数はスタックにまじめに（変数のサイズが全て8byteなので簡単）割り当てている

### 変更後

## issues

- 汎用レジスタを Xn で固定している。それに依存する形でスタックフレームの確保を行うようにハードコードしている。
- issueをここに書いている
  - 増えてきたらgithubに移そう
- local変数のスタック上での配置がテキトー
- ブロックスコープを未実装
  - 関数スコープだけしか実装してない。宣言はそれっぽいところでできる

一旦C風なオレオレ言語を作って、ある程度できたら何かの標準に従う。

## 設計メモ

### 変数の管理

#### スコープとextent

ブロック変数のextentは関数呼び出しの間とする。
つまり、ローカル変数は関数ごとにスタックに割り付ける。
ブロックスコープのことはスタック割り付けの時点では考慮しない。
全てのブロックで宣言した変数は、関数本体のトップレベルで宣言された変数と同様にスタックに割り付ける。

スタックに割り付ける変数をlocalsみたいな名前で保持することにして、
スコープはenvみたいな名前の変数/構造体で管理する。envでは名前とlocalsとの対応づけを行う。
localsは関数定義が所有して、envはスコープの区切り目のそれぞれに設定する。
localsというよりlocationみたいな名前の方が良かった？

#### 型とサイズとオフセット

変数の型は型推論のタイミングで決定したいけど、
現状では構文解析のタイミングでlocalsに型を代入してしまっている。
これは単に実装をサボっているから。

サイズとフレームポインタからのオフセットの計算は型推論のあと、コード生成の前に行う。
フレームポインタからのオフセットはlocalsが保持する。
