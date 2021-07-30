#-------------------------------------------------------------------------------------------------------
# 注釈：
# Makefileのインデントはタブ文字でなければいけません。スペース4個や8個ではエラーになります
# 引数がない場合は最初のルールが選ばれるので、この場合は9ccは指定しなくてもよい(今回の場合：makeは、make 9cc)


# 関数の適用:
# 出典(https://www.it-swarm-ja.com/ja/makefile/makefileのサブディレクトリからのソース/970784463/)
# ・ Makeは再帰的なワイルドカード機能を提供していないので、ここではそれを紹介する:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
# ・あるパターンにマッチする全てのファイルを再帰的に検索する方法
ALL_CS := $(call rwildcard,src/,*.c)


# 変数宣言
# -std=c11: Cの最新規格であるC11で書かれたソースコードということを伝える
# -g: デバグ情報を出力する
# -static: スタティックリンクする
CFLAGS=-std=c11 -g -static
SRCS=$(call ALL_CS)
OBJS=$(SRCS:.c=.o)
#-------------------------------------------------------------------------------------------------------

# clean & test
ct: clean
				make test

build: $(OBJS)
				mkdir -p build && $(CC) -o ./build/9cc $(OBJS) $(LDFLAGS)

$(OBJS): src/9cc.h # すべての.oファイルが9cc.hに依存していることを表している

test: build
				bash ./test/test.sh

# bash formmat
fmt:
				shfmt -i 2 -w ./**/*.sh && echo formmatted.

clean:
				rm -rf ./build src/*.o *~ tmp*

.PHONY: test clean
