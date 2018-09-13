﻿#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

typedef double real_type;

class pomelo : public contract
{
public:
    pomelo(account_name self) : 
        contract(self), 
        txlogs(_self, _self) {
    }

    void init(); void clean(); void test();
    void cancelsell(account_name issuer, account_name account, uint64_t id);
    void cancelbuy(account_name issuer, account_name account, uint64_t id);
    void buy(account_name account, asset bid, asset ask, account_name issuer);
    void sell(account_name account, asset bid, asset ask, account_name issuer);
    void match(account_name issuer, uint64_t buy_id, uint64_t sell_id);
    void (account_name issuer, uint64_t buy_id);

    void onTransfer(account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo);  

    void transfer(account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo);                 

    /// @abi table
    struct buyorder {
        uint64_t id;
        account_name account;
        asset bid;
        asset ask;
        time timestamp;

        uint64_t primary_key() const {return id;}
        real_type get_price() const {return real_type(ask.amount) / real_type(bid.amount);}        
        EOSLIB_SERIALIZE(buyorder, (id)(account)(bid)(ask)(timestamp))
    };
    typedef eosio::multi_index<N(buyorder), buyorder, 
        indexed_by<N(byprice), const_mem_fun<buyorder, real_type, &buyorder::get_price>>
    > buyorder_index;

    /// @abi table
    struct sellorder {
        uint64_t id;
        account_name account;
        asset bid;
        asset ask;
        time timestamp;        

        uint64_t primary_key() const {return id;}
        real_type get_price() const {return real_type(ask.amount) / real_type(bid.amount);}        
        EOSLIB_SERIALIZE(sellorder, (id)(account)(bid)(ask)(timestamp))
    };
    typedef eosio::multi_index<N(sellorder), sellorder, 
        indexed_by<N(byprice), const_mem_fun<sellorder, real_type, &sellorder::get_price>>
    > sellorder_index;

    /// @abi table
    struct txlog {
        uint64_t id;
        account_name buyer; // 买方 
        account_name seller; // 卖方
        asset bid; // 供
        asset ask; // 需
        time timestamp; // 时间戳         

        uint64_t primary_key() const {return id;}
        uint64_t get_timestamp() const {return timestamp;} 
        EOSLIB_SERIALIZE(txlog, (id)(buyer)(seller)(bid)(ask)(timestamp))        
    };
    typedef eosio::multi_index<N(txlog), txlog, 
        indexed_by<N(bytimestamp), const_mem_fun<txlog, uint64_t, &txlog::get_timestamp>>
    > txlogs_index;
    txlogs_index txlogs;

    void do_sell_trade(sellorder s) {
    //     auto per_index = buyorder.get_index<N(per)>();
    //     for (auto itr = per_index.upper_bound(s.per - 0.000000001); itr != per_index.end(); ++itr) {
    //         // 币种不同则跳过
    //         if (itr->asset.symbol != s.asset.symbol) {
    //             continue;
    //         }
    //         if (itr->per < s.per) {
    //             continue;
    //         }

    //         if (s.asset.amount >= itr->asset.amount) {
    //             auto sold_amount = itr->asset.amount;
    //             auto sold_eos = (uint64_t)(s.per * itr->asset.amount);
    //             insert_txlog(itr->account, s.account, asset(sold_amount, itr->asset.symbol), sold_eos, s.per);
    //             s.asset.amount -= sold_amount;
    //             s.total_eos -= sold_eos;
    //             auto eos_left = itr->total_eos - sold_eos;
    //             per_index.erase(itr);

    //             action(
    //                 permission_level{ _self, N(active) },
    //                 TOKEN_CONTRACT, N(transfer),
    //                 make_tuple(_self, itr->account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
    //                 .send();
    //             action(
    //                 permission_level{ _self, N(active) },
    //                 TOKEN_CONTRACT, N(transfer),
    //                 make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
    //                 .send();
    //             if (eos_left > 0) {
    //                 action(
    //                     permission_level{ _self, N(active) },
    //                     TOKEN_CONTRACT, N(transfer),
    //                     make_tuple(_self, itr->account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
    //                     .send();
    //             }
    //         }
    //         else {
    //             auto sold_amount = s.asset.amount;
    //             auto sold_eos = s.total_eos;

    //             insert_txlog(itr->account, s.account, asset(sold_amount, itr->asset.symbol), sold_eos, s.per);

    //             per_index.modify(itr, 0, [&](auto& t) {
    //                 t.asset.amount -= sold_amount;
    //                 t.total_eos -= sold_eos;
    //                 t.per = (double)t.asset.amount / (double)t.total_eos;
    //             });

    //             action(
    //                 permission_level{ _self, N(active) },
    //                 TOKEN_CONTRACT, N(transfer),
    //                 make_tuple(_self, itr->account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
    //                 .send();
    //             action(
    //                 permission_level{ _self, N(active) },
    //                 TOKEN_CONTRACT, N(transfer),
    //                 make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
    //                 .send();

    //             return;
    //         }
    //         if (s.asset.amount == 0) {
    //             break;
    //         }
    //     }
    //     if (s.asset.amount > 0) {
    //         sellorder.emplace(_self, [&](auto& t) {
    //             t.id = sellorder.available_primary_key();
    //             t.account = s.account;
    //             t.asset.symbol = s.asset.symbol;
    //             t.asset.amount = s.asset.amount;
    //             t.total_eos = s.total_eos; // 剩余的EOS
    //             t.per = s.per;
    //         });
    //     }
    }


    void do_buy_trade(buyorder b) {
//        auto index = sellorder.get_index<N(price)>();
        /*bool is_end = true;
        for (auto itr = index.lower_bound(b.price); itr != index.end(); ++itr) {
            if (b.ask.symbol != itr->bid.symbol) continue;

        }*/
                
        /*bool is_end;
        do {
        }*/
    //         is_end = true;
    //         for (auto itr = per_index.lower_bound(b.per); itr != per_index.end(); ++itr) {
    //             // 币种不同则跳过
    //             if (itr->asset.symbol != b.asset.symbol) {
    //                 continue;
    //             }

    //             if (b.per < itr->per)
    //             {
    //                 continue;
    //             }

    //             if (b.asset.amount < itr->asset.amount) {
    //                 auto sold_amount = b.asset.amount;
    //                 auto sold_eos = (uint64_t)(itr->per * b.asset.amount);
    //                 per_index.modify(itr, 0, [&](auto& t) {
    //                     t.asset.amount -= sold_amount;
    //                     t.total_eos -= sold_eos;
    //                 });

    //                 insert_txlog(b.account, itr->account, asset(sold_amount, itr->asset.symbol), sold_eos, itr->per);

    //                 auto eos_left = b.total_eos - sold_eos;

    //                 action(
    //                     permission_level{ _self, N(active) },
    //                     TOKEN_CONTRACT, N(transfer),
    //                     make_tuple(_self, b.account, asset(sold_amount, b.asset.symbol), string("transfer")))
    //                     .send();
    //                 action(
    //                     permission_level{ _self, N(active) },
    //                     TOKEN_CONTRACT, N(transfer),
    //                     make_tuple(_self, itr->account, asset(sold_eos, EOS), string("transfer")))
    //                     .send();

    //                 if (eos_left > 0) {
    //                     action(
    //                         permission_level{ _self, N(active) },
    //                         TOKEN_CONTRACT, N(transfer),
    //                         make_tuple(_self, b.account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
    //                         .send();
    //                 }

    //                 return;
    //             }
    //             else {
    //                 auto sold_amount = itr->asset.amount;
    //                 auto sold_eos = (uint64_t)(itr->per * itr->asset.amount);
    //                 b.asset.amount -= sold_amount;
    //                 b.total_eos -= sold_eos;
    //                 per_index.erase(itr);

    //                 insert_txlog(b.account, itr->account, asset(sold_amount, itr->asset.symbol), sold_eos, itr->per);

    //                 action(
    //                     permission_level{ _self, N(active) },
    //                     TOKEN_CONTRACT, N(transfer),
    //                     make_tuple(_self, b.account, asset(sold_amount, b.asset.symbol), string("transfer")))
    //                     .send();
    //                 action(
    //                     permission_level{ _self, N(active) },
    //                     TOKEN_CONTRACT, N(transfer),
    //                     make_tuple(_self, itr->account, asset(sold_eos, EOS), string("transfer")))
    //                     .send();
    //                 b.per = (double)b.asset.amount / (double)b.total_eos;
    //                 is_end = false;
    //             }
    //             /*
    //             if (b.asset.amount == 0) {
    //                 is_end = true;
    //                 break;
    //             }
    //             */
    //         }
    //     } while (!is_end);

    //     if (b.asset.amount > 0) {
    //         buyorder.emplace(_self, [&](auto& t) {
    //             t.id = buyorder.available_primary_key();
    //             t.account = b.account;
    //             t.asset.symbol = b.asset.symbol;
    //             t.asset.amount = b.asset.amount;
    //             t.total_eos = b.total_eos; // 剩余的EOS
    //             t.per = b.per;
    //         });
    //     }
    }

    void maintain_txlogs(uint64_t cnt) {
        auto timestamp_index = txlogs.get_index<N(bytimestamp)>();
        while (timestamp_index.begin() != timestamp_index.end()) {
            if (cnt-- == 0) return;
	        timestamp_index.erase(timestamp_index.begin());
        }
    }

    void insert_txlog(account_name buyer, account_name seller, asset ask, asset bid) {
        txlogs.emplace(_self, [&](auto& t){
            t.id = txlogs.available_primary_key();
            t.buyer = buyer;
            t.seller = seller;    
            t.ask = ask;
            t.bid = bid;        
            t.timestamp = current_time();
        });
    }
};