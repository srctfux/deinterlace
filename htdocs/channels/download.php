<?php

$filename=$_GET["filename"];
$saveas=$_GET["saveas"];

header("Content-disposition: attachment; filename=$saveas");
header("Content-type: application/octetstream");
header("Pragma: no-cache");
header("Expires: 0");

$fp=fopen("$filename","r");
$str=fread($fp,filesize($filename));
$str .= chr(13);
echo $str;
fclose($fp);
?>
