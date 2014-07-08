<?php

error_reporting(E_ALL);

require_once('session_handler.php');

session_start();

if (!isset($_SESSION['test'])) {
    $_SESSION['test'] = 0;
} else {
    $_SESSION['test']++;
}

header('Content-Type: text/plain');

echo 'You reloaded that page ' . $_SESSION['test'] . ' time(s).' . "\n" .
  'Your session link is: [' . SID . ']' . "\n";

?>