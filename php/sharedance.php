<?php

define('SHAREDANCE_DEFAULT_PORT', 1042);
define('SHAREDANCE_DEFAULT_TIMEOUT', 10);

function sharedance_open($host, $port = SHAREDANCE_DEFAULT_PORT,
                         $timeout = SHAREDANCE_DEFAULT_TIMEOUT) {
    global $GLOBAL_SHAREDANCE_FP;
    global $GLOBAL_SHAREDANCE_SETTING_HOST;
    global $GLOBAL_SHAREDANCE_SETTING_PORT;
    global $GLOBAL_SHAREDANCE_SETTING_TIMEOUT;
    
    if (!empty($GLOBAL_SHAREDANCE_FP)) {
        return 0;
    }
    if (empty($host)) {
        return -1;
    }
    $GLOBAL_SHAREDANCE_FP = @fsockopen($host, $port, $errno,
                                       $errstr, $timeout);
    if (!$GLOBAL_SHAREDANCE_FP) {
        return -1;
    }
    $GLOBAL_SHAREDANCE_SETTING_HOST = $host;
    $GLOBAL_SHAREDANCE_SETTING_PORT = $port;    
    $GLOBAL_SHAREDANCE_SETTING_TIMEOUT = $timeout;
    
    return 0;
}

function sharedance_reopen() {
    global $GLOBAL_SHAREDANCE_FP;
    global $GLOBAL_SHAREDANCE_SETTING_HOST;
    global $GLOBAL_SHAREDANCE_SETTING_PORT;
    global $GLOBAL_SHAREDANCE_SETTING_TIMEOUT;
    
    if (!empty($GLOBAL_SHAREDANCE_FP)) {
        return 0;
    }
    if (sharedance_open($GLOBAL_SHAREDANCE_SETTING_HOST,
                        $GLOBAL_SHAREDANCE_SETTING_PORT,
                        $GLOBAL_SHAREDANCE_SETTING_TIMEOUT) !== 0) {
        return -1;
    }
    return 0;
}

function sharedance_close() {
    global $GLOBAL_SHAREDANCE_FP;

    if (empty($GLOBAL_SHAREDANCE_FP)) {
        return;
    }
    @fclose($GLOBAL_SHAREDANCE_FP);
    $GLOBAL_SHAREDANCE_FP = FALSE;
}

function sharedance_store($key, $data)
{
    global $GLOBAL_SHAREDANCE_FP;

    if (sharedance_reopen() !== 0) {
        return -1;
    }
    $key_len = strlen($key);
    $data_len = strlen($data);
    $s = sprintf('S%c%c%c%c%c%c%c%c', 
                 ($key_len >> 24) & 0xff, ($key_len >> 16) & 0xff,
                 ($key_len >> 8) & 0xff, $key_len & 0xff,
                 ($data_len >> 24) & 0xff, ($data_len >> 16) & 0xff,
                 ($data_len >> 8) & 0xff, $data_len & 0xff);
    $s .= $key;
    $s .= $data;
    fwrite($GLOBAL_SHAREDANCE_FP, $s);
    if (@fgets($GLOBAL_SHAREDANCE_FP) !== "OK\n") {
        sharedance_close();
        return -1;
    }
    sharedance_close();
    
    return 0;
}

function sharedance_fetch($key)
{
    global $GLOBAL_SHAREDANCE_FP;

    if (sharedance_reopen() !== 0) {
        return -1;
    }
    $key_len = strlen($key);
    $s = sprintf('F%c%c%c%c',
                 ($key_len >> 24) & 0xff, ($key_len >> 16) & 0xff,
                 ($key_len >> 8) & 0xff, $key_len & 0xff);
    $s .= $key;
    stream_set_blocking($GLOBAL_SHAREDANCE_FP, 0);    
    fwrite($GLOBAL_SHAREDANCE_FP, $s);
    stream_set_blocking($GLOBAL_SHAREDANCE_FP, 1);    
    $data = '';
    while (($tmp = @fread($GLOBAL_SHAREDANCE_FP, 4096)) != FALSE) {
        $data .= $tmp;
    }    
    sharedance_close();
    if ($data === '') {
        return FALSE;
    }    
    return $data;
}

function sharedance_delete($key)
{
    global $GLOBAL_SHAREDANCE_FP;

    if (sharedance_reopen() !== 0) {
        return -1;
    }
    $key_len = strlen($key);
    $s = sprintf('D%c%c%c%c',
                 ($key_len >> 24) & 0xff, ($key_len >> 16) & 0xff,
                 ($key_len >> 8) & 0xff, $key_len & 0xff);
    $s .= $key;
    stream_set_blocking($GLOBAL_SHAREDANCE_FP, 0);
    fwrite($GLOBAL_SHAREDANCE_FP, $s);
    stream_set_blocking($GLOBAL_SHAREDANCE_FP, 1);
    if (@fgets($GLOBAL_SHAREDANCE_FP) !== "OK\n") {
        sharedance_close();
        return -1;
    }
    sharedance_close();
    
    return 0;
}

?>