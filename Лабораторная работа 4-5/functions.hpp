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
#include <iterator>

struct Alphabet
{
private:
	std::string alphabet;
	std::string reservedChars = "()+.*e ";
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

	FiniteAutomaton() = default;
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
	void checkChainForAlphabet(const std::string& chain) const
	{
		if (std::any_of(
			chain.begin(),
			chain.end(),
			[this](char symbol)
			{
				return alphabet.getAlphabet().find(symbol) == std::string::npos;
			}
		))
		{
			throw std::invalid_argument("цепочка не соответствует алфавиту автомата");
		}
	}

public: 
	static FiniteAutomaton readFiniteAutomaton();
	void printTransitionFunc() const;
	bool checkETransition() const
	{
		return std::any_of(
			transitionFunc.begin(),
			transitionFunc.end(),
			[this](const auto& elem)
			{
				return elem.first.second == alphabet.getAlphabet().size();
			});
	};
	bool checkDeterminate() const;
	std::string readChain() const;
	bool checkChain(const std::string& chain) const;
	FiniteAutomaton eliminateEpsilonTransitions() const;
	FiniteAutomaton determinateAutomaton() const;
};