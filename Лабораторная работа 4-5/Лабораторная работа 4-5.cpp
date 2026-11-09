#include "functions.hpp"

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    FiniteAutomaton automaton = FiniteAutomaton::readFiniteAutomaton();
    FiniteAutomaton automatonWoutEpsilon = automaton.eliminateEpsilonTransitions();
    automatonWoutEpsilon.printTransitionFunc();
    FiniteAutomaton determinateAutomaton = automatonWoutEpsilon.determinateAutomaton();
    determinateAutomaton.printTransitionFunc();
}