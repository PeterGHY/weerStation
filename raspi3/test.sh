a=0

#while [ 1 ]
#do
  if [ `pidof iceweasel` -gt 0 ]; then
    echo " running" 
  else
    exit 99
  fi
#	sleep 5
#done
