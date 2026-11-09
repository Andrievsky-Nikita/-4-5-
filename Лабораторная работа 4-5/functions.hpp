#pragma once
#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <unordered_map>
#include <system_error>
#include <stdexcept>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <functional>
#include <limits>
#include <algorithm>
#include <windows.h>

struct Alphabet
{
private:
	std::string alphabet;
	std::string reservedChars = "()+.*e";
public:
	Alphabet() = default;
	Alphabet(const std::string& str);
	const std::string& getAlphabet() const noexcept { return alphabet; }
	const std::string& getReservedChars() const noexcept { return reservedChars; }
};

class FiniteAutomaton
{
private: 
	using StateId = std::size_t;
	using SymbolId = std::size_t;
	using StateSet = std::vector<StateId>;

	using TransitionKey = std::pair<StateId, SymbolId>;

	struct TransitionKeyHash
	{
		std::size_t operator()(const TransitionKey& key) const noexcept
		{
			std::size_t h1 = std::hash<StateId>{}(key.first);
			std::size_t h2 = std::hash<SymbolId>{}(key.second);

			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};

	using TransitionTable = std::unordered_map<
		TransitionKey,
		StateSet,
		TransitionKeyHash>;

	struct StateInfo
	{
		bool isStart = false;
		bool isFinal = false;
	};

	Alphabet alphabet;
	std::vector<StateInfo> states;
	TransitionTable transitionFunc;

	FiniteAutomaton(Alphabet&& alphabet,
		std::vector<StateInfo>&& states,
		TransitionTable&& function) :
		alphabet(std::move(alphabet)),
		states(std::move(states)),
		transitionFunc(std::move(function)) {}

	static bool isSpace(const char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }
	static StateSet parseInputUint(const std::string& input, const size_t len);
	static Alphabet readAlhabet();
	static std::vector<StateInfo> readStates();
	static TransitionTable readFunction(const Alphabet& alphabet, const std::vector<StateInfo>& states);

public: 
	static FiniteAutomaton readFiniteAutomaton();
	void printTransitionFunc() const;
	FiniteAutomaton eliminateEpsilonTransitions() const;
	bool checkAbsenceETransition() const {
		return std::any_of(
			transitionFunc.begin(),
			transitionFunc.end(),
			[this](std::pair<TransitionKey, StateSet> elem)
			{
				return transitionFunc.contains(elem.first);
			});
	};
	FiniteAutomaton determinateAutomaton() const;
};