#include <eosrand/eosrand.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>

using std::string;

bool eosrand::hashcheck(const checksum256 hash) const {
   auto data = hash.get_array();
   return data[0] == 0 && data[1] == 0;
}

void eosrand::on_transfer(name from, name to, asset quantity, string memo) {
   if (from == _self) return;

   events idx(_self, from.value);
   const auto& it = idx.get(name(memo).value);

   check(!it.activated, "contract is already activated");
   check(hashcheck(it.seed), "not setting user seed");
   check(it.timelock < current_time_point(), "the expiration time should be in past");
   check(it.value == extended_asset{quantity, get_first_receiver()}, "token amount not match: " + it.value.quantity.to_string() + "@" + it.value.contract.to_string());

   idx.modify(it, same_payer, [&](auto& en) {
      en.activated = true;
   });
}

checksum256 eosrand::mixseed(const checksum256& sseed, checksum256& useed) const {
    st_seeds seeds;
    seeds.seed1 = sseed;
    seeds.seed2 = useed;
    //checksum256 result = eosio::sha256( (char *)&seeds.seed1, sizeof(seeds.seed1) * 2);
    //print("result: ", result);
    return eosio::sha256( (char *)&seeds.seed1, sizeof(seeds.seed1) * 2);
}

void eosrand::setbox(name owner, name contract_name, const std::vector<grade>& lbox) {
   require_auth(owner);

   check(lbox.size(), "no settings on luckybox");

   events idx(_self, owner.value);
   check(idx.find(contract_name.value) == idx.end(), "existing contract name");

   for (auto o : lbox) {
      luckybox.push_back({o.reward, o.value});
   }
}

void eosrand::newgacha(name owner, name contract_name, checksum256 rseed, uint8_t lucknum) {
}

void eosrand::newevent(name owner, name contract_name, name participant, extended_asset value, checksum256 oseed, time_point_sec timelock) {
   require_auth(owner);

   events idx(_self, _self.value);
   check(idx.find(contract_name.value) == idx.end(), "exising contract name");
   check(timelock > current_time_point(), "the expiration time should be in the future");
   check(participant != get_self(), "the contract itself connot be set as a participant");

   auto data = oseed.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());

   idx.emplace(owner, [&](auto& en){
         en.contract_name = contract_name;
         en.participant = participant;
         en.value = value;
         en.preimage = hash;
         en.timelock = timelock;
         en.activated = false;
   });
}

void eosrand::setuserseed(name owner, name contract_name, name participant, checksum256 useed) {
   require_auth(participant);

   events idx(_self, owner.value);
   const auto& it = idx.get(contract_name.value);

   check(it.participant == participant, "not expected participant");

   idx.modify(it, same_payer, [&](auto& en){
         en.seed = useed;
   });
}

void eosrand::setownseed(name owner, name contract_name, checksum256 oseed) {
   require_auth(owner);

   events idx(_self, owner.value);
   const auto& it = idx.get(contract_name.value);

   check(hashcheck(it.seed), "contract not activated");

   auto data = oseed.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());
   check(memcmp((const void*)it.preimage.data(), (const void*)hash.data(), 32) == 0, "invalid seed");

   auto result = mixseed(it.seed, oseed);

   idx.modify(it, same_payer,[&](auto& en){
         en.seed = result;
         en.activated = true;
   });
}

void eosrand::withdraw(name owner, name contract_name, checksum256 preimage) {
   events idx(_self, owner.value);
   const auto& it = idx.get(contract_name.value);
   check(it.activated, "contract not activated");

   // `preimage` works as a key here.
   //require_auth(it.recipient);

   auto data = preimage.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());
   check(memcmp((const void*)it.preimage.data(), (const void*)hash.data(), 32) == 0, "invalid preimage");

   transfer_action(it.value.contract, {{_self, "active"_n}}).send(_self, it.participant, it.value.quantity, "");

   idx.erase(it);
}

/*
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
*/
