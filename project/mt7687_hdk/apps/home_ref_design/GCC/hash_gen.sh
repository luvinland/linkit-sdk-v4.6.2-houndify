#!/bin/bash

###############################################################################
#Variables

OUTPATH=$1
CM4_BINARY_NAME=$2
ONE_PACKAGE_ENABLED=$3
N9_BINARY_NAME="WIFI_RAM_CODE_MT76X7_in_flash.bin"
HASH_OUT="hash_check"
IMAGE_CM4_OUT="image_cm4"
IMAGE_N9_OUT="image_n9"
A_POSTFIX="_A"
B_POSTFIX="_B"
HASH_POSTFIX="_hash"
FOTA_DUAL_FOLDER="fota_dual_folder"

dec2hex() {
    printf "%.8x" $1
}
###############################################################################
#Begin here
echo $OUTPATH
echo $CM4_BINARY_NAME

if [ -e "$OUTPATH/$FOTA_DUAL_FOLDER" ]; then
	echo "remove old dual_image_fota folder."
	rm -rf $OUTPATH/$FOTA_DUAL_FOLDER
fi
mkdir -p $OUTPATH/$FOTA_DUAL_FOLDER

## gen bianry A hash
./hash_gen "$OUTPATH/$N9_BINARY_NAME" "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX"
./hash_gen "$OUTPATH/$CM4_BINARY_NAME" "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX"

## gen binary B hash
./hash_gen "$OUTPATH/binary_B/$N9_BINARY_NAME" "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"
./hash_gen "$OUTPATH/binary_B/$CM4_BINARY_NAME" "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"

## cat
cat "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    > $OUTPATH/$FOTA_DUAL_FOLDER/$HASH_OUT

rm  "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"

## rename binary
cp "$OUTPATH/$N9_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$A_POSTFIX"
cp "$OUTPATH/$CM4_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$A_POSTFIX"
cp "$OUTPATH/binary_B/$N9_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$B_POSTFIX"
cp "$OUTPATH/binary_B/$CM4_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$B_POSTFIX"

if [ "$ONE_PACKAGE_ENABLED" == "y" ]; then
./bdiff "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$A_POSTFIX" \
         "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$B_POSTFIX" \
         "$OUTPATH/$FOTA_DUAL_FOLDER/diff_patch"

HASH_SIZE=256
MERGED_PATCH_SIZE=$(du -b "$OUTPATH/$FOTA_DUAL_FOLDER/diff_patch" | awk '{print $1}')
CM4_SIZE=$(du -b "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$A_POSTFIX" | awk '{print $1}')
N9_SIZE=$(du -b "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$A_POSTFIX" | awk '{print $1}')
DIFF_SIZE=$(($MERGED_PATCH_SIZE - $CM4_SIZE))

HASH_OFFSET=16
DIFF_OFFSET=$(($HASH_OFFSET + $HASH_SIZE))
CM4_OFFSET=$(($DIFF_OFFSET + $DIFF_SIZE))
N9_OFFSET=$(($CM4_OFFSET + $CM4_SIZE))

echo "merged pack size: "$MERGED_PATCH_SIZE
echo "CM4 size: "$CM4_SIZE
echo "N9 size: "$N9_SIZE

HASH_OFFSET=$(dec2hex $HASH_OFFSET)
DIFF_OFFSET=$(dec2hex $DIFF_OFFSET)
CM4_OFFSET=$(dec2hex $CM4_OFFSET)
N9_OFFSET=$(dec2hex $N9_OFFSET)

echo "offset header:" "0x"$HASH_OFFSET "0x"$DIFF_OFFSET "0x"$CM4_OFFSET "0x"$N9_OFFSET

# offset will be written into package by Big-end and be translated to little-end by download manager.
echo "$HASH_OFFSET" | xxd -r -ps > "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp"
echo "$DIFF_OFFSET" | xxd -r -ps >> "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp"
echo "$CM4_OFFSET" | xxd -r -ps >> "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp"
echo "$N9_OFFSET" | xxd -r -ps >> "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp"

cat "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp" \
    "$OUTPATH/$FOTA_DUAL_FOLDER/$HASH_OUT" \
    "$OUTPATH/$FOTA_DUAL_FOLDER/diff_patch" \
    "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$A_POSTFIX" \
    > $OUTPATH/$FOTA_DUAL_FOLDER/updated_package

rm "$OUTPATH/$FOTA_DUAL_FOLDER/header_temp" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/$HASH_OUT" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/diff_patch" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$A_POSTFIX" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$A_POSTFIX" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$B_POSTFIX" \
   "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$B_POSTFIX"

fi

echo "hash gen done."