<?php
//http://php.net/manual/en/ref.image.php
//http://php.net/manual/en/book.imagick.php
//params
$gcodefilename="bike.gcode";
$imagefilename="bike.jpg";
$grayscaledimage="bikegray.jpg";
$thickness = 2.50;
$workspeed = 50;
$movespeed = 150;
$tooldiam=0.2;
$spindle = 2000;
$safez = 5;
$prodwidth=80.0;
$prodheight=80.0;

//calculate passes
$finw = intval(floatval($prodwidth)/floatval($tooldiam));
$finh = intval(floatval($prodheight)/floatval($tooldiam));

//open gcode file for write
$gcodefile = fopen($gcodefilename, "w");

//open image
//$im = new Imagick( $imagefilename );
$im = imagecreatefromjpeg($imagefilename);

//grayscale image
//$im->modulateImage(100,0,100);
imagefilter($im, IMG_FILTER_GRAYSCALE);

//scale image for passes http://php.net/manual/en/imagick.resizeimage.php
//$im->resizeImage($finw,$finh,Imagick::FILTER_LANCZOS,0);
$im = imagescale($im, $finw, $finh, IMG_BILINEAR_FIXED);

//save scaled image for check
//$im->setImageFormat ("jpeg");
//file_put_contents($grayscaledimage, $im);
imagejpeg($im, $grayscaledimage);

//start gcode file
fwrite($gcodefile, "%\n");
fwrite($gcodefile, "M3 S".$spindle."\nG21\nG00 Z".$safez." F".$movespeed."\n");

//get final w / h
//$imw=$im->getImageWidth();
//$imh=$im->getImageHeight();

$imw = imagesx($im);
$imh = imagesy($im);

//loop rows
for ($x = 0; $x < $imw; $x++) {
    //loop columns
    for ($y = 0; $y < $imh; $y++) {
       $yc=$imh-$y-1;
       $ry=$y;
       if ( $x % 2 ) {
          $ry=$imh-$y-1;
           $yc=$y;
       }
       //$pixel = $im->getImagePixelColor($x,$yc);
       //$color = $pixel->getColor();
       //$zval = (floatval(floatval($thickness)/floatval(255)))*(floatval($color['r']));
       $pixel = imagecolorat($im,$x,$yc);
       $rval= ($pixel >> 16) & 0xFF;
       $zval = (floatval(floatval($thickness)/floatval(255)))*(floatval($rval));
       $zval = number_format($zval, 2, '.', '');
       fwrite($gcodefile, "G01 Z-".$zval." F".$workspeed."\n");
       $xg=number_format(floatval($x*$tooldiam), 2, '.', '');
       $yg=number_format(floatval($ry*$tooldiam), 2, '.', '');
       fwrite($gcodefile, "G01 X".$xg." Y".$yg." Z-".$zval." F".$workspeed."\n");
    }

}
//back home tool and stop
fwrite($gcodefile, "G00 Z".$safez." F".$movespeed."\nM5\nG00 X0.00 Y0.00 F".$movespeed."\nM2\n");
fwrite($gcodefile, "%\n");

//close gcode file
//$im->clear();
fclose($gcodefile);
?>
