<?php

define('SESSION_HANDLER_HOST', 'localhost');

require_once('sharedance.php');

class SharedanceSessionHandler implements SessionHandlerInterface
{
    public function open(string $path, string $name): bool
    {
        if (sharedance_open(SESSION_HANDLER_HOST) !== 0) {
            return false;
        }
        return true;
    }

    public function close(): bool
    {
        sharedance_close();
        return true;
    }

    public function read(string $id): string|false
    {
        $data = sharedance_fetch($id);
        if ($data === false || $data === -1) {
            return '';
        }
        return $data;
    }

    public function write(string $id, string $data): bool
    {
        if (sharedance_store($id, $data) !== 0) {
            return false;
        }
        return true;
    }

    public function destroy(string $id): bool
    {
        if (sharedance_delete($id) !== 0) {
            return false;
        }
        return true;
    }

    public function gc(int $max_lifetime): int|false
    {
        return 0;
    }
}

ini_set('session.serialize_handler', 'php_serialize');

$handler = new SharedanceSessionHandler();
session_set_save_handler($handler, true);

?>
