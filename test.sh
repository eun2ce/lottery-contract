#!/bin/bash

cleos create account eosio eosio.token $(cat ~/.eos_public_key) -p eosio
cleos create account eosio eosrand $(cat ~/.eos_public_key) -p eosio
cleos create account eosio eun2ce $(cat ~/.eos_public_key) -p eosio
cleos create account eosio conr2d $(cat ~/.eos_public_key) -p eosio

cleos set account permission eosrand active --add-code
cleos set contract eosio.token /Users/eun2ce/gitstg/eosio.contracts/build/contracts/eosio.token/. -p eosio.token
cleos push action eosio.token create '["eun2ce","10000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eun2ce","1000.0000 EOS","memo"]' -p eun2ce
