all:
	gcc echoClient.c -o echoClient
	gcc SelectEchoServ.c -o SelectEchoServ
	gcc PollClient.c -o PollClient
	gcc EpollClient.c -o EPollClient
	gcc EpollEchoServ.c -o EpollEchoServ

