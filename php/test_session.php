<?php

error_reporting(E_ALL);

require_once('session_handler.php');

session_start();

if (!isset($_SESSION['test'])) {
    $_SESSION['test'] = 0;
} else {
    $_SESSION['test']++;
}

echo 'You reloaded that page ' . $_SESSION['test'] . ' time(s).' . "\n" .
  'Your session id is: [' . session_id() . ']' . "\n";

?>