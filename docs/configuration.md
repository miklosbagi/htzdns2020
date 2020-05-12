# HTZDNS Configuration

There are two configuration layers available.
## Main config
This is htzdns.conf with items you may not want to touch (but can).

### Cache directory
htz_cache_dir variable holds this, configuring what directory should the cache file be stored at.
Note that this is relative to where htzdns is placed at.

### Logger tag
htz_oogger_tag as string defines what tag to use in syslog.

### Exclude hosts
htz_host_exclude has a comma separated list of hosts you definitely want to exclude from any listing htzdns does.
This avoids confusion, and adds an extra level of safety.

## Host specific config
Two main variables are set here for now:

### API Key
You can acquire this at Hetzner when you login to DNS. No updates without this.

### Update Zone/Record map
This is a list of items:
- zone name
- zone id
- record name
- record id

One line per configuration.

For example:
```
htz_update_zone_record_map="
example.com,sdfghjklASDFGHJKL1234567890,www,abcdefgh1234567890abcdefgh
newexample.com,zxcvbnm1234567890,vpn,qwertyuio1234567890
"
```

# Unexposed configuration options
There are some constants that can be changed, but not exposed to the main or host specific configuration files.
These are:
- cache file (data/cache.data): inc/cache_handler.inc has cache_file=; feel free to set this as needed.
- vendor scripts location (vendor/$vendor_script): init.inc has a fullpath variable embedded within the function: fullpath=...
- configuration filenames (htzdns.conf, htzdns-$host.conf): init.inc has a d= called variable including these.

### Binary dependencies
Binary dependencies are detected automatically. Directories where binaries are looked for:
- /bin
- /usr/bin
- /usr/local/bin
- /sbin
- /usr/sbin
- /opt/bin
- /jffs/bin
- /jffs/sbin
- /jffs/usr/sbin
- /jffs/usr/bin

This list can be skimmed or extended by modifying the list defined by the binloc= variable in inc/init.inc.
Useful when you have multiple binaries available but looking for one specific to be used (e.g. a curl built without SSL won't be much help here).

 
