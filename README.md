# HTZDNS
A dynamic DNS udpater for Hetzner DNS robot.
TL;DR: Rewrite of hetzner-dyndns php script in bash, so we can run on a weak openwrt/ddwrt router from cron.

## Is this for me? (checklist):
- Do you want to access computers at a remote location?
- Do you host stuff on premise, with no cloud access?
- Does your ISP change your IP address every once in a while?
- Do you want to avoid paying monthly for a DYNDNS service, or update free tier every so often?
- Do you have a domain at Hetzner?
- Would a mysite.mydomain.tld sound good to you?

If you answered with "no" to any of the above questions, this is not for you.
In any other case, read on.

## What is it?
- Run this from cron periodically.
- It is written in bash, so it runs on weaker routers, rasp pi, or any modern microwave oven. 

# Usage
```
   -r | r | --rebuild	Run configuration builder (interactive)
   -p | p | --pretend   Do not make any changes, only show what would be done.
   -v | v | --verbose   See what's happening.
   -t | t | --test      Run tests (confirms compatibilty, warns on significant API changes)
   -h | h | --help	This usage manual.
```

## Prerequisites
1) Make sure you see your DNS zones in "Your zones" when you login to https://dns.hetzner.com
2) In your profile/API Tokens (https://dns.hetzner.com/settings/api-token) generate an access token, and give it a name, for example: htzdns (note: the token, a 32 chars long alphanumeric hash is displayed only once).
3) You may want to help yourself and lower the TTL on those records you're looking to update, for fast dns resolution change that is.

## Before you run
- Clone with git clone --recurse-submodules.
- Create a backup of your zone file
- Add test entry to try your DNS update with, before going wild

## Configuration
The quick way out is to run htzdns.sh -r to build configuration. This will ask for your API key, and fetch data from Hetzner to get your configuration built interactively.
Read [docs/configuration.md](docs/configuration.md) for the hard way.

## Running from cron
- Setting up in cron: normally the script is silent, unless configured to log / notify. The intention here is to contribute exactly none to your flash disk wear-out (SD Cards, SSDs, Flash memory - you know...)
- Setup in cron with a specific user (no need for root).

# Releases
## 1.0: the rewrite
```
Feature: update DNS entries at Hetzner

Scenario: There is no change to IP address
  Given: There is no change detected in the IP address at Hetzner and the User's IP address
  Then:  Do nothing

Scenario: Notify the user upon detected IP change
  Given: User's IP address is different than entry in Hetzner DNS
  And:   We have not been updating the same entry in the past 1 minute
  Then:  Notify the user about the change, and the update execution

Scenario: Update the Hetzner DNS entry upon IP change
  Given: User's IP address is different then entry in Hetzner DNS
  And:   We have not been updating the same entry in the past 1 minute
  Then:  Update the entry with the new IP address.

Scenario: Notify the user with the outcome of DNS update on messaging services
  Given: Change of DNS entry is successful
  Then:  Notify user about the change
```

Known bugs:
n/a

Known limitations:
- Pagination is not implemented, e.g. more than 100 entries will not be handled.
- Testing is manual, with a single (active) domain only - depending on popularity, automatests with a test domain may 

## Backlog (ordered)
- Add ipv6 support
- Autotests & test domains
- Handle pagination
