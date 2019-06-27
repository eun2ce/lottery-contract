#include <eosrand/eosrand.hpp>
#include <eosio/transaction.hpp>

using std::string;

vector<int8_t> eosrand::mixseed(const checksum256& dseed, const checksum256& oseed) const {
   std::array<int8_t, 64> data;
   datastream<char*> ds(reinterpret_cast<char*>(data.data()), data.size());
   ds << dseed.extract_as_byte_array();
   ds << oseed.extract_as_byte_array();
   return std::vector<int8_t>(data.begin(), data.end());
}

void eosrand::on_transfer(name from, name to, asset quantity, string memo) {
   if (from == _self) return;

   schemes idx(_self, from.value);
   const auto& it = idx.get(name(memo).value);
   check(!it.activated, "contract is already activated");

   idx.modify(it, same_payer, [&](auto& s) {
      s.activated = true;
   });
}

void eosrand::newscheme(name dealer, name scheme_name, std::vector<grade> &grades, extended_asset budget, uint32_t withdraw_delay_sec, time_point_sec expiration, optional<uint8_t> precision) {
   require_auth(dealer);

   schemes schm(_self, dealer.value);
   check(schm.find(scheme_name.value) == schm.end(), "existing scheme name");
   check(expiration > current_time_point(), "the expiration time should be in the future");
   //check(expiration > current_time_point() + min_duration, "the expiration time should be in the future");

   schm.emplace(_self, [&](auto& s) {
      s.scheme_name = scheme_name;
      s.grades = grades;
      s.budget = budget;
      s.withdraw_delay_sec = withdraw_delay_sec;
      s.expiration = expiration;
      check(!precision || *precision <= 4, "precision cannot exceed 4 bytes");
      s.precision = (precision) ? *precision : 1;
      s.out.symbol = budget.quantity.symbol;
      s.out_count.resize(grades.size());
   });

}

void eosrand::newchance(name dealer, name scheme_name, name owner, checksum256 dseedhash, optional<uint64_t> id) {
   require_auth(dealer);

   schemes schm(_self, dealer.value);
   auto it = schm.find(scheme_name.value);
   check(it != schm.end(), "scheme not found");
   check(it->expiration > current_time_point(), "scheme expired");
   check(it->out.amount <= it->budget.quantity.amount, "budget exhausted");

   chances chn(_self, _self.value);

   auto chance_id = (id) ? *id : chance::hash(dealer, scheme_name, dseedhash);
   auto cit = chn.find(chance_id);
   check(cit == chn.end(), "existing chance");

   chn.emplace(_self, [&](auto& c) {
      c.id = chance_id;
      c.owner = owner;
      c.dealer = dealer;
      c.scheme_name = scheme_name;
      c.dseedhash = dseedhash;
   });
}

void eosrand::setoseed(name owner, uint64_t id, checksum256 oseed) {
   require_auth(owner);

   chances chn(_self, _self.value);
   const auto& cit = chn.get(id);
   check(cit.owner == owner, "not expected owner");
   chn.modify(cit, same_payer, [&](auto& c) {
      c.oseed = oseed;
   });

   schemes schm(_self, cit.dealer.value);
   const auto& sit = schm.get(cit.scheme_name.value);

   transaction out;
   out.actions.emplace_back(
         permission_level{ _self, "active"_n},
         _self,
         "withdraw"_n,
         std::make_tuple(cit.dealer, cit.owner, cit.id, cit.oseed)
   );
   out.delay_sec = sit.withdraw_delay_sec;
   cancel_deferred(cit.owner.value);
   out.send(cit.owner.value, cit.owner);
}

void eosrand::setdseed(name dealer, uint64_t id, checksum256 dseed) {
   require_auth(dealer);

   chances chn(_self, _self.value);
   const auto& it = chn.get(id);
   check(it.dealer == dealer, "not expected dealer");

   auto data = dseed.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());
	check(memcmp((const void*)it.dseedhash.data(), (const void*)hash.data(), 32) == 0, "invalid dealer seed");

   // TODO: draw
   schemes schm(_self, it.dealer.value);
   const auto& sit = schm.get(it.scheme_name.value);

   vector<int8_t> result;
   sio4::hash_drbg drbg(mixseed(dseed, it.oseed));
   drbg.generate_block(result, sizeof(result));
   print(result.data());

   uint32_t score = 0;
   memcpy((void*)&score, (const void*)result.data(), sit.precision);

   uint32_t i = 0;
   for (auto gd : sit.grades) {
      if (score >= gd.score) {
         if (gd.limit && sit.out_count[i] >= *(gd.limit)) {
            i++;
            continue;
         }
         break;
      }
      i++;
   }

   if (i >= sit.grades.size()) {
      // TODO: not chosen
      action({_self, "active"_n}, _self, "raincheck"_n,
         std::make_tuple(dealer, id, score)
      ).send();
   }

   auto reward = extended_asset{sit.grades[i].reward, sit.budget.contract};

   // TODO: token transfer
   transfer_action(sit.budget.contract, {{_self, "active"_n}}).send(_self, it.owner, sit.grades[i].reward, "");

   // TODO: out amount, out_count update
   schm.modify(sit, same_payer, [&](auto& s) {
      s.out += sit.grades[i].reward;
      s.out_count[i]++;
      check(s.out <= s.budget.quantity, "budget exceeded");
   });

   // TODO: delete chance
   chn.erase(it);
}

void eosrand::withdraw(name dealer, name owner, uint64_t id, checksum256 oseed) {
   check(has_auth(_self) || has_auth(owner), "Missing required authority");

   chances chn(_self, _self.value);
   auto cit = chn.find(id);
   check(cit->oseed == oseed, "invalid owner seed");

   schemes schm(_self, dealer.value);
   const auto& sit = schm.get(cit->scheme_name.value);

   cancel_deferred(cit->owner.value);
   check(sit.activated, "contract not activated");
   check(sit.expiration < current_time_point(), "contract is not expiration");

   transfer_action(sit.budget.contract, {{_self, "active"_n}}).send(_self, cit->owner, sit.grades.back().reward, "");

   schm.modify(sit, same_payer, [&](auto& s) {
      s.out += sit.grades.back().reward;
      s.out_count[sit.grades.size()-1]++;
      check(s.out <= s.budget.quantity, "budget exceeded");
   });

   chn.erase(cit);
}

void eosrand::winreward(name owner, uint64_t id, uint32_t score, extended_asset value) {
   require_auth(_self);
}

void eosrand::raincheck(name owner, uint64_t id, uint32_t score) {
   require_auth(_self);
}
