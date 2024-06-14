all:
	gcc echoClient.c -o bin/echoClient
	gcc SelectEchoServ.c -o bin/SelectEchoServ
	gcc PollClient.c -o bin/PollClient
	gcc EpollClient.c -o bin/EPollClient
	gcc EpollEchoServ.c -o bin/EpollEchoServ

