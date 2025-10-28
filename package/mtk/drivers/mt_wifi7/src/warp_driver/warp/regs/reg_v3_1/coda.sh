#!/bin/bash
# SPDX-License-Identifier: <SPDX License Expression>

# translate coda for warp driver usage
################################################
# wed0.h [module: wed0, h_connac link]
################################################
filename='wed_hw.h'

# copy file
cp wed0.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(WED0_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/WED0_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ WED0_/ /g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# wdma0_v3.h [module: WDMA0_v3, h_connac link]
################################################
filename='wdma_hw.h'

# copy file
cp wdma0_v3.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(WDMA0_V3_BASE + 0x/0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/WDMA0_V3_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ WDMA0_V3_/ W/g' $filename
	sed -i 's/) \/\/ [[:xdigit:]]\+$//g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi

################################################
# pao0_gather_only.h [module: pao0_gather_only, h_connac link]
################################################
filename='pao_hw.h'

# copy file
cp pao0_gather_only.h $filename

if [ $? -eq 0 ]; then
	# change offset
	sed -i 's/(PAO0_GATHER_ONLY_BASE.*\ /0x0000/g' $filename

	# remove some lines
	sed -i '/\[File\]/d' $filename
	sed -i '/\[Revision time\]/d' $filename
	sed -i '/\[Description\]/d' $filename
	sed -i '/\[Copyright\]/d' $filename
	sed -i '/PAO0_GATHER_ONLY_BASE\|include/d' $filename
	sed -i '/^$/N;/^\n$/D' $filename

	# change name
	sed -i 's/ PAO0_GATHER_ONLY_/ /g' $filename

	# remove trailing space
	sed -i 's/[[:space:]]*$//' $filename
fi
