<?php

define('SESSION_HANDLER_HOST', 'localhost');

require_once('sharedance.php');

function session_handler_open($save_path, $session_name)
{
    if (sharedance_open(SESSION_HANDLER_HOST) !== 0) {
        return FALSE;
    }
    return TRUE;
}

function session_handler_close() {
    sharedance_close();
    
    return TRUE;
}

function session_handler_store($key, $data) {
    if (sharedance_store($key, $data) !== 0) {
        return FALSE;
    }
    return TRUE;
}

function session_handler_fetch($key) {
    return sharedance_fetch($key);
}

function session_handler_delete($key) {
    if (sharedance_delete($key) !== 0) {
        return FALSE;
    }
    return TRUE;
}

function session_handler_gc($timeout) {
    return TRUE;
}

session_set_save_handler('session_handler_open', 'session_handler_close',
                         'session_handler_fetch', 'session_handler_store',
                         'session_handler_delete', 'session_handler_gc');

?>