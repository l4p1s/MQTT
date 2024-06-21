# コンパイラ
CC = gcc

# コンパイルオプション
CFLAGS = -I../lib

# ソースファイル
SRCS = ../lib/build_command.c ../lib/encode.c ../lib/decode.c ../lib/MSB_LSB.c ../lib/send_message.c broker.c

# オブジェクトファイル
OBJS = $(SRCS:.c=.o)

# 実行ファイル
TARGET = broker

# ルール
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

# 個々のソースファイルのコンパイル
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
