count=1;
for i in /home/boitho/cvstestdata/lot/*/*/anchors;
do
        echo $count;

        perl IndexerLotAnchors.pl $count;
	#sleep 30; #sover litt s� maskinen f�r mulighet til � hente seg inn hvis det er noe annet som ogs� m� gj�res

        count=`expr $count + 1`;
done

