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

## Usage

- srand

```cmd
$ cleos push action eosrand srand '[]' -p eosrand
```

```cmd
>> [ result ] 617ad3337a0ccb092c8bcc7604877699c7377c93cbae8a6ebac33d61ca3e12c6 [ uint8_t result ] 4
```

- newevent

```cmd
$ cleos push action eosrand newevent '["newevent1", 1, "eosrand", "thisisseed"]' -p eosrand
$ cleos push action eosrand newevent '["newevent2", 1, "eosrand", "thisisseed"]' -p eosrand
$ cleos push action eosrand newevent '["newevent3", 1, "eosrand", "thisisseed"]' -p eosrand
```

```cmd
$ cleos get table eosrand eosrand event
{
  "rows": [{
      "random_number": 6,
      "seed": "3ed95117ef3b32e8f64f91a9dc3af3193748234df37b2e78f1b877f331927533",
      "event_name": "newevent",
      "prize_rank": 1
    },{
      "random_number": 4,
      "seed": "454e98aaae098b0d875dc9d92e79bd0d300aa627c1dbb6c73754bdbf0ceb95a8",
      "event_name": "newevent2",
      "prize_rank": 1
    },{
      "random_number": 2,
      "seed": "4dadd7aeeff996d5bd4435de69158b01eadcacbafe266c55bf2dd10121e4425e",
      "event_name": "newevent3",
      "prize_rank": 1
    }
  ],
  "more": false
}
```
