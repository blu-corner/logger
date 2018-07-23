# logger Recipes

## Format

The format keywords are displayed below. Users can use these to modify the
order or remove them from a log line.

| Keyword | Description |
| :---: | :--- |
| severity | Log level of the message debug/info/warn/err/fatal |
| time | Event timestamp |
| name | Name of the module that logged the event |
| message | the log string |

The following example would place a pipe after the time and module name.

```bash
lh.console.enabled=true
lh.console.level=debug
lh.console.format={time} {name} {severity}| {message}
```

## Console

```bash
lh.console.enabled=true
lh.console.level=debug
lh.console.color=false
```

## Shared Memory

An app is configured to connect to a shared memory socket that is managed by a 
separate daemon process (logger-daemon). The logger-daemon processes these
events and they are passed to configured handlers for logging e.g. file.

The logger-daemon creates the shared memory socket and reads events, these are
passed to configured log handlers. In the below example, the daemon will log 
info to console and then write debug to a file.

```bash
logger.daemon.shm.sock=/tmp/logs.sock
logger.daemon.shm.key=3456

lh.console.enabled=true
lh.console.level=info

lh.file.enabled=true
lh.file.path=/tmp/output.log
lh.file.level=debug
```

An example logger-daemon config is also available in the etc folder of the
installation directory. The logger-daemon is started as follows:

```console
$ ./bin/logger-daemon -c etc/logger-daemon.ini
```

The following application config configures the app to connect to a shared 
memory socket, for writing events.

```bash
lh.console.enabled=true
lh.console.level=warn

lh.shm.enabled=true
lh.shm.sock=/tmp/logs.sock
lh.shm.level=info
```

## File Roll

The following config will roll the log file when it reaches 5MB in side. A
maximum of 10 log files will be retained.

```bash
lh.file.enabled=true
lh.file.level=debug
lh.file.size=5242880
lh.file.count=10
```
