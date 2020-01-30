/*
 * strategy_harmonic.cpp
 *
 *  Created on: 29. 1. 2020
 *      Author: ondra
 */

#include "strategy_harmonic.h"

#include "../imtjson/src/imtjson/object.h"
#include "sgn.h"
Strategy_Harmonic::Strategy_Harmonic(const Config &cfg, const State &state):cfg(cfg),st(state) {}

Strategy_Harmonic::~Strategy_Harmonic() {
}

IStrategy::OrderData Strategy_Harmonic::getNewOrder(
		const IStockApi::MarketInfo &minfo, double cur_price, double new_price,
		double dir, double assets, double currency) const {

	double power = calcPower(currency);
	if (dir * st.strike <= 0) {
		return OrderData{0, dir * power - assets};
	}
	else {
		double sum = 0;
		int cnt = std::abs(st.strike);
		for (int i = 0; i <= cnt; i++)
			sum = sum + 1.0/(i+1.0);
		double diff = (dir * power * sum) - assets;
		return OrderData{0, diff};
	}

}

double Strategy_Harmonic::calcPower(double currency) const {
	return currency / st.p * std::pow(10, cfg.power)*0.01;
}

std::pair<IStrategy::OnTradeResult, ondra_shared::RefCntPtr<const IStrategy> > Strategy_Harmonic::onTrade(
		const IStockApi::MarketInfo &minfo, double tradePrice, double tradeSize,
		double assetsLeft, double currencyLeft) const {
	double dir = sgn(tradeSize);
	State newst = st;
	if (dir * st.strike > 0) {
		newst.strike = st.strike+dir;
	} else {
		newst.strike = dir;
	}
	return {
		OnTradeResult{
			0,0,0,0
		},PStrategy(new Strategy_Harmonic(cfg, newst))
	};
}

IStrategy::MinMax Strategy_Harmonic::calcSafeRange(
		const IStockApi::MarketInfo &minfo, double assets,
		double currencies) const {
		return MinMax {0,std::numeric_limits<double>::infinity()};
}

bool Strategy_Harmonic::isValid() const {
	return st.p > 0;
}

json::Value Strategy_Harmonic::exportState() const {
	return json::Object("strike", st.strike)("p",st.p);
}

PStrategy Strategy_Harmonic::onIdle(const IStockApi::MarketInfo &minfo,
		const IStockApi::Ticker &curTicker, double assets,
		double currency) const {
	if (st.p > 0) return this;
	else {
		State newst;
		newst.p =  curTicker.last;
		newst.strike = 0;
		return new Strategy_Harmonic(cfg, newst);
	}
}

double Strategy_Harmonic::getEquilibrium() const {
	return 0;
}

PStrategy Strategy_Harmonic::reset() const {
	return new Strategy_Harmonic(cfg);
}

std::string_view Strategy_Harmonic::id("harmonic");

std::string_view Strategy_Harmonic::getID() const {
	return id;
}

json::Value Strategy_Harmonic::dumpStatePretty(
		const IStockApi::MarketInfo &minfo) const {
	return json::Object("Strike", st.strike);
}

PStrategy Strategy_Harmonic::importState(json::Value src) const {
	State newst {
		static_cast<int>(src["strike"].getInt()),
		src["p"].getNumber()
	};
	return new Strategy_Harmonic(cfg, newst);
}
