<?php
header("Content-disposition: attachment; filename=$saveas");
header("Content-type: application/octetstream");
header("Pragma: no-cache");
header("Expires: 0");

$fp=fopen("$filename","r");
$str=fread($fp,filesize($filename));
echo $str;
fclose($fp);
?>
