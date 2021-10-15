#!/bin/bash
strcmd=$1
#单口设备
arraycdc=("0730 dcba" "32c3 dcba")
#复合口设备
arrayMis=("05c6 9018" "05c6 9020" "05c6 9022" "05c6 9025" "05c6 9026" "05c6 9102" "05c6 9120" "1e0e 9001" "1e0e 902b" "1e0e 902e" "1e0e 9031" "1782 3d00" "1782 4d10")
#adb设备
arrayAdb=("18d1 d00d")
#设备加载路径
rootPath="/sys/bus/usb/devices"

#子目录数组
foldArray=("")

function read_dir(){

for folder in `ls ${rootPath}`	
do
	foldArray[i]="${folder}"
	((i=i+1))
done
}

read_dir

#遍历usb枚举子目录
for((i=0; i<${#foldArray[@]}; i++))
do
tmppath=$rootPath/${foldArray[$i]}
pid=$tmppath/idProduct
vid=$tmppath/idVendor
if [ -f "$vid" ]; then
	usb="`cat "$vid"` `cat "$pid"`"
	#echo "--------------------------------------"
	#echo "$usb"
	#复合口设备
	for((j=0; j<${#arrayMis[@]}; j++))
	do
		if [ "${arrayMis[$j]}" = "$usb" ]; then
			ls "$tmppath:1.2/" | grep ttyUSB*
		fi
	done
	#单口设备
	for((j=0; j<${#arraycdc[@]}; j++))
	do	
		if [ "${arraycdc[$j]}" = "$usb" ]; then
			find "$tmppath/" -type d|sed 's#.*/##'|grep ttyACM
		fi
	done
	#adb 设备
	for((j=0; j<${#arrayAdb[@]}; j++))
	do	
		if [ "${arrayAdb[$j]}" = "$usb" ]; then
			cat "$tmppath/serial"
		fi
	done
  
fi
  
done