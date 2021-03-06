mods(modest starter)
==========

# 寡黙なスターター（ランチャー）
自分自身の次の要望に応えるために作成しました。

1. ノートPCという限られた大きさの画面で作業するので、常に表示場所を取るランチャーを起動しておくのは邪魔だ
2. デスクトップ環境の用意しているメニューをカスタマイズしまくるのは嫌だ
3. Linuxのデスクトップに、アイコンを置きたくない
4. XWindowアプリを作ってみたい
5. 机以外でも作業するので、マウスに触らずにプログラムを起動したい

modsの何が控えめ(modest)なのでしょうか?

1. 常駐するなんてだいそれた事は考えていません
2. 起動しても、フォーカスを失えば終了します
3. ウィンドウ・タイトルも、メニュー・バーもありません
4. 独自に表示色を指定していません
5. 機能もシンプルです
6. タスク・バーにも載りません

この控えめさは、ノートPCで使うには魅力なのではないでしょうか?フォーカスを失えば終了する、というのが実現できた時に喜びを感じました。
常駐しない代わりに、便利に使うには何らかの形で mods を起動してもらわなければならないのですが、それはキーバインディングを処理するデスクトップ環境の機能等に委ねる事になります。lxde の場合は lxde-rc.xml に登録しておくなどです。

mods は引数で渡されたテキスト形式のメニュー定義ファイルを元に、簡単なリスト形式のメニューを表示して選ばれたものを起動する、ただそれだけの機能です。
私は自作のawin(https://github.com/takatak/awin)と共に使うことで、コマンドの2重起動を防いでいます。


# Install

## 事前準備

このツールはC言語のソース形式で公開しており、CMake というツールを使って Makefile を作成し、make し インストールする必要があります。次は debian での例を示します。ディストリビューションに合わせて適宜対応をおねがいします。C言語の開発が出来る状態にない場合、ディストリビューションに合わせて事前に導入しておいてください。「Makefile 位自分で書く」という方は CMake は不要です。

### CMake
      sudo apt-get install cmake
  

### libgtk-3-dev (libgtk-2-dev を使う場合、src/CMakeLists.txt を書き換える必要があるでしょう)
      sudo apt-get install libgtk-3-dev


## 本体のインストール

### Makefile 生成

build サブ・ディレクトリに移動して、「cmake ../」を実行


### make

build サブ・ディレクトリで、「make」を実行


### インストール

build サブ・ディレクトリで、「sudo make install」を実行
(または「sudo make install/strip」) 



# 使い方

## テキスト・エディタでメニュー定義ファイルを記述します
### 基本的なサンプル
`------------------------<<<次の行から>>>---------------------------------------`
<br>
`# sample`<br>
`text="edit .bashrc"          ;cmd="leafpad ~/.bashrc";`<br>
`text="edit lxde-rc.xml(user)";cmd="leafpad ~/.config/openbox/lxde-rc.xml";`<br>
`text="edit test.mnu"         ;cmd="leafpad ~/dev/gtk/test.mnu";`<br>
`text="edit fstab"            ;cmd="gksudo leafpad /etc/fstab";`<br>
`------------------------<<<上の行まで>>>---------------------------------------`

1. 「#」 で始まる行はコメント行です。
2. 「text=」の後に、ダブル・クォーテーションで囲んで、リスト表示させる内容を記述します。
3. 「cmd=」の後に、ダブル・クォーテーションで囲んで、対応するコマンドを記述します。
4. コマンドはリストに表示はされません。
5. 対となる「text=」と「cmd=」は同一行に記述し、その間は、「;」で区切ります。
6. 「text」と「=」の間、「cmd」と「=」の間に空白文字等は含める事はできません。
7. 「text=」と「cmd=」は必ず小文字で記述します。
8. 上記のサンプルでは、行末に「;」を記していますが、これは必須ではありません。
9. 記述には基本的にはシングル・バイト文字を使いましょう。
10. コード変換はしていませんが、UTF-8環境であれば日本語表示は出来ています。
11. 登録できるのは、30個迄になっています。それ以上が記述されていても、リスト表示されません。<br>
    私の環境では30個登録するとリスト表示される迄が、ややもたついた印象を受けます。
12. 「text=」の内容として記述出来るのは127文字、「cmd=」の内容として記述できるのは、255文字迄です。<br>
    それ以上記述してあると、その行はリスト表示されません。
13. 1行は510文字迄にしてください。
14. 「text=」の中に、ダブル・クォーテーションは記述できません。回避する手段はありません。
15. 「cmd=」の中には、ダブル・クォーテーションを含める事ができます。
16. 最後に現れるダブル・クォーテーションがコマンドの末尾を指します。
17. 「cmd=」の中の先頭と末尾に空白文字またはタブがある場合は、読み込み時に取り除きます。
18. 処理を軽くするために、コード変換や高度な字句解析はしません。<br>
    そのため制限事項はついていますが、ご容赦ください。

### メニューの階層化について

該当行を選んで Enter キーを押すと、そのテキスト・ファイルを読み込みます。
Escキーを押すと一つ前に戻ります。

`------------------------<<<次の行から>>>---------------------------------------`
<br>
`# sample`<br>
`text="sub menu-1";cmd="</home/hoge/.mods/submenu1.mnu";`<br>
`text="sub menu-2";cmd="</home/hoge/.mods/submenu2.mnu";`<br>
`text="sub menu-3";cmd="</home/hoge/.mods/submenu3.mnu";`<br>
`------------------------<<<上の行まで>>>---------------------------------------`

1. 「cmd=」が「#」 で始まる行はメニューファイルの読み込みを意味します。
2. Escキーを押すと一つ前に戻るために、以前のテキスト・ファイルを読み込み直します。<br>
    内容を記憶しているわけではないので、ファイルが変更されていれば変更後のものになります。<br>
    厳密な意味での戻るという動作ではありません。この機能の仕様上はそんなには問題にならないと思います。<br>
    他のウィンドウにフォーカスが移れば終了してしまうため、そのPCのマニュアル操作では読み込みなおしても変更されていないでしょう。<br>
    リモート接続で書き換えられたり cron 登録された処理で書き換えられる可能性は残ってはいますが...
3. 階層は10迄です。
4. '<'の後はcmd=内の内容は全て一つのファイル名として扱われます。<br>
    余分な空白や、「~/」のようにシェルで解釈される表現は使えないので注意してください。
5. 「text="hoge";cmd=""<h oge.mnu"";」のように、’<’の前に空白以外の文字がある場合は、期待(?)通りには機能しません。

### メニューの表示
mods 起動時にメニュー定義ファイル名を渡すとメニュー表示します。これを何らかのランチャーに登録しておけば、利用できます。

### 補足

ほんの少しでも軽快に動作させるためには、テキスト・ファイルを記述する際に次に気をつけると良いでしょう。

1. コメント行は必要最小限にする
2. 一つのファイルに多くを登録しない(10個位迄が目安)
3. 無駄な空白はなくす
4. 「cmd=」には「~/」など、プログラムを起動する前に、シェルに変換される種類の記述は極力避ける(フルパス記述)
5. 表示させる内容は極力簡潔にし、少ない文字数とする

一つ一つは体感出来る効果はないかも知れませんが、積み重ねられれば少しは変わると思います。
少しの工夫で閾値を割り込み、「あれ？少し軽くなった？」と感じる時があるかも知れません。

また、テキスト・ファイルやこのプログラムを置く場所を少しでも速いデバイスを選択すると良いでしょう。
HDD よりも SSD 、SSD よりも tmpfs が高速です。tmpfs の場合は、内容維持に要注意です。
現実的には、tmpfsに配置する必要はないとは思いますが...

定義してあるキーはソースコードにベタ書きしています。動作の軽快さと技術の未熟さにより、外部ファイル化する事はしていません。
変更したい場合は、ソース編集から行う必要があります。変更する場所は、「check_key()」内部です。

今現在は次のように定義されています。

| 機能      | キー | 補足             |
|:---------:|:----:|:----------------:|
|抜ける     |Esc   |                  |
|           | q    |                  |
|           | Q    | Shift + q        |
|           | /    |                  |
|実行する   |Enter |                  |
|           |Enter | 10 key?          |
|           |Space |                  |
|           | s    |                  |
|           | S    | Shift + s        |
|           | j    |                  |
|           | J    | Shift + j        |
|           | @    |                  |
|           | ＼   |backslash(半角)   |
|一つ上     | ↑   |カーソル移動キー  |
|           | ←   |カーソル移動キー  |
|           | b    |                  |
|           | B    | Shift + b        |
|           | p    |                  |
|           | P    | Shift + p        |
|一つ下     | ↓   |カーソル移動キー  |
|           | →   |カーソル移動キー  |
|           | f    |                  |
|           | F    | Shift + f        |
|           | n    |                  |
|           | N    | Shift + n        |
|最初へ     | Home |                  |
|           | t    |                  |
|           | T    | Shift + t        |
|最後へ     | End  |                  |
|           | l    |                  |
|           | L    | Shift + l        |
|           | v    |                  |
|           | V    | Shift + v        |
|該当番号へ |数字  | 0 は 10 の扱い   |
|該当番号+10|数字  | Ctrl + 数字      |
|該当番号+20|数字  | Alt  + 数字      |

Ctrl キーと同時に一つ上へ移動するキーを押すと最初の項目が選択状態になり、Ctrl キーと同時に一つ下へ移動するキーを押すと最後の項目が選択状態になります。

# 謝辞

文末にはなりましたが、「入門GTK+」を公開してくださっている菅谷 保之さんには心より感謝いたします。
(http://www.iim.cs.tut.ac.jp/~sugaya/books/GUI-ApplicationProgramming/files/GTK-book-all.pdf)
mods はこれに記載されているソースをベースにして作成しました。

# 最後に

バグ、要望、その他ご意見等御座いましたら下記よりお問い合わせください。

- Blog: http://unixer.xyz/
