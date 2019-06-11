#include <eosrand/eosrand.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>

uint8_t eosrand::srand(){
   auto mb = tapos_block_prefix() * tapos_block_num();   //mb = mixed Block
   const char *mc = reinterpret_cast<const char *>(&mb); //mc = mixed Char
   auto result = sha256((char *)mc, sizeof(mc));
   const char *p64 = reinterpret_cast<const char *>(&result);
   uint8_t r = (abs((int8_t)p64[0]) % (10)) + 1;
   //print("result: ", result, " r: ", r);
   print("[ result ] ", result," [ uint8_t result ] ", r);
   return r;
}
