#seltter eventuelt gamle verdier
perl -ni -le 'print unless /memlock/' /etc/security/limits.conf

#legger inn nye verdier for memlock
echo "#gj�r at man kan l�se uendelig med mine" 		>> /etc/security/limits.conf
echo "*               hard    memlock         -1" 	>> /etc/security/limits.conf
echo "*               soft    memlock         -1" 	>> /etc/security/limits.conf

