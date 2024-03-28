<처리 완료>

1)서버에 register하지 않은 사용자의 올바른 명령어
	[client] TOPIC #coffee b
	server)	:irc.local 451 * TOPIC :You have not registered.


◼︎)이하 서버에 register한 사용자의 command 사용

2) Not enough parameters.
[client] TOPIC
server) :irc.local 461 green TOPIC :Not enough parameters.
테스트) :irc.local 461 Jackson5 TOPIC :Not enough parameters.

3) No such channel
[client] TOPIC a
server) :irc.local 403 green a :No such channel
테스트) :irc.local 403 Jackson5 a :No such channel

[client] TOPIC #love	(love라는 채널이 없는 경우)
server) :irc.local 403 green #love :No such channel
테스트) :irc.local 403 Jackson5 #love :No such channel

[client] TOPIC #coffee Never gonna dance again (:이 없는 경우 채널과 topicMsg 구분이 안 됨)
server) :irc.local 403 Panda #coffee :No such channel

4) You're not on that channel (coffee라는 채널이 있는데 아직 채널에 들어가지 않은 경우)
[client] TOPIC #coffee c
server) :irc.local 442 green #coffee :You're not on that channel!
테스트) :irc.local 442 Jackson5 #coffee :You're not on that channel!


// 5와 6번 중 무엇을 먼저 처리해야할지 상용서버 행동 관찰

5) 사용자가 operator 여부 및 MODE +-t에 따른 처리 (채널도 있고, 사용자가 채널에 join한 상태인데 operator)

6) 332 (존재하는 채널이고, 사용자가 채널에 join한 상태인데, 변경할 topic 매개변수를 입력하지 않은 경우)
>>> 결론 : 332만 하기로

[client] TOPIC #coffee	(coffee라는 채널이 있는 경우)	
	• 🤔? 상용 클라이언트에서는 client에서 내부적으로 처리해 server로 발송하지 않는 message다 
	• 그러나 nc테스트에서는 아래와 같이 응답한다 (:a 는 현재 channel의 TOPIC | :blue 는 현재 channel의 TOPIC)
	• 마지막에 IP주소 다음 171152.... 숫자 정체 파악 필요 (시간으로 추정)
	• 어? root가 a로 바뀌었다.
 
[client] TOPIC #coffee	(서버가 coffee채널의 topic이 blue라고 응답하고 있다.)
server) :irc.local 332 green #coffee :a 
		:irc.local 333 green #coffee dragon!root@127.0.0.1 :1711521110

[client] TOPIC #coffee	(서버가 coffee채널의 topic이 blue라고 응답하고 있다.)
server) :irc.local 332 shoulder #coffee :blue
		:irc.local 333 shoulder #coffee dragon!root@127.0.0.1 :1711524296

[client] TOPIC #coffee	(서버가 coffee채널의 topic이 lucky day!라고 응답하고 있다.)
server) :irc.local 332 Batman #coffee :lucky day!
		:irc.local 333 Batman #coffee Batman!a@127.0.0.1 :1711525315


-------------------------------------------------------------------------

MODE에 따라 operator 권한에 따른 처리
	1) Mode +t인 경우 operator인지 확인, 아니면 에러 메시지
	2) Mode -t인 경우 continue





[client] TOPIC #coffee,bus		
• 🤔오, `,`를 구분자로 처리하지 않는다. 공백을 구분자로 해서 channle명 다음은 바꾸고자 하는 topic 내용으로 처리
server) :irc.local 403 Batman #coffee,bus :No such channel


[client] TOPIC #coffee especially for you
server) :irc.local 442 green #coffee :You're not on that channel!


<권한 없음 (!operator)>


<정상처리>
[client] TOPIC #coffee Sorry seems to be the hardes word
server)  :seaweed!root@127.0.0.1 TOPIC #coffee :sorry seems to be the hardest word
server)  :Thunder!a@127.0.0.1 TOPIC #coffee :Sorry seems to be the hardes word