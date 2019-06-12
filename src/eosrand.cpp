#include <eosrand/eosrand.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>

uint8_t eosrand::newevent(name event_name, uint32_t prize_rank, name owner, string srv_seed){
   require_auth(owner);

   events idx(_self, owner.value);
   check(idx.find(event_name.value) == idx.end(), "existing event name");

   auto mb = (tapos_block_prefix() * tapos_block_num() * reinterpret_cast<const uint32_t>(&srv_seed));
   const char *mc = reinterpret_cast<const char *>(&mb); //mc = mixed Char
   auto seed = sha256((char *)mc, sizeof(mc));
   const char *p64 = reinterpret_cast<const char *>(&seed);
   //uint8_t random_number = (int8_t)p64[0];
   uint8_t random_number = (abs((int8_t)p64[0]) % (10)) + 1;

   idx.emplace(owner, [&](auto& gw) {
         gw.random_number = random_number;
         gw.seed = seed;
         gw.event_name = event_name;
         gw.prize_rank = prize_rank;
   });

   // print("[ seed ] ", seed," [ uint8_t result ] ", random_number);
   return random_number;
}

uint8_t eosrand::srand(){
   auto mb = tapos_block_prefix() * tapos_block_num();   //mb = mixed Block
   const char *mc = reinterpret_cast<const char *>(&mb); //mc = mixed Char
   auto result = sha256((char *)mc, sizeof(mc));
   const char *p64 = reinterpret_cast<const char *>(&result);
   uint8_t r = (abs((int8_t)p64[0]) % (10)) + 1;
   print("[ result ] ", result," [ uint8_t result ] ", r);
   return r;
}
