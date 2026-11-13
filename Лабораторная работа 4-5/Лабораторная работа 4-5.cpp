#include "functions.hpp"

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        FiniteAutomaton automaton =
            FiniteAutomaton::readFiniteAutomaton();

        FiniteAutomaton automatonWoutEpsilon =
            automaton.eliminateEpsilonTransitions();

        std::cout << "\nАвтомат без e-переходов:\n";
        automatonWoutEpsilon.printTransitionFunc('S');

        FiniteAutomaton deterministicAutomaton =
            automatonWoutEpsilon.determinateAutomaton();

        std::cout << "\nДетерминированный автомат:\n";
        deterministicAutomaton.printTransitionFunc('P');

        while (true)
        {
            std::cout << "\n========== МЕНЮ ==========\n"
                << "1. Проверить цепочку\n"
                << "0. Завершить программу\n"
                << "Ваш выбор: ";

            int choice;

            if (!(std::cin >> choice))
            {
                if (std::cin.eof())
                {
                    break;
                }

                std::cin.clear();
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(),
                    '\n'
                );

                std::cout << "Некорректный ввод. Повторите попытку.\n";
                continue;
            }

            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n'
            );

            switch (choice)
            {
            case 1:
            {
                try
                {
                    std::string chain =
                        deterministicAutomaton.readChain();

                    if (deterministicAutomaton.checkChain(chain))
                    {
                        std::cout << "Цепочка допускается автоматом!\n";
                    }
                    else
                    {
                        std::cout << "Цепочка не допускается автоматом!\n";
                    }
                }
                catch (const std::runtime_error& er)
                {
                    if (std::cin.eof())
                    {
                        std::cout << "\nВвод завершён.\n";
                        return 0;
                    }

                    throw;
                }

                break;
            }

            case 0:
            {
                std::cout << "Завершение программы.\n";
                return 0;
            }

            default:
            {
                std::cout << "Такого пункта меню нет. Повторите выбор.\n";
                break;
            }
            }
        }
    }
    catch (const std::exception& er)
    {
        std::cerr << "\nОшибка выполнения программы: "
            << er.what() << '\n';

        return 1;
    }

    return 0;
}