FROM ubuntu:24.04

RUN apt-get update && \
	apt-get install inspircd -y && \
	apt-get install irssi -y && \
	apt-get install tcpflow -y

RUN mkdir /var/run/inspircd

CMD ["/usr/sbin/inspircd", "--runasroot", "--nofork"]