# HtzDNS
This is a dynamic DNS udpater for Hetzner [DNS robot](https://dns.hetzner.com).

TL;DR: Rewrite of hetzner-dyndns php script in bash, so we can run on a weak openwrt/ddwrt router from cron.

## Is this for me? (checklist):
- Do you want to access computers at a remote location?
- Do you host stuff on premise, with no cloud access?
- Does your ISP change your IP address every once in a while?
- Do you want to avoid paying monthly for a DYNDNS service, or update free tier every so often?
- Do you have a domain at Hetzner?
- Would a mysite.mydomain.tld sound good to you?
=

If you answered with "no" to any of the above questions, this is not for you. In any other case, read on.


- Run this from cron periodically.
- It is written in bash, so it runs on weaker routers, rasp pi, or any modern microwave oven. 

# Usage
## Prerequisites
1) Make sure you see your DNS zones in "Your zones" when you login to https://dns.hetzner.com
2) In your profile/API Tokens (https://dns.hetzner.com/settings/api-token) generate an access token, and give it a name, for example: htzdns (note: the token, a 32 chars long alphanumeric hash is displayed only once).
3) You may want to help yourself and lower the TTL on those records you're looking to update, for fast dns resolution change that is.
4) Create a backup of your zone files, just in case something goes sideways (not that I expect it).

On the dependencies front, the only non-standard \*nix package you will need is jq (for handling json objects). This is available for ARMv7, MIPS, X86, and pretty much any linux distro; and brew will install this on MacOS too.

## Clone this

Run git clone --recurse-submodules gogs@gogs.mb:rt-vault-gogsadmin/htzdns.git

```
$ git clone --recurse-submodules gogs@gogs.mb:rt-vault-gogsadmin/htzdns.git
Cloning into 'htzdns'...
remote: Enumerating objects: 136, done.
remote: Counting objects: 100% (136/136), done.
remote: Compressing objects: 100% (128/128), done.
remote: Total 136 (delta 61), reused 0 (delta 0)
Receiving objects: 100% (136/136), 35.77 KiB | 1.99 MiB/s, done.
Resolving deltas: 100% (61/61), done.
Submodule 'vendor/whatismyip' (gogs@gogs.mb:linux-config/whatismyip.git) registered for path 'vendor/whatismyip'
Cloning into '/home/test/htzdns/vendor/whatismyip'...
remote: Enumerating objects: 16, done.
remote: Counting objects: 100% (16/16), done.
remote: Compressing objects: 100% (16/16), done.
remote: Total 16 (delta 4), reused 0 (delta 0)
Receiving objects: 100% (16/16), done.
Resolving deltas: 100% (4/4), done.
Submodule path 'vendor/whatismyip': checked out '101322e15ffb5ec58fef97b295a4ce4bffb0ceef'
```

## Configure
Run htzdns.sh -r to rebuild configuration for your host, and follow along.
Here is what's going to happen:
- You will be asked for your Hetzner API key.
- Once provided you will get asked for each record in each zone to be selected for dyndns update, starting with the first zone.
- Set the number of line you are intetested in, and press enter.
- When you are done configuring the actual zone, press enter (empty input).
- Keep going until you are done with all your zones.
- A configuration file will be revealed and written for you.
- A pretend-run will be executed to see how we are doing (with no changes made).


### In a terminal
Note: you don't need root or anything, any user should be able to run this once binary dependencies are all in place.

```
[myuser@mydevice]$# cd htzdns/
[myuser@mydevice htzdns ]$# ./htzdns.sh
⚠️  WARNING: No API key was found, launching builder.
A few items are missing from the config (htzdns-mb-macbook), we need to build them in order to continue.
This is ideally performed only once, but you can force this by setting the --rebuild flag to ./htzdns.sh.

Configuration will be: htzdns-mb-macbook.conf
🔑 API key set:
   Please go to https://dns.hetzner.com/settings/api-token, generate an API token, and provide it here: m3AvUdZAiuaU15qNJoEHzBTBAdSLnOM9
   
   🔗 Checking mapping configuration...

   ⚙️  mbag.at has the following configuration options:
    [1] www
    [2] vpn

   Records marked with ✅ are currently configured.
   Swith these on/off individually by typing the line number, or hit enter to continue [1-2]:
   
```

Only A records are displayed, and you won't see "localhost" or "@" records by default.
Select the records you want to update, and they will get marked in an updated list display.

```

Configuration updated, here's your new list:

    [1] www
    [2] vpn ✅

   Records marked with ✅ are currently configured.
   Swith these on/off individually by typing the line number, or hit enter to continue [1-2]:

```

You can keep turning them on, but when you're done switching, hit enter.
This process will continue going through all your configured domain names, and all the A records.
Eventually, you will get to the point where the configuration is shown and you are asked if you want it saved:

```
⌛ Building config...
✅ Configuration is successfully constructed.

👁️  Please take a moment to eyeball the configuration below,
   and confirm that ./htzdns-rt0.conf can be updated.

### Automatically generated htzdns-rt0.conf (2020-05-12 21:04:23):
### for manual modifications, take a peak in htzdns.conf, we have quite some docs there.
htz_api_key="e56fg7890h1ijk23l4567m8ne56fg7890h1ijk23l4567m8n"
htz_update_zone_record_map="
example.com,1A23bcEF4G6h6IjklmnOpQ,www,ab1c234de56fg7890h1ijk23l4567m8n
example.com,1A23bcEF4G6h6IjklmnOpQ,vpn,1a23b456cde78f9g0h1ij23kl456m789
notexample.net,ABC1DEfg2hIJkl3M456Nop,lan,1b1c234de56fg7890h1ijk23lMNO78pQ
"

Are we OK to save to ./htzdns-rt0.conf? (Y/n)

✅ Excellent, configuration saved, cache flushed. We are done here.
   Running ./htzdns.sh with pretend to see what would be done. 😊

👻 PRETEND: Would write the following data to cache file ./data/cache.data:
2E84VpYX4J4x4AmdjsuCqX,8034ceb40747dd26a367b83dcdcd19d0,178.164.238.237

[myuser@mydevice]$#

```

And that's it :)

## Usage
Normally you run this on its own, and allow hosts to be updated.
However, there are usage flags you can use:

```
 ℹ️  Usage params: [ -h | h | --help ]
                  [ -pfrvv | pfrvv | --pretend --force --rebuild --verbose --verbose ]
                  [ -t | t | --test ]

   -f | f | --force	Force update to hetzner regardless of cache
   -r | r | --rebuild	Run configuration builder (interactive)
   -p | p | --pretend	Do not make any changes, only show what would be done.
   -v | v | --verbose	See what is happening.
   -t | t | --test	Run tests (confirms compat, cries out on API changes)
   -h | h | --help	This usage manual.

 Certain arguments can be used in combination with each other.
 The following scenario would require you to add multiple arguments:
 - You don't want to write any changes (pretend)
 - You want to see what would be updated regardless of what is cached
   or data in hetzner records (force)
 - You want to build a new configuration to add more records (rebuild)
 Then you can execute with --pretend --force --rebuild (or -prf for short)

 # ./htzdns.sh -prf

 Adding verbosity or double verbosity to this mix is also possible as -prfvv.
```

What these actually mean:
- force: you can update whe you have no IP changed at all. Otherwise, htzdns will aim to avoid unnecessary calls to the API, and will stop running when your current IP address is in its cache file, or in case your current IP address matches the one in Hetzner's records.
- rebuild: you can re-launch the configuration to make changes. It will guide you through the process again, but you can "enter yourself through" much of it, as settings are read from the saved config.
- verbose: htzdns is designed to run quietly (e.g. what you would expect in cron) - however, for debugging purposes, you can set it to be verbose. 
- pretend: show what would be done, but do not execute. This applies to writing cache file, or updating record(s) at Hetzner.
- test runs tests (not much of these just yet, but will do later on).


Note on verbosity levels:
- A simple -v will display all the internal functions and additional information to understand what is happening.
- Double verbosity (-vv) will display all data that's being sent between the functions (including api key, JSON results, etc).

Check update on your hetzner domain control ui.
If all looks cool, feel free to cron this for every minute.

### Under the hood
Few things to keep in mind:
1. It is safe to run this every minute, as a local cache file is present with the configured hosts and values, you will not be hitting the API unless there is a change in your IP.
2. Depending on whatismyip - this tool has a few hosts configured, you won't be hitting the same host over and over again with this either.

# Finalizing settings
## Adding to cron
Decide what user you want to use to run htzdns with. In the following example, user "nobody" will be used.

Run sudo -u nobody crontab -e, and add the following line: 

```
*/15 * * * * a="/path/to/htzdns/htzdns.sh"; [ -f $a ] && $a
```

Where /path/to/htzdns is obviously a full path to where you have cloned this repo to.

# Docs
More information is available in the [Docs](docs/) section.
