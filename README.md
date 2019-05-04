# Yu-Gi-Oh  

Yu-Gi-Oh is a famous card game and anime show invented in Japan and is now popular all around the world. Our project is to design and program a Yu-Gi-Oh game on a on Gumstix Verdex Pro board. In this project, the Gumstix Verdex Pro board is used as the main process platform, and an LCD screen is used to show the user interface made with Qt. In order to make this game more engaging, an RFID reader connected with an Arduino Micro is used to read the cards. Some buttons are used to control the game instead of using keyboard input. In the end of the project, the game logic is able to handle one monster card for each side, and a Qt program is developed to make the game table for both players head-to-head. Also, a kernel module is implemented to help read from the button and push the game forward.
 
## Instructions
1. Make sure SD card is in the EC 535 Gumstix board.  Transfer the files from the Final Version of this project into the SD card after you have ran the make on the Qt, the kernel, and the game components of our game (located in the qt, km, and ul directories).  Make sure to connect the buttons to the GPIO pins specified in the mytimer.c kernel module source code (also, even though it is called mytimer.c, it doesn't actually have anything to do with timers... it is for button handling only).
2. Transfer the data directory and the data.txt file onto the SD card.
3. Transfer the Arduino code onto an Arduino Micro.  Make sure to have the RC522 Arduino library installed in your Arduino IDE.
4. Connect your the RC522 RFID reader to the Arduino as mentioned here: https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/
5. Continuing following the steps below:

(From Lab 5 instructions)
  
create binary:
  
	make  
copy binary to gumstix via zmodem
  
copy libraries into a sd card via zmodem:
  
	sd card is in /media/card on gumstix
	create a folder called lib
	copy following: "fonts libQtCore.so.4 libQtGui.so.4 libQtNetwork.so.4 ld-uClibc.so.0 libc.so.0 libm.so.0 libstdc++.so.6"
		from folder: /ad/eng/courses/ec/ec535/gumstix/oe/qt/lib
		from folder: /ad/eng/courses/ec/ec535/arm-linux/lib
		into /media/card/lib/
    
create library links in gumstix:
  
	cd /usr/lib
	ln -s /media/card/lib/libQtCore.so.4 libQtCore.so.4
	ln -s /media/card/lib/libQtGui.so.4 libQtGui.so.4
	ln -s /media/card/lib/libQtNetwork.so.4 libQtNetwork.so.4
	ln -s /media/card/lib/ld-uClibc.so.0 ld-uClibc.so.0
	ln -s /media/card/lib/libc.so.0 libc.so.0
	ln -s /media/card/lib/libm.so.0 libm.so.0
	ln -s /media/card/lib/libstdc\+\+.so.6 libstdc\+\+.so.6
export some variables in gumstix:
  
	export QWS_MOUSE_PROTO='tslib:/dev/input/touchscreen0'
	export TSLIB_CONFFILE=/etc/ts.conf
	export TSLIB_PLUGINDIR=/usr/lib
	export TSLIB_TSDEVICE=/dev/input/event0
	export TSLIB_FBDEVICE=/dev/fb0
	export TSLIB_CONSOLEDEVICE=/dev/tty
	export QT_QWS_FONTDIR=/media/card/lib/fonts
	export TSLIB_PLUGINDIR=/usr/lib/ts

run binary:
  
	./project -qws &
  
add the kernel module for buttons:
  
    mknod /dev/mytimer c 61 0
    insmod mytimer.ko

run game:
  
    ./game
    
For the cards, make sure to update the data.txt file with the card ID's and card information for the monsters you want to be included in your game.  In addition, update line 24 in game.c with the number of cards that you have in your data.txt file and update line 26 with the number of cards in your data.txt - 1.
