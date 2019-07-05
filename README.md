# EOS Randomevent Contract

## Clone
- Clone this repo to your local machine using `https://github.com/eun2ce/eosrand.git`

## build

```cmd
$ ./build.sh
```

## Deploy

```cmd
$ cleos create account eosio eosrand [public_key] -p eosio
$ cleos set contract eosrand . -p eosrand
```
