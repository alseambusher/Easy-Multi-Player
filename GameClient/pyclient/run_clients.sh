flag=0
cat keys| while read line
do
	if [ $flag = 0 ]
	then
		echo 'HOST = "127.0.0.1"' >config.py
		echo 'PORT_SUB = "6001"' >>config.py
		echo 'PORT_PUSH = ["6002", "6003", "6004", "6005", "6006"]' >>config.py
		echo 'SERVER_SHARED_KEY = "b8b15ab61f3fe23b968bf72762ba3d77"' >>config.py
		echo 'SESSION_KEY = "TBS"' >>config.py
		echo 'ALIVE_PULSE=60' >>config.py
		echo 'RATE=10' >>config.py
		flag=1
	else
		flag=0
	fi
	echo $line >>config.py
	#python EMPC.py&
done
