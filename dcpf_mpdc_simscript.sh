#!/bin/csh

set SF_list = (10) 
set PROTOCOL_list = (dcpf mpdc) #两种协议进行比较
set PGI_list = (5 10 15 20 25) #PGI表示packet generation interval，10-50秒，间隔10秒
set Random_seed = 123477
set Sim_times = 1 #具有相同参数的实验，所做的次数，用来去平均值
set UNIT = s
set CHANNEL_list = (1 2 3)
#set CHANNEL_list = (3)
set STAGGER_list = (st nst) #是否交错时间表
 
# change to your own directory !!
set CONTIKIDIR="/home/user/contiki"
set SCRIPTDIR="/home/user/contiki/examples/MPDCMAC/MPDC_board_sim"


set makefile = "$SCRIPTDIR/Makefile"

set COOJASIM = "1" # 1: YES, 0: NO

# need to modify according to it's sending fixed or endless data
set DATANUM="-1"	#send endless data
set SIMTIME = "2400000"


# key words in $makefile
set mf_key_0 = "TOTAL_DATA_NUM"			#target 0, to determine sending fixed number of data or endless data
set mf_key_1 = "MPDC_SUPPORT"
set mf_key_2 = "SF"						#target 2
set mf_key_3 = "IS_NOT_STAGGER"
set mf_key_4 = "DATA_INTERVAL"			#target 4
set mf_key_6 = "IS_COOJA_SIM"			#target 6
set mf_key_7 = "NUM_CHANNELS"

# key words in $CSCFILE
set csc_key_0 = "TIMEOUT"
set TIMEOUT = "$csc_key_0($SIMTIME);"
set csc_randseed_key = "<randomseed>"

while ($Sim_times > 0)
	echo " "
	echo "A new round simulation:"
	echo " "
	@ Sim_times = $Sim_times - 1
	
	foreach PROTOCOL($PROTOCOL_list)
		#modify makefile: dcpf or mpdf
		sleep 0.5s
		set mf_key_1_ln = `nl -ba $makefile|grep "$mf_key_1 "|awk '{print $1}'` # ln: line number
		echo $PROTOCOL 
		if($PROTOCOL == mpdc) then
			echo "here 1"
			sed -i "${mf_key_1_ln}c $mf_key_1 ?= 1" $makefile
		else if($PROTOCOL == dcpf) then
			echo "here 2"
			sed -i "${mf_key_1_ln}c $mf_key_1 ?= 0" $makefile
		endif
		sleep 0.5s	#to avoid the permission-denited issue when continuously modifying the makefile
		
		set LOGSDIR="$SCRIPTDIR/${PROTOCOL}_$Random_seed"
		@ Random_seed = $Random_seed + 1000
		mkdir $LOGSDIR

			
		foreach CHANNEL($CHANNEL_list)
			if($PROTOCOL == dcpf && ($CHANNEL == 2 || $CHANNEL == 3)) then
						sleep 0.5s
						continue
			endif
			if($PROTOCOL == mpdc && $CHANNEL == 1) then
						sleep 0.5s
						continue
			endif
			set CSCFILENAME="MPDC-MAC-6-${CHANNEL}c.csc" #MPDC-MAC-1c.csc  MPDC-MAC-2c.csc MPDC-MAC-3c.csc

			echo " "
			set CSCFILE = "$SCRIPTDIR/$CSCFILENAME"	
			echo $CSCFILE
			
			# Get the line number of the line containing "<randomseed>"
			set csc_randseed_key_ln = `nl -ba $CSCFILE|grep "$csc_randseed_key"|awk '{print $1}'` 

			# change the value of randomseed in csc file
			sed -i "${csc_randseed_key_ln}c <randomseed>$Random_seed</randomseed>" $CSCFILE		
			echo " "
			sleep 0.5s
			

			sleep 0.5s	#to avoid the permission-denited issue when continuously modifying the makefile
			
			#modify makefile:channel number
			set mf_key_7_ln = `nl -ba $makefile|grep "$mf_key_7 "|awk '{print $1}'`
			sed -i "${mf_key_7_ln}c $mf_key_7 ?= $CHANNEL" $makefile
			sleep 0.5s
			
			foreach SF($SF_list)
				#modify makefile: sleeping factor: 10 or 18
				set mf_key_2_ln = `nl -ba $makefile|grep "$mf_key_2 "|awk '{print $1}'`
				sed -i "${mf_key_2_ln}c $mf_key_2 ?= $SF" $makefile
				sleep 0.5s
				
				foreach STAGGER($STAGGER_list)
					if($PROTOCOL == dcpf && $STAGGER == st) then
						sleep 0.5s
						continue
					endif
					if($PROTOCOL == mpdc && $CHANNEL == 3 && $STAGGER == nst) then
								sleep 0.5s
								continue
					endif
					#modify makefile: support stagger or not
					set mf_key_3_ln = `nl -ba $makefile|grep "$mf_key_3 "|awk '{print $1}'`
					if($STAGGER == st) then
						sed -i "${mf_key_3_ln}c $mf_key_3 ?= 0" $makefile
					else if($STAGGER == nst) then
						sed -i "${mf_key_3_ln}c $mf_key_3 ?= 1" $makefile
					endif
					sleep 0.5s
					foreach PGI($PGI_list)
						#modify makefile: set packet generation interval
						set mf_key_4_ln = `nl -ba $makefile|grep "$mf_key_4 "|awk '{print $1}'`
						sed -i "${mf_key_4_ln}c $mf_key_4 ?= $PGI" $makefile
						sleep 0.5s
						
						#modify makefile: how many data to be sent: fixed number of endless
						set mf_key_0_ln = `nl -ba $makefile|grep "$mf_key_0 "|awk '{print $1}'` # ln: line number
						sed -i "${mf_key_0_ln}c $mf_key_0 ?= $DATANUM" $makefile
						sleep 0.5s
						
						
						#modify makefile: is it cooja simulation?
						set mf_key_6_ln = `nl -ba $makefile|grep "$mf_key_6 "|awk '{print $1}'` # ln: line number
						sed -i "${mf_key_6_ln}c $mf_key_6 ?= $COOJASIM" $makefile
						sleep 0.5s
						
						
						set CURRENTFOLDER=${PROTOCOL}_${SF}_${CHANNEL}C_${STAGGER}_${PGI}${UNIT}
						
						echo $CURRENTFOLDER
						mkdir $LOGSDIR/$CURRENTFOLDER
						
						cd $SCRIPTDIR
						make -s TARGET=z1 clean
						make -s TARGET=exp5438 clean
						make -s TARGET=sky clean
						
						cd $LOGSDIR/$CURRENTFOLDER
						#rm -f *.*
						
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
		end	
	end
end

echo "Done (endless data)! Congratulations!"

