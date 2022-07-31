CC := gcc

CFLAGS := -g

LDFLAGS := -pthread

.PHONY := all clean

all: client_for_load_test_tcp_udp.out\
server_one_thread_for_one_client_tcp.out\
server_one_thread_for_one_client_udp.out\
server_thread_pull_scheme.out\
server_consumer_producer_scheme.out\
server_threads_plus_multiplexing_cheme.out\
server_doubleflow_echo_tcp_udp.out

client_for_load_test_tcp_udp.out: client_for_load_test_tcp_udp.c
	$(CC) -o client_for_load_test_tcp_udp.out client_for_load_test_tcp_udp.c $(CFLAGS)
server_one_thread_for_one_client_tcp.out: server_one_thread_for_one_client_tcp.c
	$(CC) -o server_one_thread_for_one_client_tcp.out server_one_thread_for_one_client_tcp.c $(CFLAGS) $(LDFLAGS)
server_one_thread_for_one_client_udp.out: server_one_thread_for_one_client_udp.c
	$(CC) -o server_one_thread_for_one_client_udp.out server_one_thread_for_one_client_udp.c $(CFLAGS) $(LDFLAGS)
server_thread_pull_scheme.out: server_thread_pull_scheme.c
	$(CC) -o server_thread_pull_scheme.out server_thread_pull_scheme.c $(CFLAGS) $(LDFLAGS)
server_consumer_producer_scheme.out: server_consumer_producer_scheme.c
	$(CC) -o server_consumer_producer_scheme.out server_consumer_producer_scheme.c $(CFLAGS) $(LDFLAGS) -lrt
server_threads_plus_multiplexing_cheme.out: server_threads_plus_multiplexing_cheme.c
	$(CC) -o server_threads_plus_multiplexing_cheme.out server_threads_plus_multiplexing_cheme.c $(CFLAGS) $(LDFLAGS)
server_doubleflow_echo_tcp_udp.out: server_doubleflow_echo_tcp_udp.c
	$(CC) -o server_doubleflow_echo_tcp_udp.out server_doubleflow_echo_tcp_udp.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.out