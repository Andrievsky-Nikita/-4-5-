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
		std::cout << "Последовательно введите символы алфавита: ";
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
	std::vector<size_t> numbers;
	size_t pos = 0;

	while (pos < input.size())
	{
		while (pos < input.size() && isSpace(input[pos]))
		{
			pos++;
		}

		if (pos == input.size())
		{
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

		if (value == 0 || value > len)
		{
			throw std::out_of_range("Введеное число не соответствует номеру состояния");
		}

		numbers.push_back(value - 1);
	}
	
	std::sort(numbers.begin(), numbers.end());

	numbers.erase(
		std::unique(numbers.begin(), numbers.end()),
		numbers.end()
	);

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

	std::cout << "Введите номера начальных состояний\n"
		<< "Формат ввода: вводите целые числа разделяемые пробелом\n";

	defineStates(input, numbers);

	for (StateId id : numbers)
	{
		states[id].isStart = true;
	}

	std::cout << "Введите номера финальных состояний\n"
		<< "Формат ввода: вводите целые числа разделяемые пробелом\n";

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
					std::cout << "(q" << i + 1 << ", e) : ";
				else 
					std::cout << "(q" << i + 1 << ", " << alphabet.getAlphabet()[j] << ") : ";

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
	FiniteAutomaton automatonWoutEpsilon(*this);

	std::vector<StateSet> EClosureStates;

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
		
	for (size_t i = 0; i < states.size(); i++)
	{
		for (size_t j = 0; j < EClosureStates[i].size(); j++)
		{
			if (states[j].isFinal)
			{
				automatonWoutEpsilon.states[i].isFinal = true;
				continue;
			}

		}
		automatonWoutEpsilon.states[i].isFinal = false;
	}

	const auto getTransitionEClosureBySymbol = [this](StateId stateId, SymbolId symbolId, std::set<SymbolId>& arrayStates)
		{
			const auto ETransition = transitionFunc.find({ stateId, alphabet.getAlphabet().size() });
			const auto symbolTransition = transitionFunc.find({ stateId, symbolId });
			const auto endIt = transitionFunc.end();

			const auto getFromSymbol = [this, &arrayStates](StateId stateId, SymbolId symbolId)
				{
					for (size_t ind : transitionFunc.find({ stateId, symbolId })->second)
						arrayStates.emplace(ind);
				};

			const auto getFromEpsilon = [this, &arrayStates](StateId stateId)
				{
					for (size_t ind : transitionFunc.find({ stateId, alphabet.getAlphabet().size() })->second)
						arrayStates.emplace(ind);
				};

			if (symbolTransition != endIt)
			{
				getFromSymbol(stateId, symbolId);
				
				if (ETransition != endIt)
				{
					for (size_t ind : symbolTransition->second)
					{
						getFromEpsilon(ind);
					}
				}

			}

			if (ETransition != endIt)
				for (size_t indStateEpsilon : ETransition->second)
					if (transitionFunc.find({ indStateEpsilon, symbolId }) != endIt)
						for (size_t indStateSymbol : transitionFunc.find({ indStateEpsilon, symbolId })->second)
						{
							getFromSymbol(indStateSymbol, symbolId);

							if (transitionFunc.find({ indStateSymbol, symbolId }) != endIt)
								for (size_t indStateNewEpsilon : transitionFunc.find({ indStateSymbol, symbolId })->second)
									getFromEpsilon(indStateNewEpsilon);
						}

		};

	for (size_t i = 0; i < alphabet.getAlphabet().size(); i++)
	{
		for (size_t j = 0; j < states.size(); j++)
		{
			std::set<SymbolId> statesAchievableFromThisEClosure;
			for (size_t indState : EClosureStates[j])
			{
				getTransitionEClosureBySymbol(indState, i, statesAchievableFromThisEClosure);
			}

			for (size_t k = 0; k < EClosureStates.size(); k++)
			{
				StateSet states;
				if (std::any_of(
					EClosureStates.begin(),
					EClosureStates.end(),
					[&statesAchievableFromThisEClosure](size_t element)
					{
						return statesAchievableFromThisEClosure.contains(element);
					}))
				{
					states.push_back(k);
				}
				automatonWoutEpsilon.transitionFunc.emplace(std::pair(j, k), states);
			}
		}
	}

	return automatonWoutEpsilon;
}