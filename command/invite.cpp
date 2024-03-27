// 채널이 없는경우
// INVITE a hi
// :irc.local 403 c hi :No such channel

//c가 a를 초대 -> a에게 초대 메시지 보냄
// :irc.local 341 c a :#hi
// :c!root@127.0.0.1 INVITE a :#hi

// 없는 채널 케이스1 
// MODE hi +i
// :irc.local 401 c hi :No such nick

// 없는 채널 케이스2 
// MODE #hi2 +i
// :irc.local 403 c #hi2 :No such channel

// MODE #hi +i
// :c!root@127.0.0.1 MODE #hi :+i

