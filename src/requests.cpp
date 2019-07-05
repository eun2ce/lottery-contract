#include <eosrand/eosrand.hpp>

void eosrand::requests::refresh_schedule() {
   auto _idx = get_index<"schedtime"_n>();
   auto _it = _idx.begin();

   chances chn(self(), self().value);
   const auto& cit = chn.get(_it->id);

   schemes schm(self(), cit.dealer.value);
   const auto& sit = schm.get(cit.scheme_name.value);


   if (_it != _idx.end()) {
      auto timeleft = (_it->scheduled_time - sit.expiration).to_seconds();
      print("timeleft: ", timeleft);
      if (timeleft <= 0) {
         withdraw_processed(sit.budget.contract, {{self(), "active"_n}}).send(owner(), _it->id);
         clear();
      } else {
         transaction out;
         out.actions.emplace_back(
               permission_level{self(), "active"_n},
               self(),
               "withdraws"_n,
               std::make_tuple(owner(), _it->id)
         );
         out.delay_sec = static_cast<uint32_t>(timeleft);
         //cancel_deferred(owner().value);
         out.send(owner().value, owner());
         clear();
      }
   }
}

void eosrand::requests::clear() {
   require_auth(self());

   auto _idx = get_index<"schedtime"_n>();
   auto _it = _idx.begin();

   chances chn(self(), self().value);
   const auto& cit = chn.find(_it->id);

   schemes schm(self(), cit->dealer.value);
   const auto& sit = schm.get(cit->scheme_name.value);

   check(_it != _idx.end(), "withdrawal requests not found");

   for ( ; _it != _idx.end(); _it = _idx.begin()) {
      if (_it->scheduled_time > time_point_sec(sit.withdraw_delay_sec)) break;
      clear_withdraws(self(), {self(), "active"_n}).send(owner(), _it->id);
      _idx.erase(_it);
   }
}
