#!/bin/csh
set PROTOCOL = nullrdc
set PGI_list = (10 15 20 25 30) #PGI表示packet generation interval，10-50秒，间隔10秒
set Random_seed = 123377
set Sim_times = 1 #具有相同参数的实验，所做的次数，用来去平均值
set UNIT = s


# change to your own directory !!
set CONTIKIDIR="/home/user/contiki"
set SCRIPTDIR="/home/user/contiki/examples/CCP/CCP_board_sim"


set CSCFILENAME="CCP-collect-2.csc"

set CSCFILE = "$SCRIPTDIR/$CSCFILENAME"
set makefile = "$SCRIPTDIR/Makefile"



# need to modify according to it's sending fixed or endless data
set SIMTIME = "2400000"


# key words in $makefile
set mf_key_1 = "RDC_DRIVER"
set mf_key_2 = "DATA_INTERVAL"

# key words in $CSCFILE
set csc_key_0 = "TIMEOUT"
set TIMEOUT = "$csc_key_0($SIMTIME);"
set csc_randseed_key = "<randomseed>"

while ($Sim_times > 0)
	echo " "
	echo "A new round simulation:"
	echo " "
	@ Sim_times = $Sim_times - 1

	# Get the line number of the line containing "<randomseed>"
	set csc_randseed_key_ln = `nl -ba $CSCFILE|grep "$csc_randseed_key"|awk '{print $1}'` 
	
	# change the value of randomseed in csc file
	sed -i "${csc_randseed_key_ln}c <randomseed>$Random_seed</randomseed>" $CSCFILE
	
	echo " "

	sleep 0.5s
	set LOGSDIR="$SCRIPTDIR/CCP_Board_$Random_seed"
	@ Random_seed = $Random_seed + 1000
	mkdir $LOGSDIR
	
	sleep 0.5s	#to avoid the permission-denited issue when continuously modifying the makefile
	

	foreach PROTOCOL($PROTOCOL_list)
		#modify makefile: how many data to be sent: fixed number of endless
		set mf_key_0_ln = `nl -ba $makefile|grep "$mf_key_0 "|awk '{print $1}'` 
		sed -i "${mf_key_0_ln}c $mf_key_0 ?= $PROTOCOL" $makefile
		sleep 0.5s
			
		foreach PGI($PGI_list)
			#modify makefile: set packet generation interval
			set mf_key_2_ln = `nl -ba $makefile|grep "$mf_key_2 "|awk '{print $1}'`
			sed -i "${mf_key_2_ln}c $mf_key_2 ?= $PGI" $makefile
			sleep 0.5s
			
			
			set CURRENTFOLDER=${PROTOCOL}_${PGI}${UNIT}
			
			echo $CURRENTFOLDER
			mkdir $LOGSDIR/$CURRENTFOLDER
			
			cd $SCRIPTDIR
			make -s TARGET=z1 clean
			make -s TARGET=exp5438 clean
			make -s TARGET=sky clean
			
			cd $LOGSDIR/$CURRENTFOLDER
			
			#modify the cooja csc file
			set csc_key_0_ln = `nl -ba $CSCFILE|grep "$csc_key_0"|awk '{print $1}'` # ln: line number
			sed -i "${csc_key_0_ln}c $TIMEOUT" $CSCFILE
			sleep 0.5s
			cp $CSCFILE ./
			
			dbus-launch gnome-terminal -x  bash -c "java -mx512m -jar $CONTIKIDIR/tools/cooja/dist/cooja.jar -nogui="$CSCFILE" -contiki="$CONTIKIDIR"; exec bash"
			echo "this over"
			echo " "
            sleep 1m
			cd $SCRIPTDIR
		end
	end 
end

echo "Done (endless data)! Congratulations!"


