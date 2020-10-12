#!/bin/bash

WH=$(identify -format \
%w \
$1 \
 )
HE=$(identify -format \
%h \
$1 \
 )

for (( x=1; x<=WH; x++ ))
do
  for (( y=1; y<=HE; y++ ))
  do
    xc=$(($x-1))
    yin=$(($y-1))    
    yc=0    
if [[ $(($xc % 2)) -eq 0 ]];
       then yc=$yin;
       else yc=$(($HE-$y));
    fi
    #convert $1 -resize $xx$y\! -format "%[pixel:u]\n" info:
    #identify -format "%[fx:s.p{$x,$y}.r]" $1
    #convert $1[1x1+$xc+$yc] -format "%[fx:round(255*u.r)],%[fx:round(255*u.g)],%[fx:round(255*u.b)]" info:
    r=$(convert \
    $1[1x1+$xc+$yc] \
    -format \
    '%[fx:round(255*u.r)]' \
    info:
    )
    g=$(convert \
    $1[1x1+$xc+$yc] \
    -format \
    '%[fx:round(255*u.g)]' \
    info:
    )
    b=$(convert \
    $1[1x1+$xc+$yc] \
    -format \
    '%[fx:round(255*u.b)]' \
    info:
    )
    med=$((($r+$g+$b)/3))
    echo "x=$xc y=$yc val=$med"
    #identify -format "%[fx:s.p{$x,$y}.g]" $1
    #identify -format "%[fx:s.p{$x,$y}.b]" $1
  done
done
