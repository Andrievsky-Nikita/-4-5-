#include "functions.hpp"

Alphabet::Alphabet(const std::string& str) : alphabet(str)
{
	if (std::set<char>(str.begin(), str.end()).size() == str.size())
	{
		for (auto ch : str)
		{
			if (reservedChars.find(ch) != std::string::npos)
			{
				throw std::invalid_argument("Введен специальный символ. Алфавит не может содержать специальные символы");
			}
		}
	}
	else
	{
		throw std::invalid_argument("Алфавит содержит повторяющиеся символы");
	}
}

Alphabet FiniteAutomaton::readAlhabet()
{
	while (true)
	{
		std::cout << "Последовательно введите символы алфавита без пробелов: ";
		std::string alph;
		std::cin >> alph;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		try
		{
			return Alphabet(alph);
		}
		catch (const std::invalid_argument& er)
		{
			std::cout << "Введенный вами алфавит некорректен\nПодробности ошибки:"
				<< er.what() << '\n' << "Повторите попытку ввода!\n";
		}
	}
}

FiniteAutomaton::StateSet FiniteAutomaton::parseInputUint(const std::string& input, const size_t len)
{
	StateSet numbers;
	size_t pos = 0;

	const auto sortAndUnique = [](StateSet& numbers)
		{
			std::sort(numbers.begin(), numbers.end());

			numbers.erase(
				std::unique(numbers.begin(), numbers.end()),
				numbers.end()
			);
		};

	while (pos < input.size())
	{
		while (pos < input.size() && isSpace(input[pos]))
		{
			pos++;
		}

		if (pos == input.size())
		{
			sortAndUnique(numbers);

			return numbers;
		}

		const std::size_t start = pos;

		while (pos < input.size() && !isSpace(input[pos]))
		{
			pos++;
		}

		const char* first = input.data() + start;
		const char* last = input.data() + pos;

		std::size_t value;

		const auto [ptr, ec] = std::from_chars(first, last, value);

		if (ec == std::errc::result_out_of_range)
		{
			throw std::out_of_range("Введено слишком большое число");
		}

		if (ec != std::errc{} || ptr != last)
		{
			throw std::invalid_argument("Ошибка формата");
		}

		if (value >= len)
		{
			throw std::out_of_range("Введеное число не соответствует номеру состояния");
		}

		numbers.push_back(value);
	}
	
	sortAndUnique(numbers);

	return numbers;
}

std::vector<FiniteAutomaton::StateInfo> FiniteAutomaton::readStates()
{
	std::vector<StateInfo> states;
	int n;

	while (true)
	{
		std::cout << "Введите число - количество состояний автомата: ";

		if (std::cin >> n && n >= 1)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			states.resize(n);
			break;
		}
		else
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Неправильный ввод. Введите целое положительное число\n";
		}
	}

	const auto defineStates = [&](std::string& input, StateSet& numbers)
		{
			while (true)
			{
				std::cout << "Ваш ввод: ";

				std::getline(std::cin, input);

				try
				{
					numbers = parseInputUint(input, n);

					if (numbers.empty())
					{
						throw std::invalid_argument("Введена пустая строка");
					}

					break;
				}
				catch (const std::logic_error& er)
				{
					std::cout << "Ввод некорректен\nПодробности ошибки:"
						<< er.what() << '\n' << "Повторите попытку ввода!\n";
				}
			}
		};

	std::string input;
	std::vector<size_t> numbers;

	std::cout << "Введите индексы начальных состояний\n"
		<< "Формат ввода: целые индексы от 0 до " << n - 1
		<< ", разделенные пробелами\n";

	defineStates(input, numbers);

	for (StateId id : numbers)
	{
		states[id].isStart = true;
	}

	std::cout << "Введите индексы финальных состояний\n"
		<< "Формат ввода: целые индексы от 0 до " << n - 1
		<< ", разделенные пробелами\n";

	defineStates(input, numbers);

	for (StateId id : numbers)
	{
		states[id].isFinal = true;
	}

	return states;
}

FiniteAutomaton::TransitionTable FiniteAutomaton::readFunction(const Alphabet& alphabet, const std::vector<StateInfo>& states)
{
	TransitionTable function;

	std::cout << "ОПРЕДЕЛИТЕ ФУНЦИЮ ПЕРЕХОДОВ\n\n"
		<< "Формат ввода:\n"
		<< "Программа построчно перечислит все комбинации (состояние, символ)\n"
		<< "Для каждой из них вы перечисляете возвращаемые значения через пробел\n"
		<< "Если для данной пары функция не определена, нажмите enter\n"
		<< "Введите значения:\n\n\n";

	const auto readTransitionStates = [&alphabet, &states](const size_t i, const size_t j, std::string& input)
		{
			while (true)
			{
				if (j == alphabet.getAlphabet().size())
					std::cout << "(q" << i << ", e) : ";
				else 
					std::cout << "(q" << i << ", " << alphabet.getAlphabet()[j] << ") : ";

				std::getline(std::cin, input);

				try
				{
					StateSet numbers = parseInputUint(input, states.size());
					
					return numbers;
				}
				catch (const std::logic_error& er)
				{
					std::cout << "Ввод некорректен\nПодробности ошибки:"
						<< er.what() << '\n' << "Повторите попытку ввода!\n";
				}
			}
		};

	std::string input;

	for (size_t i = 0; i < states.size(); i++)
	{
		for (size_t j = 0; j < alphabet.getAlphabet().size() + 1; j++) 
		{
			StateSet valueFunc = readTransitionStates(i, j, input);

			if (!valueFunc.empty())
			{
				function.emplace(std::pair(i, j), std::move(valueFunc));
			}
		}
	}

	return function;
}

FiniteAutomaton FiniteAutomaton::readFiniteAutomaton()
{
	Alphabet alphabet = readAlhabet();
	std::vector<StateInfo> states = readStates();
	TransitionTable function = readFunction(alphabet, states);

	return FiniteAutomaton(std::move(alphabet), std::move(states), std::move(function));
}

FiniteAutomaton FiniteAutomaton::eliminateEpsilonTransitions() const
{
	if (!checkETransition())
	{
		return FiniteAutomaton(*this);
	}

	FiniteAutomaton automatonWoutEpsilon(*this);

	std::vector<StateSet> EClosureStates;
	EClosureStates.reserve(states.size());

	const auto getEClosureState = [this](auto&& self, StateId i, StateSet& EClosure)
		{
			if (std::find(EClosure.begin(), EClosure.end(), i) != EClosure.end())
			{
				return;
			}

			EClosure.push_back(i);

			const auto ETransiiton = transitionFunc.find({ i, alphabet.getAlphabet().size() });
			
			if (ETransiiton == transitionFunc.end())
			{
				return;
			}
			else
			{
				for (auto ind : ETransiiton -> second)
				{
					self(self, ind, EClosure);
				}
			}
		};

	for (size_t i = 0; i < states.size(); i++)
	{
		StateSet EClosure;
		getEClosureState(getEClosureState, i, EClosure);
		EClosureStates.push_back(EClosure);
	}
		
	for (size_t i = 0; i < EClosureStates.size(); i++)
	{
		automatonWoutEpsilon.states[i].isFinal =
			std::any_of(
				EClosureStates[i].begin(),
				EClosureStates[i].end(),
				[this](StateId id)
				{
					return states[id].isFinal;
				}
			);
	}

	const auto getTransitionEClosureBySymbol = [this](StateId stateId, SymbolId symbolId, std::set<StateId>& arrayStates)
		{
			const auto endIt = transitionFunc.end();

			if (transitionFunc.find({ stateId, symbolId } ) != endIt)
				for (size_t i : transitionFunc.find({ stateId, symbolId })->second)
				{
					arrayStates.emplace(i);

					if (transitionFunc.find({i, alphabet.getAlphabet().size()}) != endIt)
						for (size_t ind : transitionFunc.find({ i, alphabet.getAlphabet().size() })->second)
						arrayStates.emplace(ind);
				}

			if (transitionFunc.find({ stateId, alphabet.getAlphabet().size() }) != endIt)
				for (size_t indStateEpsilon : transitionFunc.find({ stateId, alphabet.getAlphabet().size() })->second)
					if (transitionFunc.find({ indStateEpsilon, symbolId }) != endIt)
						for (size_t indStateSymbol : transitionFunc.find({ indStateEpsilon, symbolId })->second)
						{
							arrayStates.emplace(indStateSymbol);

							if (transitionFunc.find({ indStateSymbol, alphabet.getAlphabet().size()}) != endIt)
								for (size_t ind : transitionFunc.find({ indStateSymbol , alphabet.getAlphabet().size() })->second)
									arrayStates.emplace(ind);
						}

		};

	automatonWoutEpsilon.transitionFunc.clear();

	for (size_t i = 0; i < alphabet.getAlphabet().size(); i++)
	{
		for (size_t j = 0; j < states.size(); j++)
		{
			std::set<StateId> statesAchievableFromThisEClosure;
			for (size_t indState : EClosureStates[j])
			{
				getTransitionEClosureBySymbol(indState, i, statesAchievableFromThisEClosure);
			}

			StateSet newStates;

			for (size_t k = 0; k < EClosureStates.size(); k++)
			{
				if (std::all_of(
					EClosureStates[k].begin(),
					EClosureStates[k].end(),
					[&statesAchievableFromThisEClosure](size_t element)
					{
						return statesAchievableFromThisEClosure.contains(element);
					}))
				{
					newStates.push_back(k);
				}
			}

			if (!newStates.empty())
			{
				automatonWoutEpsilon.transitionFunc.emplace(
					TransitionKey{ j, i },
					std::move(newStates)
				);
			}
		}
	}

	return automatonWoutEpsilon;
}

void FiniteAutomaton::printTransitionFunc(char statePrefix) const
{
	std::cout << "---ФУНКЦИЯ ПЕРЕХОДОВ АВТОМАТА---\n";

	const size_t epsilonId = alphabet.getAlphabet().size();

	for (size_t i = 0; i < states.size(); i++)
	{
		for (size_t j = 0; j < epsilonId + 1; j++)
		{
			std::cout << "(" << statePrefix << i << ", ";

			if (j == epsilonId)
			{
				std::cout << "e) : ";
			}
			else
			{
				std::cout << alphabet.getAlphabet()[j] << ") : ";
			}

			const auto it = transitionFunc.find({ i, j });

			if (it == transitionFunc.end())
			{
				std::cout << "∅\n";
				continue;
			}

			std::cout << "{";

			for (size_t k = 0; k < it->second.size(); k++)
			{
				if (k != 0)
				{
					std::cout << ", ";
				}

				std::cout << statePrefix << it->second[k];
			}

			std::cout << "}\n";
		}
	}
}

bool FiniteAutomaton::checkDeterminate() const
{
	if (checkETransition())
	{
		return false; 
	}

	if (std::any_of(
		transitionFunc.begin(),
		transitionFunc.end(),
		[this](const auto& elem)
		{
			return elem.second.size() > 1;
		}))
	{
		return false;
	}

	if (std::count_if(
		states.begin(),
		states.end(),
		[](const auto& elem)
		{
			return elem.isStart;
		}) != 1)
	{
		return false;
	}

	return true;
}

FiniteAutomaton FiniteAutomaton::determinateAutomaton() const
{
	if (checkDeterminate())
	{
		return FiniteAutomaton(*this);
	}

	FiniteAutomaton deterministicAutomaton = checkETransition()
		? eliminateEpsilonTransitions()
		: FiniteAutomaton(*this);

	deterministicAutomaton.states.clear();
	deterministicAutomaton.transitionFunc.clear();

	std::vector<StateSet> statesDeterminateAutomaton;
	statesDeterminateAutomaton.reserve(states.size());
	statesDeterminateAutomaton.emplace_back();

	deterministicAutomaton.states.push_back({ true, false });
	
	for (StateId i = 0; i < states.size(); i++)
		if (states[i].isStart)
			statesDeterminateAutomaton[0].push_back(i);

	const auto getTransition = [this](StateId stateId, SymbolId symbolId, std::vector<StateSet>& statesDeterminateAutomaton, FiniteAutomaton& deterministicAutomaton)
		{
			std::set<StateId> statesTransition;

			for (StateId i = 0; i < statesDeterminateAutomaton[stateId].size(); i++)
			{
				auto it = transitionFunc.find({ statesDeterminateAutomaton[stateId][i], symbolId });

				if (it != transitionFunc.end())
				{
					statesTransition.insert(it->second.begin(), it->second.end());
				}
			}

			if (statesTransition.size() == 0)
			{
				return;
			}

			StateId indTransition = statesDeterminateAutomaton.size();

			for (StateId i = 0; i < statesDeterminateAutomaton.size(); i++)
			{
				if (std::set(
					statesDeterminateAutomaton[i].begin(),
					statesDeterminateAutomaton[i].end()
					) == statesTransition)
				{
					indTransition = i;
					break;
				}
			}

			StateId newStateId = statesDeterminateAutomaton.size();

			if (indTransition == statesDeterminateAutomaton.size())
			{
				statesDeterminateAutomaton.push_back(StateSet(statesTransition.begin(), statesTransition.end()));
				deterministicAutomaton.states.push_back(StateInfo());
				deterministicAutomaton.transitionFunc.insert({ { stateId, symbolId }, StateSet{newStateId} });
			}
			else 
			{
				deterministicAutomaton.transitionFunc.insert({ { stateId, symbolId }, StateSet{indTransition} });
			}
		};

	size_t ind = 0;

	while (ind != statesDeterminateAutomaton.size())
	{
		for (SymbolId i = 0; i < alphabet.getAlphabet().size(); i++)
		{
			getTransition(ind, i, statesDeterminateAutomaton, deterministicAutomaton);
		}

		ind++;
	}

	for (StateId i = 0; i < statesDeterminateAutomaton.size(); i++)
	{
		if (std::any_of(
			statesDeterminateAutomaton[i].begin(),
			statesDeterminateAutomaton[i].end(),
			[this](const StateId& stateId)
			{
				return states[stateId].isFinal;
			}))
		{
			deterministicAutomaton.states[i].isFinal = true;
		}
		else
		{
			deterministicAutomaton.states[i].isFinal = false;
		}
	}

	return deterministicAutomaton;
}

std::string FiniteAutomaton::readChain() const
{
	std::string chain;

	while (true)
	{
		std::cout << "Введите цепочку: ";

		if (!std::getline(std::cin, chain))
		{
			throw std::runtime_error("Ошибка чтения цепочки");
		}

		try
		{
			checkChainForAlphabet(chain);
			return chain;
		}
		catch (const std::invalid_argument& er)
		{
			std::cout << "Введенная вами цепочка некорректна\nПодробности ошибки: "
				<< er.what() << '\n' << "Повторите попытку ввода!\n";
		}
	}

}

bool FiniteAutomaton::checkChain(const std::string& chain) const 
{
	checkChainForAlphabet(chain);

	if (!checkDeterminate())
	{
		FiniteAutomaton deterministicAutomaton =
			determinateAutomaton();

		return deterministicAutomaton.checkChain(chain);
	}

	const auto applyChain = [this](const std::string& chain, StateId startId)
		{
			std::set<StateId> statesStart{startId};
			std::set<StateId> statesResult;

			for (SymbolId i = 0; i < chain.size(); i++)
			{
				for (StateId indStateStart : statesStart)
				{
					const auto iterResult = transitionFunc.find({ indStateStart, alphabet.getAlphabet().find(chain[i]) });

					if (iterResult != transitionFunc.end())
						for (StateId ind : iterResult->second)
							statesResult.insert(ind);
				}

				if (statesResult.size() == 0)
					return false;

				statesStart = statesResult; 
				statesResult.clear();
			}

			return std::any_of(
				statesStart.begin(),
				statesStart.end(),
				[this](StateId ind)
				{
					return states[ind].isFinal;
				}
			);
		};

	for (StateId i = 0; i < states.size(); i++)
	{
		if (states[i].isStart && applyChain(chain, i))
		{
			return true;
		}
	}

	return false;
}