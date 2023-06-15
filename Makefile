CC = g++
CFLAGS = -O2 -g -Wall

all: chat_server

chat_server:
	@echo "[+] Building project..."
	$(CC) $(CFLAGS) -o chat_server ./main.cpp

clean:
	@echo "[+] Cleaning..."
	rm -f chat_server