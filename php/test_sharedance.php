<?php

error_reporting(E_ALL);

require_once('sharedance.php');

if (sharedance_open('localhost') !== 0) {
    die();
}
sharedance_store('foo-key', 'bar-content');
echo '[' . sharedance_fetch('foo-key') . ']' . "\n";
sharedance_delete('foo-key');
sharedance_close();

?>